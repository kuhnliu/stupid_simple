#include "rtc_base/timeutils.h"
#include "fakevideosource.h"
#include "test_sender.h"
#include <signal.h>

static int runTime=500;
static int run_status=1;
void signal_exit_handler(int sig)
{
	run_status=0;
}
int main(){
    signal(SIGTERM, signal_exit_handler);
	signal(SIGINT, signal_exit_handler);
	signal(SIGTSTP, signal_exit_handler);
	rtc::TaskQueue task_queue("test");
	uint32_t bw=1000000;
	zsy::VideoGenerator source(&task_queue,25);
	source.SetMinRate(bw);
	zsy::TestSender sender(&source);
	uint32_t now=rtc::TimeMillis();
	uint32_t Stop=now+runTime;
	while(run_status){
		now=rtc::TimeMillis();
		if(now>Stop){
			break;
		}
	}
	sender.Stop();
	return 0;
}




