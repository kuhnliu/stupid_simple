#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include "sender.h"
#include "fakevideosource.h"
#include "rtc_base/timeutils.h"
static int runTime=200000;
static int run_status=1;

char ip[]="10.0.0.1";
uint16_t port=1234;
char serv_addr[]="10.0.4.2:4321";

void signal_exit_handler(int sig)
{
	run_status=0;
}
int main(){
    signal(SIGTERM, signal_exit_handler);
	signal(SIGINT, signal_exit_handler);
	signal(SIGTSTP, signal_exit_handler);
	rtc::TaskQueue task_queue("test");
	zsy::VideoGenerator source(&task_queue,25);
	zsy::Sender sender;
	sender.SetEncoder(&source);
	sender.Bind(ip,port);
	sender.SetPeer(serv_addr);
	uint32_t now=rtc::TimeMillis();
	uint32_t Stop=now+runTime;
	std::string name("webrtc");
	//source.SetLogFile(name);
	sender.Start();
	while(run_status){
		sender.Process();
		now=rtc::TimeMillis();
		if(now>Stop){
			break;
		}
	}
	sender.Stop();
	sleep(1);
	return 0;
}




