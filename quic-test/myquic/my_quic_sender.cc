#include "my_quic_sender.h"
#include "net/quic/core/quic_data_reader.h"
#include "net/quic/core/quic_data_writer.h"
#include <memory.h>
#include <iostream>
namespace net{
MyQuicSender::MyQuicSender(Perspective pespective)
:pespective_(pespective)
,sent_packet_manager_(pespective,&clock_,&stats_,kBBR,kNack)
,time_of_last_received_packet_(clock_.ApproximateNow())
,next_(QuicTime::Zero()){
	 versions_.push_back(ParsedQuicVersion(PROTOCOL_QUIC_CRYPTO, QUIC_VERSION_43));
}
void MyQuicSender::OnIncomingData(uint8_t *data,int len){
	QuicTime now=clock_.Now();
	time_of_last_received_packet_=now;
	char buf[kMaxPacketSize]={0};
	memcpy(buf,data,len);
	QuicDataReader reader(buf,len, NETWORK_BYTE_ORDER);
	MyQuicFramer framer(versions_,now,pespective_);
	framer.set_visitor(this);
	QuicPacketHeader header;// no use
	framer.ProcessFrameData(&reader,header);
}
bool MyQuicSender::OnAckFrame(const QuicAckFrame& frame){
	std::cout<<"should not happen";
	return false;
}
bool MyQuicSender::OnAckFrameStart(QuicPacketNumber largest_acked,
	                 QuicTime::Delta ack_delay_time){

	sent_packet_manager_.OnAckFrameStart(largest_acked,ack_delay_time,
			time_of_last_received_packet_);
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
	  return true;
}
bool MyQuicSender::Process(){
	bool ret=true;
	QuicTime now=clock_.Now();
	if(now>next_){
		SendFakePacket();
		next_=now+QuicTime::Delta::FromMilliseconds(10);
		counter_++;
	}
	if(counter_>10){
		ret=false;
	}
	return ret;
}
bool MyQuicSender::OnPacketSent(SerializedPacket* serialized_packet,
                  	  	  	  	QuicPacketNumber original_packet_number,
								QuicTime sent_time,
								TransmissionType transmission_type,
								HasRetransmittableData has_retransmittable_data){

}
void MyQuicSender::SendFakePacket(){
	char buf[kMaxPacketSize]={0};
	QuicDataWriter writer(kMaxPacketSize, buf, NETWORK_BYTE_ORDER);
	writer.WriteUInt32(seq_);
	seq_++;
	socket_->SendTo(&peer_,(uint8_t*)buf,kMaxPacketSize);
}
}


