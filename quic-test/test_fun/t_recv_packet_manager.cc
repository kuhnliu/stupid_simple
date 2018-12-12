#include <type_traits>
#include "net/quic/core/quic_received_packet_manager.h"
#include "net/quic/core/quic_connection_stats.h"
#include "net/quic/core/quic_data_writer.h"
#include "net/quic/core/quic_data_reader.h"
#include "net/quic/core/quic_versions.h"
#include "net/quic/core/quic_types.h"
#include "my_quic_framer.h"
#include "quic_framer_visitor.h"
#include <iostream>
#include <string>
#include <stdio.h>
using namespace std;
using namespace net;
uint32_t delta=10;
int main(){
QuicTime now=QuicTime::Zero()+
		QuicTime::Delta::FromMilliseconds(1234);
QuicConnectionStats stats;
QuicReceivedPacketManager manager(&stats);
int i=0;
QuicPacketHeader header;
for(i=0;i<10;i++){
	header.packet_number=(i+1);
        if(i==5){
          continue;
	}
	QuicTime offset=now+
			QuicTime::Delta::FromMilliseconds(i*delta);
	manager.RecordPacketReceived(header,offset);
}
QuicTime approx=now+
		QuicTime::Delta::FromMilliseconds(11*delta);
QuicFrame frame=manager.GetUpdatedAckFrame(approx);
printf("%d\n",frame.type);
if(frame.ack_frame->received_packet_times.size()){
	for(auto it=frame.ack_frame->received_packet_times.begin();
	it!=frame.ack_frame->received_packet_times.end();it++){
		std::cout<<it->first<<" "<<it->second.ToDebuggingValue()<<std::endl;
	}
}

 QuicAckFrame ackframe(*frame.ack_frame);
  //std::ostringstream stream;
 char buffer[kMaxPacketSize];
 uint32_t packet_length=kMaxPacketSize;
 QuicDataWriter writer(packet_length, buffer, NETWORK_BYTE_ORDER);
 ParsedQuicVersionVector  versions;
 versions.push_back(ParsedQuicVersion( PROTOCOL_QUIC_CRYPTO, QUIC_VERSION_43));
 QuicTime recv_time=approx+QuicTime::Delta::FromMilliseconds(5*delta);
 MyQuicFramer framer(versions,recv_time,Perspective::IS_CLIENT);
 framer.AppendAckFrameAndTypeByte(ackframe,&writer);
 uint32_t length=writer.length();
 printf("size %d\n",length);
 uint8_t byte=buffer[0];
 printf("%x\n",byte);
 uint64_t seq;
/*
 if((byte&0x06)==0){
	byte=buffer[1];
	seq=byte;
	printf("%d\n",seq);
	}*/
char rbuf[kMaxPacketSize]={0};
memcpy(rbuf,buffer,length);
QuicDataReader reader(rbuf,length, NETWORK_BYTE_ORDER);
QuicTime sender_time=approx+QuicTime::Delta::FromMilliseconds(20*delta);
MyQuicFramer readframer(versions,sender_time,Perspective::IS_CLIENT);
AckFrameVisitor visitor;
readframer.set_visitor(&visitor);
readframer.ProcessFrameData(&reader,header);
return 0;
}



