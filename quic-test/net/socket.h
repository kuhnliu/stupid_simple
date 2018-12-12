#ifndef SOCKET_H_
#define SOCKET_H_
#include "cf_platform.h"
namespace zsy{
class Socket{
public:
	virtual ~Socket(){}
	virtual void Bind(char *ip,uint16_t port) {}
	virtual void Listen() {}
	virtual int Send(uint8_t *buf,uint32_t len) {return 0;}
	virtual int Recv(uint8_t *buf,uint32_t len) {return 0;}
	virtual int RecvFrom(su_addr*peer,uint8_t *buf,uint32_t len) { return 0;}
	virtual int SendTo(su_addr*peer,uint8_t *buf,uint32_t len) {return 0;}
};
class UdpSocket:public Socket{
public:
	~UdpSocket();
	void Bind(char *ip,uint16_t port) override;
	int RecvFrom(su_addr*peer,uint8_t *buf,uint32_t len) override;
	int SendTo(su_addr*peer,uint8_t *buf,uint32_t len) override;
private:
	su_socket fd_;
};
}
#endif /* SOCKET_H_ */
