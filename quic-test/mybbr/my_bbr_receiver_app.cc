#include "my_bbr_receiver_app.h"
#include "my_proto.h"
#include "my_quic_header.h"
#include "my_quic_framer.h"
#include "net/quic/core/quic_data_reader.h"
#include "net/quic/core/quic_data_writer.h"
#include <unistd.h>
#include <iostream>
namespace net{
const uint8_t kPublicHeaderSequenceNumberShift = 4;
const uint32_t MaxPacketSize=1400;
MyBbrReceiverApp::MyBbrReceiverApp()
:stop_(QuicTime::Zero())
,ref_time_(QuicTime::Zero())
,next_(QuicTime::Zero()){

}
MyBbrReceiverApp::~MyBbrReceiverApp(){
    if(f_rate_.is_open()){
        f_rate_.close();
    }
    if(f_loss_.is_open()){
        f_loss_.close();
    }
}
bool MyBbrReceiverApp::Process(){
	bool ret=true;
	uint32_t max_len=1500;
	char buf[1500]={0};
	su_addr remote;
	int recv=0;
	recv=socket_->RecvFrom(&remote,(uint8_t*)buf,max_len);
	QuicTime now=clock_.Now();
	if(recv>0){
	    if(ref_time_==QuicTime::Zero()){
	        ref_time_=now;
	    }
		if(first_){
			peer_=remote;
			first_=false;
			stop_=now+QuicTime::Delta::FromMilliseconds(duration_);
		}
		OnIncomingData(buf,recv);
	}
	if(!first_){
		if(now>stop_){
			ret=false;
		}
	}
	RecordRate(now);
	return ret;
}
void MyBbrReceiverApp::SetRecordPrefix(std::string name){
	log_prefix_=name;
}
void MyBbrReceiverApp::EnableRateRecord(){
    enable_rate_record_=true;
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX))
			+"/"+log_prefix_+"-rate.txt";
	f_rate_.open(path.c_str(), std::fstream::out);
}
void MyBbrReceiverApp::EnableLossRecord(){
    enable_loss_record_=true;
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX))
			+"/"+log_prefix_+"-loss.txt";
	f_loss_.open(path.c_str(), std::fstream::out);
}
void MyBbrReceiverApp::OnIncomingData(char *data, int len){
	recv_byte_+=len;
	QuicDataReader reader(data,len, NETWORK_BYTE_ORDER);
	my_quic_header_t header;
	uint8_t public_flags=0;
	reader.ReadBytes(&public_flags,1);
	header.seq_len= ReadSequenceNumberLength(
        public_flags >> kPublicHeaderSequenceNumberShift);
	uint8_t type=public_flags&0x0F;
	if(type==TestProto::test_proto_stream||type==TestProto::test_proto_padding){
		uint64_t seq=0;
		reader.ReadBytesToUInt64(header.seq_len,&seq);
		SendAck(seq);
	if(seq>base_seq_+1){
		uint64_t i=0;
		for(i=base_seq_+1;i<seq;i++){
			RecordLoss(i);
		}
	  }
	  if(seq>=base_seq_+1){
		  base_seq_=seq;
	  }
	}
}
void MyBbrReceiverApp::SendAck(uint64_t packet_number){
	char buf[MaxPacketSize]={0};
	my_quic_header_t header;
	header.seq=packet_number;
	header.seq_len=GetMinSeqLength(header.seq);
	uint8_t public_flags=0;
	public_flags |= GetPacketNumberFlags(header.seq_len)
                  << kPublicHeaderSequenceNumberShift;
	QuicDataWriter writer(MaxPacketSize, buf, NETWORK_BYTE_ORDER);
	uint8_t type=TestProto::test_proto_ack;
	public_flags|=type;
	writer.WriteBytes(&public_flags,1);
	writer.WriteBytesToUInt64(header.seq_len,header.seq);
	uint32_t total_len=writer.length();
	socket_->SendTo(&peer_,(uint8_t*)buf,total_len);
    //std::cout<<"ack "<<packet_number<<std::endl;
}
void MyBbrReceiverApp::RecordLoss(uint64_t seq){
    if(enable_loss_record_){
    	QuicTime now=clock_.Now();
		QuicTime::Delta delta=now-ref_time_;
		uint64_t abs=delta.ToMilliseconds()+offset_;
		char line [256];
		memset(line,0,256);
		sprintf (line, "%lld %16lld",abs,seq);
		f_loss_<<line<<std::endl;
    }
}
void MyBbrReceiverApp::RecordRate(QuicTime now){
	if(enable_rate_record_){
		if(next_==QuicTime::Zero()){
			next_=now+QuicTime::Delta::FromMilliseconds(500);
			return;
		}
		if(now>next_){
			uint64_t kbps=recv_byte_*8/(500);
			next_=now+QuicTime::Delta::FromMilliseconds(500);
			recv_byte_=0;
			QuicTime::Delta delta=now-ref_time_;
			uint64_t abs=delta.ToMilliseconds()+offset_;
			char line [256];
			memset(line,0,256);
			sprintf (line, "%lld %16lld",abs,kbps);
			f_rate_<<line<<std::endl;
		}
	}
}
}




