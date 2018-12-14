#include <signal.h>
#include <string>
#include "my_quic_sender.h"
using namespace net;
using namespace zsy;
static char ip[]="10.0.0.1";
uint16_t port=1234;
static char addr[]="10.0.4.2:4321";
static int run_status=1;
void signal_exit_handler(int sig)
{
	run_status=0;
}
int main(){
	signal(SIGTERM, signal_exit_handler);
	signal(SIGINT, signal_exit_handler);
	signal(SIGTSTP, signal_exit_handler);
	UdpSocket socket;
	socket.Bind(ip,port);
	su_addr peer;
	su_string_to_addr(&peer,addr);
	MyQuicSender sender(Perspective::IS_CLIENT);
	sender.set_peer(peer);
	sender.set_socket(&socket);
	sender.set_duration(20000);
	std::string name=std::string("quic-bbr");
	sender.EnableRateRecord(name);
	while(sender.Process()&&run_status){

	}
	return 0;
}




