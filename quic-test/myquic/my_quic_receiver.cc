#include "my_quic_receiver.h"
#include "net/quic/core/quic_data_reader.h"
#include "net/quic/core/quic_data_writer.h"
#include "my_quic_header.h"
#include "my_quic_framer.h"
#include "quic_framer_visitor.h"
#include <iostream>
namespace net{
const uint8_t kPublicHeaderSequenceNumberShift = 4;
QuicPacketNumberLength ReadSequenceNumberLength(uint8_t flags) {
  switch (flags & PACKET_FLAGS_8BYTE_PACKET) {
    case PACKET_FLAGS_8BYTE_PACKET:
      return PACKET_6BYTE_PACKET_NUMBER;
    case PACKET_FLAGS_4BYTE_PACKET:
      return PACKET_4BYTE_PACKET_NUMBER;
    case PACKET_FLAGS_2BYTE_PACKET:
      return PACKET_2BYTE_PACKET_NUMBER;
    case PACKET_FLAGS_1BYTE_PACKET:
      return PACKET_1BYTE_PACKET_NUMBER;
    default:
      return PACKET_6BYTE_PACKET_NUMBER;
  }
}
MyQuicReceiver::MyQuicReceiver():recv_packet_manager_(&stats_){
	versions_.push_back(ParsedQuicVersion( PROTOCOL_QUIC_CRYPTO, QUIC_VERSION_43));
}
void MyQuicReceiver::OnIncomingPacket(char *data,int len){
	QuicDataReader reader(data,len, NETWORK_BYTE_ORDER);
	my_quic_header_t header;
	uint8_t public_flags=0;
	reader.ReadBytes(&public_flags,1);
	header.seq_len= ReadSequenceNumberLength(
        public_flags >> kPublicHeaderSequenceNumberShift);	
	uint64_t seq=0;
	reader.ReadBytesToUInt64(header.seq_len,&seq);
	//std::cout<<"recv "<<len<<" seq "<<seq<<std::endl;
	QuicTime now=clock_.Now();
	QuicPacketHeader fakeheader;
	fakeheader.packet_number=seq;
	if(seq==5||seq==9){
		std::cout<<"overlook "<<seq<<std::endl;
	}else{
	  recv_packet_manager_.RecordPacketReceived(fakeheader,now);
	}
	SendAck();
	counter_++;
}
bool MyQuicReceiver::Process(){
	bool ret=true;
	uint32_t max_len=1500;
	char buf[1500]={0};
	su_addr remote;
	int recv=0;
	recv=socket_->RecvFrom(&remote,(uint8_t*)buf,max_len);
	if(recv>0){
		if(first_){
			peer_=remote;
			first_=false;
		}
		OnIncomingPacket(buf,recv);
	}
	if(counter_>20){
		ret=false;
	}
	return ret;
}
void MyQuicReceiver::SendAck(){
	if(counter_>7/*&&!ack_sent_*/){
	QuicTime approx=clock_.Now();
	QuicFrame frame=recv_packet_manager_.GetUpdatedAckFrame(approx);
	QuicAckFrame ackframe(*frame.ack_frame);
 	char buffer[kMaxPacketSize];
 	uint32_t packet_length=kMaxPacketSize;
 	QuicDataWriter writer(packet_length, buffer, NETWORK_BYTE_ORDER);
	MyQuicFramer framer(versions_,approx,Perspective::IS_CLIENT);
 	framer.AppendAckFrameAndTypeByte(ackframe,&writer);
	uint32_t length=writer.length();
	socket_->SendTo(&peer_,(uint8_t*)buffer,length);
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



