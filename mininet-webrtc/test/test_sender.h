#ifndef TEST_TEST_SENDER_H_
#define TEST_TEST_SENDER_H_
#include "sendinterface.h"
#include "videosource.h"
namespace zsy{
class TestSender:public SendInterface{
public:
	TestSender(VideoSource *vs);
	~TestSender();
	void Start();
	void Stop();
	void SendVideo(uint8_t payload_type,int ftype,
			void *data,uint32_t len) override;
private:
	VideoSource *s_{NULL};
	int64_t last_{0};
	bool running_{false};
};
}




#endif /* TEST_TEST_SENDER_H_ */
