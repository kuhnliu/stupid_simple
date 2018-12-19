#include "my_quic_receiver.h"
#include <signal.h>
#include <string>
#include "cmdline.h"
using namespace net;
using namespace zsy;
static int run_status=1;
void signal_exit_handler(int sig)
{
	run_status=0;
}
int main(int argc,char *argv[]){
	signal(SIGTERM, signal_exit_handler);
	signal(SIGINT, signal_exit_handler);
	signal(SIGTSTP, signal_exit_handler);
    cmdline::parser parser;
    parser.add<std::string>("host", 'h', "host name", false, "10.0.4.2");
    parser.add<int>("port", 'p', "port number", false,4321, cmdline::range(1,65535));
     parser.add<int>("duration", 'd', "duration", false, 110000);//110 s
    parser.parse_check(argc, argv);
    std::string serv_ip=parser.get<std::string>("host");
    uint16_t serv_port=parser.get<int>("port");
    int duration=parser.get<int>("duration");
	UdpSocket socket;
	socket.Bind(const_cast<char*>(serv_ip.c_str()),serv_port);
	MyQuicReceiver receiver;
	receiver.set_socket(&socket);
	receiver.set_duration(duration);
	while(receiver.Process()&&run_status){

	}
	return 0;
}

