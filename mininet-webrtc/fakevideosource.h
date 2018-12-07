#ifndef FAKEVIDEOSOURCE_H_
#define FAKEVIDEOSOURCE_H_
#include "rtc_base/thread.h"
#include "videosource.h"
namespace zsy{
class VideoGenerator:public VideoSource{
public:
	VideoGenerator(rtc::Thread *w,uint32_t fs,uint32_t minR);
	~VideoGenerator() override{};
	void Start() override;
	void Stop() override;
	void RegisterSender(SendInterface *sender) override;
	void SendFrame();
	void Generate();
private:
	rtc::Thread *w_;
	uint32_t fs_;
	uint32_t rate_;
	uint32_t minR_;
	bool running_{false};
	SendInterface* sender_{NULL};
};
}



#endif /* FAKEVIDEOSOURCE_H_ */
