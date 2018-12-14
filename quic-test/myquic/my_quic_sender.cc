#include "my_quic_sender.h"
#include "my_quic_header.h"
#include "net/quic/core/quic_data_reader.h"
#include "net/quic/core/quic_data_writer.h"
#include "net/quic/core/frames/quic_stream_frame.h"
#include "net/quic/core/frames/quic_frame.h"
#include "net/quic/core/quic_pending_retransmission.h"
#include "my_quic_framer.h"
#include <memory.h>
#include <iostream>
#include <unistd.h>
namespace net{
const uint8_t kPublicHeaderSequenceNumberShift = 4;
MyQuicSender::MyQuicSender(Perspective pespective)
:pespective_(pespective)
,sent_packet_manager_(pespective,&clock_,&stats_,kBBR,kNack)
,time_of_last_received_packet_(clock_.ApproximateNow())
,next_(QuicTime::Zero())
,stop_(QuicTime::Zero())
,last_output_(QuicTime::Zero())
,ref_time_(QuicTime::Zero()){
	sent_packet_manager_.SetHandshakeConfirmed();
	 versions_.push_back(ParsedQuicVersion(PROTOCOL_QUIC_CRYPTO, QUIC_VERSION_43));
}
MyQuicSender::~MyQuicSender(){
	if(f_rate_.is_open()){
		f_rate_.close();
	}
}
void MyQuicSender::EnableRateRecord(std::string name){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX))
			+"-"+name+"-rate.txt";
	enable_log_=true;
	f_rate_.open(path.c_str(), std::fstream::out);
}
void MyQuicSender::RecordRate(QuicTime now){
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
void MyQuicSender::OnIncomingData(uint8_t *data,int len){
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
bool MyQuicSender::OnAckFrame(const QuicAckFrame& frame){
	return false;
}
bool MyQuicSender::OnAckFrameStart(QuicPacketNumber largest_acked,
	                 QuicTime::Delta ack_delay_time){
	sent_packet_manager_.OnAckFrameStart(largest_acked,ack_delay_time,
			time_of_last_received_packet_);
	if(largest_acked>=largest_acked_){
		largest_acked_=largest_acked;
	}
	return true;
}
bool MyQuicSender::OnAckRange(QuicPacketNumber start,
	            QuicPacketNumber end,
	            bool last_range){
	sent_packet_manager_.OnAckRange(start, end);
	if (!last_range) {
	    return true;
	  }
	  bool acked_new_packet =
	      sent_packet_manager_.OnAckFrameEnd(time_of_last_received_packet_);
	  PostProcessAfterAckFrame(GetLeastUnacked() > start, acked_new_packet);
	  return true;
}
bool MyQuicSender::Process(){
	bool ret=true;
	QuicTime now=clock_.Now();
	{
	if(counter_<60){
		if(sent_packet_manager_.TimeUntilSend(now)==QuicTime::Delta::Zero()){
		SendFakePacket();
		counter_++;
		}
	}
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
	SendRetransmission();
	if(stop_waiting_count_>2){
		SendStopWaitingFrame();
	}
	RecordRate(now);
	return ret;
}
void MyQuicSender::set_duration(uint32_t ms){
	QuicTime now=clock_.Now();
	stop_=now+QuicTime::Delta::FromMilliseconds(ms);
}
void MyQuicSender::OnPacketSent(QuicPacketNumber packet_number,
                   QuicPacketNumberLength packet_number_length, QuicPacketLength encrypted_length){
	QuicTime now=clock_.Now();
	SerializedPacket info(packet_number,packet_number_length,NULL,encrypted_length,false,false);
	bool retransmittable =HAS_RETRANSMITTABLE_DATA;
	if (retransmittable == HAS_RETRANSMITTABLE_DATA) {
		QuicStreamFrame *stream=new QuicStreamFrame();
      		info.retransmittable_frames.push_back(
         	 QuicFrame(stream));
    	}
	sent_packet_manager_.OnPacketSent(&info,0,now, NOT_RETRANSMISSION,HAS_RETRANSMITTABLE_DATA);

}
void MyQuicSender::SendFakePacket(){
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
	uint8_t type=STREAM_FRAME;
	writer.WriteBytes(&type,1);
	seq_++;
	socket_->SendTo(&peer_,(uint8_t*)buf,kMaxPacketSize);
	uint16_t payload=kMaxPacketSize-(1+header.seq_len+1);
	OnPacketSent(header.seq,(QuicPacketNumberLength)header.seq_len,payload);
	//std::cout<<"new "<<header.seq<<std::endl;
}
void MyQuicSender::SendRetransmission(){
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
void MyQuicSender::OnRetransPacket(QuicPendingRetransmission pending){
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
	seq_++;
	uint8_t type=STREAM_FRAME;
	writer.WriteBytes(&type,1);
	socket_->SendTo(&peer_,(uint8_t*)buf,kMaxPacketSize);
	uint16_t payload=kMaxPacketSize-(1+header.seq_len+1);
	QuicTime now=clock_.Now();
	SerializedPacket info(header.seq,(QuicPacketNumberLength)header.seq_len,NULL,payload,false,false);
	//void QuicPacketCreator::ReserializeAllFrames
	for(const QuicFrame& frame : pending.retransmittable_frames){
		info.retransmittable_frames.push_back(frame);
	}
	uint64_t old_seq=pending. packet_number;
	sent_packet_manager_.OnPacketSent(&info,old_seq,now,LOSS_RETRANSMISSION,HAS_RETRANSMITTABLE_DATA);
	//std::cout<<"retrans "<<old_seq<<" new seq "<<header.seq<<std::endl;
}
void MyQuicSender::SendStopWaitingFrame(){
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
QuicPacketNumber MyQuicSender::GetLeastUnacked() const {
  return sent_packet_manager_.GetLeastUnacked();
}
void MyQuicSender::PostProcessAfterAckFrame(bool send_stop_waiting,
                                              bool acked_new_packet){
	  if (send_stop_waiting) {
	    ++stop_waiting_count_;
	  } else {
	    stop_waiting_count_ = 0;
	  }
}
}
