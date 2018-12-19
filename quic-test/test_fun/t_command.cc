#include "cmdline.h"

#include <iostream>
using namespace std;

int main(int argc, char *argv[]){
    cmdline::parser parser;
    parser.add<std::string>("client", 'h', "host name", false, "127.0.0.1");
	parser.add<int>("cp",'c',"client port",false,1234);
    parser.add<std::string>("server", 's', "host name", false, "127.0.0.1");
	parser.add<int>("sp",'p',"server port",false,4321);
    parser.add<std::string>("log", 'l', "log", false, "1");
    parser.parse_check(argc, argv);
    std::string client_ip=parser.get<std::string>("client");
    std::string serv_ip=parser.get<std::string>("server");
    uint16_t c_port=parser.get<int>("cp");
    uint16_t s_port=parser.get<int>("sp");
    std::cout<<client_ip<<":"<<c_port<<" "<<serv_ip<<":"<<s_port<<std::endl;
    return 0;
}
