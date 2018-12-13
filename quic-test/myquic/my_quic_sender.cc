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
namespace net{
const uint8_t kPublicHeaderSequenceNumberShift = 4;
uint8_t GetPacketNumberFlags(uint8_t seq_len) {
  switch (seq_len) {
    case PACKET_1BYTE_PACKET_NUMBER:
      return PACKET_FLAGS_1BYTE_PACKET;
    case PACKET_2BYTE_PACKET_NUMBER:
      return PACKET_FLAGS_2BYTE_PACKET;
    case PACKET_4BYTE_PACKET_NUMBER:
      return PACKET_FLAGS_4BYTE_PACKET;
    case PACKET_6BYTE_PACKET_NUMBER:
    case PACKET_8BYTE_PACKET_NUMBER:
      return PACKET_FLAGS_8BYTE_PACKET;
    default:
      return PACKET_FLAGS_8BYTE_PACKET;
  }
}
QuicPacketNumberLength GetMinSeqLength(uint64_t packet_number) {
  if (packet_number < 1 << (PACKET_1BYTE_PACKET_NUMBER * 8)) {
    return PACKET_1BYTE_PACKET_NUMBER;
  } else if (packet_number < 1 << (PACKET_2BYTE_PACKET_NUMBER * 8)) {
    return PACKET_2BYTE_PACKET_NUMBER;
  } else if (packet_number < UINT64_C(1) << (PACKET_4BYTE_PACKET_NUMBER * 8)) {
    return PACKET_4BYTE_PACKET_NUMBER;
  } else {
    return PACKET_6BYTE_PACKET_NUMBER;
  }
}
MyQuicSender::MyQuicSender(Perspective pespective)
:pespective_(pespective)
,sent_packet_manager_(pespective,&clock_,&stats_,kBBR,kNack)
,time_of_last_received_packet_(clock_.ApproximateNow())
,next_(QuicTime::Zero())
,stop_(QuicTime::Zero()){
	sent_packet_manager_.SetHandshakeConfirmed();
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
	return ;
}
bool MyQuicSender::OnAckFrame(const QuicAckFrame& frame){
	return false;
}
bool MyQuicSender::OnAckFrameStart(QuicPacketNumber largest_acked,
	                 QuicTime::Delta ack_delay_time){
	sent_packet_manager_.OnAckFrameStart(largest_acked,ack_delay_time,
			time_of_last_received_packet_);
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
	  return true;
}
bool MyQuicSender::Process(){
	bool ret=true;
	QuicTime now=clock_.Now();
	{
	if(counter_<40){
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
		if(stop_==QuicTime::Zero())
		{
		   stop_=clock_.Now()+QuicTime::Delta::FromMilliseconds(200);
		}	
	}
	if(stop_!=QuicTime::Zero()){
		if(now>stop_){
			ret=false;
		}
	}
	SendRetransmission();
	return ret;
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
	seq_++;
	socket_->SendTo(&peer_,(uint8_t*)buf,kMaxPacketSize);
	uint16_t payload=kMaxPacketSize-header.seq_len;
	OnPacketSent(header.seq,(QuicPacketNumberLength)header.seq_len,payload);
	
}
void MyQuicSender::SendRetransmission(){
	QuicTime now=clock_.Now();
	while(sent_packet_manager_.TimeUntilSend(now)==QuicTime::Delta::Zero()){
		if(sent_packet_manager_.HasPendingRetransmissions()){
			QuicPendingRetransmission pending=sent_packet_manager_.NextPendingRetransmission();
			OnRetransPacket(pending);
		}else{
			break;
		}
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
	socket_->SendTo(&peer_,(uint8_t*)buf,kMaxPacketSize);
	uint16_t payload=kMaxPacketSize-header.seq_len;
	QuicTime now=clock_.Now();
	SerializedPacket info(header.seq,(QuicPacketNumberLength)header.seq_len,NULL,payload,false,false);
	for(const QuicFrame& frame : pending.retransmittable_frames){
		info.retransmittable_frames.push_back(frame);
	}
	uint64_t old_seq=pending. packet_number;
	sent_packet_manager_.OnPacketSent(&info,old_seq,now,LOSS_RETRANSMISSION,HAS_RETRANSMITTABLE_DATA);
}
}



