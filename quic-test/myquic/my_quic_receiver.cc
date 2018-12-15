#include "my_quic_receiver.h"
#include "net/quic/core/quic_data_reader.h"
#include "net/quic/core/quic_data_writer.h"
#include "my_quic_header.h"
#include "my_quic_framer.h"
#include "quic_framer_visitor.h"
#include <iostream>
namespace net{
const uint8_t kPublicHeaderSequenceNumberShift = 4;
MyQuicReceiver::MyQuicReceiver()
:recv_packet_manager_(&stats_)
,stop_(QuicTime::Zero()){
	versions_.push_back(ParsedQuicVersion( PROTOCOL_QUIC_CRYPTO, QUIC_VERSION_43));
}
void MyQuicReceiver::OnIncomingData(char *data,int len){
	QuicDataReader reader(data,len, NETWORK_BYTE_ORDER);
	my_quic_header_t header;
	uint8_t public_flags=0;
	reader.ReadBytes(&public_flags,1);
	header.seq_len= ReadSequenceNumberLength(
        public_flags >> kPublicHeaderSequenceNumberShift);
	uint64_t seq=0;
	reader.ReadBytesToUInt64(header.seq_len,&seq);
	//uint32_t header_len=sizeof(uint8_t)+header.seq_len;
	uint8_t type=0;
	reader.ReadBytes(&type,1);
	if(type==STREAM_FRAME){
		QuicTime now=clock_.Now();
		QuicPacketHeader fakeheader;
		fakeheader.packet_number=seq;
		/*if(seq==5||seq==9){
			std::cout<<"overlook "<<seq<<std::endl;
		}else*/{
		  //std::cout<<"recv "<<seq<<std::endl;
		  recv_packet_manager_.RecordPacketReceived(fakeheader,now);
		  if(seq!=base_seq_+1){
			//std::cout<<"l "<<base_seq_+1<<std::endl;
		  }
		  base_seq_=seq;
		}
		SendAck();
		counter_++;
	}
	if(type==STOP_WAITING_FRAME){
		QuicPacketNumber least_unack;
		reader.ReadBytesToUInt64(header.seq_len,&least_unack);
		recv_packet_manager_.DontWaitForPacketsBefore(least_unack);
		//std::cout<<"stop waititng "<<least_unack<<std::endl;
	}
}
bool MyQuicReceiver::Process(){
	bool ret=true;
	uint32_t max_len=1500;
	char buf[1500]={0};
	su_addr remote;
	int recv=0;
	recv=socket_->RecvFrom(&remote,(uint8_t*)buf,max_len);
	QuicTime now=clock_.Now();
	if(recv>0){
		OnIncomingData(buf,recv);
		if(first_){
			peer_=remote;
			first_=false;
			stop_=now+QuicTime::Delta::FromMilliseconds(duration_);
		}
	}
	if(!first_){
		if(now>stop_){
			ret=false;
		}
	}
	return ret;
}
void MyQuicReceiver::SendAck(){
	/*if(counter_>7&&ack_sent_)*/{
	QuicTime approx=clock_.Now();
	QuicFrame frame=recv_packet_manager_.GetUpdatedAckFrame(approx);
	QuicAckFrame ackframe(*frame.ack_frame);
 	char buffer[kMaxPacketSize];
	my_quic_header_t header;
	header.seq=seq_;
	header.seq_len=GetMinSeqLength(header.seq);
	uint8_t public_flags=0;
	public_flags |= GetPacketNumberFlags(header.seq_len)
                  << kPublicHeaderSequenceNumberShift;

	uint32_t header_len=sizeof(uint8_t)+header.seq_len;
	QuicDataWriter header_writer(header_len,buffer,NETWORK_BYTE_ORDER);
	header_writer.WriteBytes(&public_flags,1);
	header_writer.WriteBytesToUInt64(header.seq_len,seq_);
 	uint32_t packet_length=kMaxPacketSize-header_len;
 	QuicDataWriter writer(packet_length, buffer+header_len, NETWORK_BYTE_ORDER);
	MyQuicFramer framer(versions_,approx,Perspective::IS_CLIENT);
 	framer.AppendAckFrameAndTypeByte(ackframe,&writer);
	uint32_t total_len=writer.length()+header_len;
	socket_->SendTo(&peer_,(uint8_t*)buffer,total_len);
	ack_sent_=true;	
	/*
	QuicPacketHeader header;
	char rbuf[kMaxPacketSize]={0};
	memcpy(rbuf,buffer,length);
	std::cout<<"ack len "<<length<<std::endl;
	QuicDataReader reader(rbuf,length, NETWORK_BYTE_ORDER);
	MyQuicFramer readframer(versions_,approx,Perspective::IS_CLIENT);
	AckFrameVisitor visitor;
	readframer.set_visitor(&visitor);
	readframer.ProcessFrameData(&reader,header);*/
	}
}
}



