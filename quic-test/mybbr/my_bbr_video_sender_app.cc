#include "my_bbr_video_sender_app.h"
#include "my_proto.h"
#include "net/quic/core/quic_data_reader.h"
#include "net/quic/core/quic_data_writer.h"
#include <unistd.h>
#include "my_quic_header.h"
#include "my_quic_utils.h"
#include "fakevideogenerator.h"
#include <iostream>
const uint8_t kPublicHeaderSequenceNumberShift = 4;
const uint32_t MaxPacketSize=1400;
namespace net{
MyBbrVideoSenderApp::MyBbrVideoSenderApp(uint64_t min_bps,
		uint64_t start_bps,
		uint64_t max_bps)
:next_(QuicTime::Zero())
,ref_time_(QuicTime::Zero())
,stop_(QuicTime::Zero())
,cc_(min_bps,start_bps,max_bps){
	pacing_.set_sender(&cc_);
}
MyBbrVideoSenderApp::~MyBbrVideoSenderApp(){
	if(f_rate_.is_open()){
		f_rate_.close();
	}
}
void MyBbrVideoSenderApp::set_duration(uint32_t ms){
    QuicTime now=clock_.Now();
    stop_=now+QuicTime::Delta::FromMilliseconds(ms);
}
bool MyBbrVideoSenderApp::Process(){
	bool ret=true;
	QuicTime now=clock_.Now();
	if(ref_time_==QuicTime::Zero()){
		ref_time_=now;
	}
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
	if(pacing_.TimeUntilSend(now)==QuicTime::Delta::Zero()){
		SendDataOrPadding(now);
	}
	int recv=0;
	su_addr remote;
	uint32_t max_len=1500;
	char buf[1500]={0};
	recv=socket_->RecvFrom(&remote,(uint8_t*)buf,max_len);
	if(recv>0){
		ProcessAckFrame(now,(uint8_t*)buf,recv);
	}
	if(stop_!=QuicTime::Zero()){
		if(now>stop_){
			ret=false;
		}
	}
	RecordRate(now);
	return ret;
}
void MyBbrVideoSenderApp::EnableRateRecord(std::string name){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX))
			+"/"+name+"-rate.txt";
	enable_log_=true;
	f_rate_.open(path.c_str(), std::fstream::out);
}
void MyBbrVideoSenderApp::RecordRate(QuicTime now){
	if(next_==QuicTime::Zero()){
		next_=now+QuicTime::Delta::FromMilliseconds(100);
		return;
	}
	if(now>next_){
		int64_t bw=0;
		QuicTime::Delta delta=now-ref_time_;
		int64_t ms=delta.ToMilliseconds()+offset_;
		bw=cc_.BandwidthEstimate().ToKBitsPerSecond();
		if(f_rate_.is_open()){
			char line [256];
			memset(line,0,256);
			sprintf (line, "%lld %16lld",ms,bw);
			f_rate_<<line<<std::endl;
		}
		next_=now+QuicTime::Delta::FromMilliseconds(100);
	}
}
void MyBbrVideoSenderApp::SendDataOrPadding(QuicTime now){
	if(pending_queue_.empty()){
		if(cc_.ShouldSendProbePacket()){
			SendPaddingPacket(now);
		}
	}else{
		SendFakePacket(now);
	}
}
void MyBbrVideoSenderApp::SendFakePacket(QuicTime now){
	char buf[MaxPacketSize]={0};
	my_quic_header_t header;
	header.seq=seq_;
	header.seq_len=GetMinSeqLength(header.seq);
	uint8_t public_flags=0;
	public_flags |= GetPacketNumberFlags(header.seq_len)
                  << kPublicHeaderSequenceNumberShift;
	QuicDataWriter writer(MaxPacketSize, buf, NETWORK_BYTE_ORDER);
	uint8_t type=TestProto::test_proto_stream;
	public_flags|=type;
	writer.WriteBytes(&public_flags,1);
	writer.WriteBytesToUInt64(header.seq_len,header.seq);
	seq_++;
	socket_->SendTo(&peer_,(uint8_t*)buf,kMaxPacketSize);
	uint16_t payload=MaxPacketSize-(1+header.seq_len);
	pacing_.OnPacketSent(now,header.seq,payload);
}
void MyBbrVideoSenderApp::SendPaddingPacket(QuicTime now){
	char buf[MaxPacketSize]={0};
	my_quic_header_t header;
	header.seq=seq_;
	header.seq_len=GetMinSeqLength(header.seq);
	uint8_t public_flags=0;
	public_flags |= GetPacketNumberFlags(header.seq_len)
                  << kPublicHeaderSequenceNumberShift;
	QuicDataWriter writer(MaxPacketSize, buf, NETWORK_BYTE_ORDER);
	uint8_t type=TestProto::test_proto_padding;
	public_flags|=type;
	writer.WriteBytes(&public_flags,1);
	writer.WriteBytesToUInt64(header.seq_len,header.seq);
	seq_++;
	socket_->SendTo(&peer_,(uint8_t*)buf,kMaxPacketSize);
	uint16_t payload=MaxPacketSize-(1+header.seq_len);
	pacing_.OnPacketSent(now,header.seq,payload);
}
void MyBbrVideoSenderApp::ProcessAckFrame(QuicTime now,uint8_t *data,int len){
	char buf[MaxPacketSize]={0};
	memcpy(buf,data,len);
	my_quic_header_t header;
	uint8_t public_flags=0;
	QuicDataReader header_reader(buf,len,NETWORK_BYTE_ORDER);
	header_reader.ReadBytes(&public_flags,1);
	uint8_t type=public_flags&0x0F;
	header.seq_len= ReadSequenceNumberLength(
        public_flags >> kPublicHeaderSequenceNumberShift);
	if(type==TestProto::test_proto_ack){
		uint64_t num=0;
		header_reader.ReadBytesToUInt64(header.seq_len,&num);
		pacing_.OnCongestionEvent(now,num);
	}
}
}





