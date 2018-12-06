#include "rtc_base/location.h"
#include "rtc_base/timeutils"
#include "modules/include/module.h"
#include "system_wrappers/include/clock.h"

#include <stdio.h>
#include <signal.h>
#include <memory>
class TestModule public webrtc::Module{
public:
    TestModule(){
        clock_= Clock::GetRealTimeClock();
    }
    ~TestModule(){
        if(clock_){
            delete clock_;
        }
    }
int64_t TimeUntilNextProcess() override{
    int64_t ret=10;
    if(first_){
        first_=false;
       ret=0;
    }
    return ret;
}
void Process(){
    int64_t now=0;
    uint32_t delta=0;
    if(clock_){
        now=clock_->TimeInMilliSeconds();
    }
    if(last_==0){
        last_=now;
    }
    delta=now-last;
    last_=now;
    printf("hello %d\n",delta);
}    
private:
    webrtc::Clock *clock_{NULL};
    bool first_{true};
    int64_t last_{0}
}

static runTime=100;
static int run_status=1;
void signal_exit_handler(int sig)
{
	run_status=0;
}
int main(){
    signal(SIGTERM, signal_exit_handler);
	signal(SIGINT, signal_exit_handler);
	signal(SIGTSTP, signal_exit_handler);
    TestModule module;
    std::unique_ptr<webrtc::ProcessThread> process_;
    process_=ProcessThread::Create("TestThread");
    process_->RegisterModules(&module,rtc::RTC_FROM_HERE);
    uint32_t now=rtc::TimeMillis();
    uint32_t stop=now+runTime;
    process_.Start();
    while(run_status){
        now=rtc::TimeMillis();
        if(now>stop){
            break;
        }
    }
    process_->Stop();
    process_->DeRegisterModule(&module);
    return 0;
}