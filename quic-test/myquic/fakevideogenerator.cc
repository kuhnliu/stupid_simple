#include "fakevideogenerator.h"
#include <iostream>
namespace net{
FakeVideoGenerator::FakeVideoGenerator(int fs,int start_rate)
:next_(QuicTime::Zero()){
	rate_=start_rate;
	min_rate_=start_rate;
	duration_=1000/fs;
}
void FakeVideoGenerator::UpdataRate(int64_t bps){
	int64_t bw=bps;
    std::cout<<"bw "<<bps<<std::endl;
	if(bw>max_rate_){
		bw=max_rate_;
	}
	if(bw<min_rate_){
		bw=min_rate_;
	}
	rate_=bw;
}
//in byte
int64_t FakeVideoGenerator::GetFrame(QuicTime now){
	int64_t len=0;
	int64_t expect=(rate_*duration_)/(1000*8);
	if(now>next_){
		len=expect;
		next_=now+QuicTime::Delta::FromMilliseconds(duration_);
	}
	return len;
}
}




