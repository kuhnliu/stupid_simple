#include <type_traits>
#include "net/quic/core/quic_received_packet_manager.h"
#include "net/quic/core/quic_connection_stats.h"
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
return 0;
}



