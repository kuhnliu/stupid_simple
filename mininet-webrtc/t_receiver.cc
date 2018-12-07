#include <stdint.h>
#include <signal.h>
#include "receiver.h"

static int runTime=5000;
static int run_status=1;
char ip[]="127.0.0.1";
uint16_t port=4321;
void signal_exit_handler(int sig)
{
	run_status=0;
}
int main(){
    signal(SIGTERM, signal_exit_handler);
	signal(SIGINT, signal_exit_handler);
	signal(SIGTSTP, signal_exit_handler);
	zsy::Receiver receiver;
	receiver.Bind(ip,port);
	receiver.SetRunDuration(runTime);
	while(run_status&&!receiver.IsStop()){
		receiver.Process();
	}
	return 0;
}




