#include "my_quic_receiver.h"
#include <signal.h>
using namespace net;
using namespace zsy;
static char serv_ip[]="10.0.4.2";
uint16_t serv_port=4321;
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
	socket.Bind(serv_ip,serv_port);
	MyQuicReceiver receiver;
	receiver.set_socket(&socket);
	receiver.set_duration(21000);
	while(receiver.Process()&&run_status){

	}
	return 0;
}

