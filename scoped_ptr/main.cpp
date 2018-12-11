#include "quic_arena_scoped_ptr.h"
#include <stdio.h>
#include <signal.h>
#include "quic_time.h"
#include "sys_clock.h"
#include "quic_alarm.h"
#include "null_alarm.h"
class Test{
public:
    Test(uint32_t i):i_(i){
    }
    ~Test(){
        printf("dtor %d\n",i_);
    }
private:
    uint32_t i_{0};
};
template<typename T>
void test_move(quic::QuicArenaScopedPtr<T> arg){
    quic::QuicArenaScopedPtr<T> a=std::move(arg);

}
quic::QuicArenaScopedPtr<Test> g_alarm;
void test_move2(quic::QuicArenaScopedPtr<Test> test){
    g_alarm=std::move(test);
}
void test_fun(){
    quic::QuicArenaScopedPtr<Test> alarm(new Test(1));
    test_move2(std::move(alarm));
    printf("test fun\n");
}
static uint32_t running=1;
void signal_exit_handler(int sig)
{
	running=0;
}
uint32_t run_duration=200;
int main(){
    signal(SIGTERM, signal_exit_handler);
	signal(SIGINT, signal_exit_handler);
    test_fun();
    printf("you know what\n");

    quic::SysClock clock;
    //quic::QuicTime time=clock.Now();
    //printf("%lld\n",time.ToDebuggingValue());
    //uint64_t now=iclock64();
    //printf("%lld\n",now*1000);
    quic::QuicTime now=clock.Now();
    quic::QuicTime Stop=now+
    quic::QuicTime::Delta::FromMilliseconds(run_duration);
    quic::NullAlarm alarm(quic::QuicArenaScopedPtr<quic::QuicAlarm::Delegate>(new quic::NullDelegate()));
    quic::QuicTime future=now+
    quic::QuicTime::Delta::FromMilliseconds(10);
    alarm.Set(future);
    while(running){
        now=clock.Now();
        if(now>Stop){
            break;
        }
        if(alarm.IsSet()){
            if(now>alarm.deadline()){
                alarm.Trigger();
                future=now+
                quic::QuicTime::Delta::FromMilliseconds(10);
                alarm.Set(future);
            }
        }
    }
    return 0;
}
