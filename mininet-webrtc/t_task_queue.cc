#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include<stdio.h>
#include <string>
#include <iostream>
#include "rtc_base/task_queue.h"
#include "rtc_base/checks.h"
#include "rtc_base/platform_thread.h"
static int run=1;
void signal_exit_handler(int sig)
{
	run=0;
}
const char *name="worker";
class Test{
	public:
	Test(rtc::TaskQueue *queue):w_(queue){
	}
	void test_task(){
		w_->PostTask([this]{
			printf("what the fuck\n");
		});
	}
	private:
	rtc::TaskQueue *w_;
};
class Test2{
	public:
	Test2(uint32_t i):i_(i){}
	void Print(){
	std::cout<<"fuck"<<i_<<std::endl;
	}
	private:
	uint32_t i_{0};
};
class Test3{
	public:
	Test3(const char *classname):name_(classname){
		RTC_DCHECK(!name_.empty());
		RTC_DCHECK(name_.length() < 64);
	}
	private:
	const std::string name_;
};
void Thread_fun(void* obj){
	Test2 *pointer=static_cast<Test2*>(obj);
	pointer->Print();
};
int main(){
rtc::TaskQueue queue(name);
Test myclass(&queue);
//std::string queue_name(name);
 //RTC_DCHECK(!queue_name.empty());
myclass.test_task();
//Test2 obj(2);
//rtc::PlatformThread thread(Thread_fun,&obj,"test");
//thread.Start();
Test3 test3(name);
while(run){
}
//thread.Stop();
}
