#include "test_sender.h"
#include "rtc_base/timeutils.h"
#include <iostream>
#include <string>
namespace zsy{
TestSender::TestSender(VideoSource *vs){
	s_=vs;
	s_->RegisterSender(this);
	running_=true;
	Start();
}
void TestSender::Start(){
	if(s_){
		s_->Start();
	}
}
void TestSender::Stop(){
	if(s_){
		s_->Stop();
	}
}
void TestSender::SendVideo(uint8_t payload_type,int ftype,
		void *data,uint32_t len){
	int64_t now=rtc::TimeMillis();
	if(last_==0){
		last_=now;
	}
	uint32_t delta=now-last_;
	std::cout<<std::to_string(delta)<<" "
			<<std::to_string(len)<<std::endl;
	last_=now;
}
}




