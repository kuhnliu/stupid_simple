#ifndef MY_QUIC_RECEIVER_H_
#define MY_QUIC_RECEIVER_H_
#include "socket.h"
namespace net{
class MyQuicReceiver{
public:
	void OnIncomingPacket(char* data,int len);
	bool Process();
	void set_socket(zsy::Socket *socket) { socket_=socket;}
private:
	bool first_{true};
	uint32_t counter_{0};
	zsy::Socket *socket_{NULL};
	su_addr peer_;
};
}




#endif /* MY_QUIC_RECEIVER_H_ */
