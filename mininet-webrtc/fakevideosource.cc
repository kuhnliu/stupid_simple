#include "fakevideosource.h"
VideoGenerator::VideoGenerator(rtc::Thread *w,uint32_t fs,uint32_t minR){
	w_=w;
	fs_=fs;
	rate_=minR;
	minR_=minR;
}
void VideoGenerator::Start(){
	running_=true;
	Generate();
}
void VideoGenerator::Stop(){
	running_=false;
}
void VideoGenerator::RegisterSender(SendInterface *sender){
	sender_=sender;
}
void VideoGenerator::SendFrame(){
	float len=(float)rate_/(fs_ * 8.f);
	uint32_t bytesToSend=len;
	if(sender_){
		uint8_t *buf=new uint8_t[bytesToSend];
		sender_->SendVideo(0,0,buf,bytesToSend);
		delete buf;
	}
}
void VideoGenerator::Generate(){
	if(running_){
		SendFrame();
		uint32_t duration=1000/fs_;
		w_->PostDelayedTask([this]{
			this->Generate();
		},duration);
	}
}





