#include "my_quic_receiver.h"
#include "net/quic/core/quic_data_reader.h"
#include <iostream>
namespace net{
void MyQuicReceiver::OnIncomingPacket(char *data,int len){
	QuicDataReader reader(data,len, NETWORK_BYTE_ORDER);
	uint32_t seq=0;
	reader.ReadUInt32(&seq);
	std::cout<<"recv "<<len<<" seq "<<seq<<std::endl;
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
	if(counter_>8){
		ret=false;
	}
	return ret;
}
}



