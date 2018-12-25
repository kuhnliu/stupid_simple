#ifndef SENDINTERFACE_H_
#define SENDINTERFACE_H_
#include <stdint.h>
#include "modules/pacing/paced_sender.h"
namespace zsy{
class VideoFrameTarget{
public:
	virtual void SendVideo(uint8_t payload_type,int ftype,void *data,uint32_t len)=0;
	virtual ~VideoFrameTarget(){}
};
class SendInterface{
public:
    virtual bool TimeToSendPacket(uint32_t ssrc,
                                  uint16_t sequence_number,
                                  int64_t capture_time_ms,
                                  bool retransmission,
                                  const webrtc::PacedPacketInfo& cluster_info)=0;
    virtual size_t TimeToSendPadding(size_t bytes,
                                     const webrtc::PacedPacketInfo& cluster_info)=0;
    virtual void RTT(int64_t* rtt,
            		 int64_t* avg_rtt,
                     int64_t* min_rtt,
                     int64_t* max_rtt)=0;
    virtual ~SendInterface(){}
};
}
#endif /* SENDINTERFACE_H_ */
