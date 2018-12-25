#ifndef FAKEVIDEOSOURCE_H_
#define FAKEVIDEOSOURCE_H_
#include <stdlib.h>
//#include "rtc_base/thread.h"
#include "rtc_base/task_queue.h"
#include "videosource.h"
#include <string>
#include <iostream>
#include <fstream>
#include <string>
namespace zsy{
typedef struct{
uint32_t now;
uint32_t bps;
}time_rate_t;
class VideoGenerator:public VideoSource{
public:
	VideoGenerator(rtc::TaskQueue *w,uint32_t fs);
	~VideoGenerator() override{
	Close();
	}
	void Start() override;
	void Stop() override;
	void RegisterVideoTaget(VideoFrameTarget *sender) override;
	void SetMinRate(uint32_t minR) override;
	void ChangeRate(uint32_t bitrate) override;
	void SendFrame();
	void Generate();
	void SetLogFile(std::string name);
private:
	void LogRate(uint32_t bitrate);
	void WriteToFile(time_rate_t record);
	void Close();
	rtc::TaskQueue *w_;
	uint32_t fs_;
	uint32_t rate_{0};
	uint32_t minR_;
	bool running_{false};
	VideoFrameTarget* sender_{NULL};
	bool log_enable_{false};
	uint32_t first_{0};
	std::fstream f_rate_;
};
}



#endif /* FAKEVIDEOSOURCE_H_ */
