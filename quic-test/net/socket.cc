#include "socket.h"
namespace zsy{
UdpSocket::~UdpSocket(){
	su_socket_destroy(fd_);
}
void UdpSocket::Bind(char *ip,uint16_t port){
	su_udp_create(ip,port,&fd_);
}
int UdpSocket::RecvFrom(su_addr*peer,uint8_t *buf,uint32_t len){
	return su_udp_recv(fd_,peer,(void*)buf,len,0);
}
int UdpSocket::SendTo(su_addr*peer,uint8_t *buf,uint32_t len){
	return su_udp_send(fd_,peer,(void*)buf,len);
}
}
