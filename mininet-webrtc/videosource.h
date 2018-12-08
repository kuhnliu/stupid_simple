#ifndef VIDEOSOURCE_H_
#define VIDEOSOURCE_H_
#include "sendinterface.h"
namespace zsy{
class VideoSource{
public:
	virtual ~VideoSource(){}
	virtual void Start()=0;
	virtual void Stop()=0;
	virtual void ChangeRate(uint32_t bitrate)=0;
	virtual void RegisterSender(SendInterface *s)=0;
	virtual void SetMinRate(uint32_t min)=0;
};
}




#endif /* VIDEOSOURCE_H_ */
