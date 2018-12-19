#include "my_quic_video_sender.h"
#include "my_quic_header.h"
#include "net/quic/core/quic_data_reader.h"
#include "net/quic/core/quic_data_writer.h"
#include "net/quic/core/frames/quic_stream_frame.h"
#include "net/quic/core/frames/quic_frame.h"
#include "net/quic/core/quic_pending_retransmission.h"
#include "my_quic_framer.h"
#include "fakevideogenerator.h"
#include "my_quic_utils.h"
#include <memory.h>
#include <iostream>
#include <unistd.h>
namespace net{
const uint8_t kPublicHeaderSequenceNumberShift = 4;
MyQuicVideoSender::MyQuicVideoSender(Perspective pespective)
:pespective_(pespective)
,sent_packet_manager_(pespective,&clock_,&stats_,kBBR,kNack)
,time_of_last_received_packet_(clock_.ApproximateNow())
,next_(QuicTime::Zero())
,stop_(QuicTime::Zero())
,last_output_(QuicTime::Zero())
,ref_time_(QuicTime::Zero())
,next_rate_update_(QuicTime::Zero()){
	sent_packet_manager_.SetHandshakeConfirmed();
	 versions_.push_back(ParsedQuicVersion(PROTOCOL_QUIC_CRYPTO, QUIC_VERSION_43));
}
MyQuicVideoSender::~MyQuicVideoSender(){
	if(f_rate_.is_open()){
		f_rate_.close();
	}
}
void MyQuicVideoSender::EnableRateRecord(std::string name){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX))
			+"/"+name+"-rate.txt";
	enable_log_=true;
	f_rate_.open(path.c_str(), std::fstream::out);
}
void MyQuicVideoSender::RecordRate(QuicTime now){
	if(!enable_log_){
		return ;
	}
	if(last_output_==QuicTime::Zero()){
		ref_time_=now;
		last_output_=now+QuicTime::Delta::FromMilliseconds(100);
		return ;
	}
	if(now>last_output_){
		int64_t bw=0;
		QuicTime::Delta delta=now-ref_time_;
		int64_t ms=delta.ToMilliseconds();
		bw=sent_packet_manager_.BandwidthEstimate().ToKBitsPerSecond();
		if(f_rate_.is_open()){
			char line [256];
			memset(line,0,256);
			sprintf (line, "%lld %16lld",ms,bw);
			f_rate_<<line<<std::endl;
		}
		last_output_=now+QuicTime::Delta::FromMilliseconds(100);
	}
}
void MyQuicVideoSender::OnIncomingData(uint8_t *data,int len){
	QuicTime now=clock_.Now();
	time_of_last_received_packet_=now;
	char buf[kMaxPacketSize]={0};
	memcpy(buf,data,len);
	my_quic_header_t header;
	uint8_t public_flags=0;
	QuicDataReader header_reader(buf,len, NETWORK_BYTE_ORDER);
	header_reader.ReadBytes(&public_flags,1);
	header.seq_len= ReadSequenceNumberLength(
        public_flags >> kPublicHeaderSequenceNumberShift);
	uint64_t seq=0;
	header_reader.ReadBytesToUInt64(header.seq_len,&seq);
	uint32_t header_len=sizeof(uint8_t)+header.seq_len;
	uint32_t remain=len-header_len;
	QuicDataReader reader(buf+header_len,remain, NETWORK_BYTE_ORDER);
	MyQuicFramer framer(versions_,now,pespective_);
	framer.set_visitor(this);
	QuicPacketHeader quic_header;// no use
	framer.ProcessFrameData(&reader,quic_header);
	return ;
}
bool MyQuicVideoSender::OnAckFrame(const QuicAckFrame& frame){
	return false;
}
bool MyQuicVideoSender::OnAckFrameStart(QuicPacketNumber largest_acked,
	                 QuicTime::Delta ack_delay_time){
	sent_packet_manager_.OnAckFrameStart(largest_acked,ack_delay_time,
			time_of_last_received_packet_);
	if(largest_acked>=largest_acked_){
		largest_acked_=largest_acked;
	}
	return true;
}
bool MyQuicVideoSender::OnAckRange(QuicPacketNumber start,
	            QuicPacketNumber end,
	            bool last_range){
	sent_packet_manager_.OnAckRange(start, end);
	QuicPacketNumber i=0;
	for(i=start;i<end;i++){
		RemoveAckedPacket(i);
	}
	if (!last_range) {
	    return true;
	  }
	  bool acked_new_packet =
	      sent_packet_manager_.OnAckFrameEnd(time_of_last_received_packet_);
	  PostProcessAfterAckFrame(GetLeastUnacked() > start, acked_new_packet);
	  return true;
}
bool MyQuicVideoSender::Process(){
	bool ret=true;
	QuicTime now=clock_.Now();
	int64_t frame_len=source_->GetFrame(now);
	if(frame_len>0){
		uint16_t splits[MAX_SPLIT_NUMBER], total=0;
		int i=0;
		assert((frame_len/ QUIC_MAX_VIDEO_SIZE) < MAX_SPLIT_NUMBER);
		total = FrameSplit(splits,frame_len);
		for(i=0;i<total;i++){
			int value=splits[i];
			pending_queue_.push_back(value);
		}
	}
	if(sent_packet_manager_.TimeUntilSend(now)==QuicTime::Delta::Zero()){
		SendDataOrPadding();
	}
	int recv=0;
	su_addr remote;
	uint32_t max_len=1500;
	char buf[1500]={0};
	recv=socket_->RecvFrom(&remote,(uint8_t*)buf,max_len);
	if(recv>0){
		OnIncomingData((uint8_t*)buf,recv);
	}
	if(stop_!=QuicTime::Zero()){
		if(now>stop_){
			ret=false;
		}
	}
	if(stop_waiting_count_>2){
		SendStopWaitingFrame();
	}
    SendRetransmission();
	UpdateSourceRate(now);
	RecordRate(now);
	return ret;
}
void MyQuicVideoSender::set_duration(uint32_t ms){
	QuicTime now=clock_.Now();
	stop_=now+QuicTime::Delta::FromMilliseconds(ms);
}
void MyQuicVideoSender::OnPacketSent(QuicPacketNumber packet_number,
                   QuicPacketNumberLength packet_number_length,
				   QuicPacketLength encrypted_length,
				   HasRetransmittableData has_retransmittable_data){
	QuicTime now=clock_.Now();
	SerializedPacket info(packet_number,packet_number_length,NULL,encrypted_length,false,false);
	uint8_t retransmittable =has_retransmittable_data;
	if (retransmittable == HAS_RETRANSMITTABLE_DATA) {
		QuicStreamFrame *stream=new QuicStreamFrame();
      		info.retransmittable_frames.push_back(
         	 QuicFrame(stream));
    	}
	sent_packet_manager_.OnPacketSent(&info,0,now, NOT_RETRANSMISSION,has_retransmittable_data);

}
void MyQuicVideoSender::SendDataOrPadding(){
	if(pending_queue_.empty()){
		int64_t bw=sent_packet_manager_.BandwidthEstimate().ToKBitsPerSecond()*1000;
		int64_t encoder_rate=source_->GetMaxRate();
		if(bw<encoder_rate){
			SendPaddingData();
            //if(bw>encoder_rate){
                 //std::cout<<"bw "<<bw<<" "<<encoder_rate<<std::endl;
            //}
		}
	}else{
		SendFakePacket();
	}
}
void MyQuicVideoSender::SendFakePacket(){
	if(!pending_queue_.empty()){
		auto it=pending_queue_.begin();
		int len=(*it);
		pending_queue_.erase(it);
		char buf[kMaxPacketSize]={0};
		int used=0;
		my_quic_header_t header;
		header.seq=seq_;
		header.seq_len=GetMinSeqLength(header.seq);
		uint8_t public_flags=0;
		public_flags |= GetPacketNumberFlags(header.seq_len)
	                  << kPublicHeaderSequenceNumberShift;
		QuicDataWriter writer(kMaxPacketSize, buf, NETWORK_BYTE_ORDER);
		writer.WriteBytes(&public_flags,1);
		used+=1;
		writer.WriteBytesToUInt64(header.seq_len,seq_);
		used+=header.seq_len;
		uint8_t type=STREAM_FRAME;
		writer.WriteBytes(&type,1);
		used+=1;
		socket_->SendTo(&peer_,(uint8_t*)buf,used+len);
		uint16_t payload=len;
		OnPacketSent(header.seq,(QuicPacketNumberLength)header.seq_len,
				payload,HAS_RETRANSMITTABLE_DATA);
		sent_queue_.insert(std::make_pair(seq_,len));
		seq_++;
	}

}
void MyQuicVideoSender::SendPaddingData(){
	char buf[QUIC_MAX_VIDEO_SIZE]={0};
	int used=0;
	my_quic_header_t header;
	header.seq=seq_;
	header.seq_len=GetMinSeqLength(header.seq);
	uint8_t public_flags=0;
	public_flags |= GetPacketNumberFlags(header.seq_len)
                  << kPublicHeaderSequenceNumberShift;
	QuicDataWriter writer(kMaxPacketSize, buf, NETWORK_BYTE_ORDER);
	writer.WriteBytes(&public_flags,1);
	used+=1;
	writer.WriteBytesToUInt64(header.seq_len,seq_);
	used+=header.seq_len;
	uint8_t type=STREAM_FRAME;
	writer.WriteBytes(&type,1);
	used+=1;
	socket_->SendTo(&peer_,(uint8_t*)buf,QUIC_MAX_VIDEO_SIZE);
	uint16_t payload=QUIC_MAX_VIDEO_SIZE-used;
	OnPacketSent(header.seq,(QuicPacketNumberLength)header.seq_len,
			payload, NO_RETRANSMITTABLE_DATA);
	seq_++;
}
void MyQuicVideoSender::SendRetransmission(){
	QuicTime now=clock_.Now();
	bool retrans=false;
	while(sent_packet_manager_.TimeUntilSend(now)==QuicTime::Delta::Zero()){
		if(sent_packet_manager_.HasPendingRetransmissions()){
			QuicPendingRetransmission pending=sent_packet_manager_.NextPendingRetransmission();
			OnRetransPacket(pending);
			retrans=true;
		}else{
			break;
		}
	}
	if(retrans){
		//SendStopWaitingFrame();
	}
	return ;
}
void MyQuicVideoSender::OnRetransPacket(QuicPendingRetransmission pending){
	uint64_t old_seq=pending.packet_number;
	auto it=sent_queue_.find(old_seq);
	//only retrans a packet once
	if(it!=sent_queue_.end()){
		int len=it->second;
		sent_queue_.erase(it);
		char buf[kMaxPacketSize]={0};
		int used=0;
		my_quic_header_t header;
		header.seq=seq_;
		header.seq_len=GetMinSeqLength(header.seq);
		uint8_t public_flags=0;
		public_flags |= GetPacketNumberFlags(header.seq_len)
	                  << kPublicHeaderSequenceNumberShift;
		QuicDataWriter writer(kMaxPacketSize, buf, NETWORK_BYTE_ORDER);
		writer.WriteBytes(&public_flags,1);
		used+=1;
		writer.WriteBytesToUInt64(header.seq_len,seq_);
		used+=header.seq_len;
		uint8_t type=STREAM_FRAME;
		writer.WriteBytes(&type,1);
		used+=1;
		socket_->SendTo(&peer_,(uint8_t*)buf,used+len);
		uint16_t payload=len;
		QuicTime now=clock_.Now();
		SerializedPacket info(header.seq,(QuicPacketNumberLength)header.seq_len,NULL,payload,false,false);
		for(const QuicFrame& frame : pending.retransmittable_frames){
			//if(frame.type==STREAM_FRAME){
				//QuicStreamFrame *stream=frame.stream_frame;
				//delete stream;
			//}
			info.retransmittable_frames.push_back(frame);
		}
		sent_packet_manager_.OnPacketSent(&info,old_seq,now,LOSS_RETRANSMISSION,HAS_RETRANSMITTABLE_DATA);
		sent_queue_.insert(std::make_pair(seq_,len));
		seq_++;
	}
}
void MyQuicVideoSender::SendStopWaitingFrame(){
	  stop_waiting_count_=0;
	  QuicPacketNumber unack_seq=sent_packet_manager_.GetLeastUnacked();
	  //std::cout<<"unack "<<unack_seq<<" largest ack "<<largest_acked_<<std::endl;
	  char buf[kMaxPacketSize]={0};
	  my_quic_header_t header;
	  header.seq=seq_;
	  header.seq_len=GetMinSeqLength(header.seq);
	  uint8_t public_flags=0;
	  public_flags |= GetPacketNumberFlags(header.seq_len)
	                  << kPublicHeaderSequenceNumberShift;
	  QuicDataWriter writer(kMaxPacketSize, buf, NETWORK_BYTE_ORDER);
	  writer.WriteBytes(&public_flags,1);
	  writer.WriteBytesToUInt64(header.seq_len,seq_);
	  uint8_t type=STOP_WAITING_FRAME;
	  writer.WriteBytes(&type,1);
	  writer.WriteBytesToUInt64(header.seq_len,unack_seq);
	  uint32_t len=writer.length();
	  socket_->SendTo(&peer_,(uint8_t*)buf,len);
}
QuicPacketNumber MyQuicVideoSender::GetLeastUnacked() const {
  return sent_packet_manager_.GetLeastUnacked();
}
void MyQuicVideoSender::PostProcessAfterAckFrame(bool send_stop_waiting,
                                              bool acked_new_packet){
	  if (send_stop_waiting) {
	    ++stop_waiting_count_;
	  } else {
	    stop_waiting_count_ = 0;
	  }
}
void MyQuicVideoSender::UpdateSourceRate(QuicTime now){
	if(next_rate_update_==QuicTime::Zero()){
		next_rate_update_=now+
				QuicTime::Delta::FromMilliseconds(rate_updata_gap_);
	}else{
		if(now>next_rate_update_){
			int64_t bw=sent_packet_manager_.BandwidthEstimate().ToKBitsPerSecond()*1000;
			source_->UpdataRate(bw);
			next_rate_update_=now+
					QuicTime::Delta::FromMilliseconds(rate_updata_gap_);
		}
	}
}
void MyQuicVideoSender::RemoveAckedPacket(QuicPacketNumber seq){
	auto it=sent_queue_.find(seq);
	if(it!=sent_queue_.end()){
		sent_queue_.erase(it);
	}
}
}
