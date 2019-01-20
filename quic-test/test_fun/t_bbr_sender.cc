#include <signal.h>
#include <string>
#include "cmdline.h"
#include "my_bbr_sender_app.h"
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
    parser.add<std::string>("client", 'h', "host name", false, "10.0.0.1");
	parser.add<int>("cp",'c',"client port",false,1234);
    parser.add<std::string>("server", 's', "host name", false, "10.0.4.2");
	parser.add<int>("sp",'p',"server port",false,4321);
    parser.add<std::string>("log", 'l', "log", false, "1");
    parser.add<int>("duration", 'd', "duration", false, 200000);//1 s
    parser.add<int>("offset", 'o', "time offset, MilliSeconds", false,0);
    parser.parse_check(argc, argv);
    std::string client_ip=parser.get<std::string>("client");
    std::string serv_ip=parser.get<std::string>("server");
    uint16_t c_port=parser.get<int>("cp");
    uint16_t s_port=parser.get<int>("sp");
    std::string test_case=parser.get<std::string>("log");
    int duration=parser.get<int>("duration");
    int offset=parser.get<int>("offset");
	UdpSocket socket;
	socket.Bind(const_cast<char*>(client_ip.c_str()),c_port);
	su_addr peer;
	std::string serv_port=serv_ip+":"+std::to_string(s_port);
	char *ip_port=const_cast<char*>(serv_port.c_str());
	su_string_to_addr(&peer,ip_port);
	MyBbrSenderApp sender(500000,1000000,2000000);
	sender.set_peer(peer);
	sender.set_socket(&socket);
	sender.set_duration(duration);
    sender.SetTimeOffset(offset);
	std::string name=test_case+std::string("-my-bbr");
	sender.SetRecordPrefix(name);
    sender.EnableRateRecord();
    sender.EnableLossRecord();
	while(sender.Process()&&run_status){

	}
   // sender.PrintPacingInfo();
	return 0;
}




