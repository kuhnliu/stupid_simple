#ifndef SENDER_H_
#define SENDER_H_
#include "rtc_base/criticalsection.h"
#include "system_wrappers/include/clock.h"
#include "modules/utility/include/process_thread.h"
#include "system_wrappers/include/clock.h"
#include "modules/congestion_controller/send_side_congestion_controller.h"
#include "logging/rtc_event_log/rtc_event_log.h"
#include "modules/rtp_rtcp/source/rtcp_packet/transport_feedback.h"
#include "modules/pacing/paced_sender.h"

#include "sendinterface.h"
#include "videosource.h"
#include "sim_proto.h"

#include <memory>
#include <map>
#include <string>
namespace zsy{
class Sender:public SendInterface,
public webrtc::PacedSender::PacketSender,
public webrtc::SendSideCongestionController::Observer{
public:
	Sender();
	~Sender();
	void SetEncoder(VideoSource *encoder);
	void Bind(std::string ip,uint16_t port);
	//"ip:port"
	void SetPeer(std::string addr);
	void Start();
	void Stop();
	void Process();

	void SendVideo(uint8_t payload_type,int ftype,void *data,uint32_t len) override;
    virtual bool TimeToSendPacket(uint32_t ssrc,
                                  uint16_t sequence_number,
                                  int64_t capture_time_ms,
                                  bool retransmission,
                                  const webrtc::PacedPacketInfo& cluster_info);
    virtual size_t TimeToSendPadding(size_t bytes,
                                     const webrtc::PacedPacketInfo& cluster_info);
	void OnNetworkChanged(uint32_t bitrate_bps,
	                      uint8_t fraction_loss,  // 0 - 255.
	                      int64_t rtt_ms,
	                      int64_t probing_interval_ms) override;
private:
	sim_segment_t* get_segment_t(uint16_t sequence_number);
	void SendSegment(sim_segment_t *seg);
	int SendPadding(uint16_t payload_len,uint32_t ts);
	void SendToNetwork(uint8_t*data,uint32_t len);
	void ProcessingMsg(sim_header_t*header,bin_stream_t *stream);
	void InnerProcessFeedback(sim_feedback_t* feedback);
	void SendPing(int64_t now);
	void UpdateRtt(uint32_t time,int64_t now);
	bool running_{false};
	su_addr dst;
	su_socket fd_{0};
	rtc::CriticalSection buf_mutex_;
	std::map<uint16_t,sim_segment_t*>pending_buf_;
	uint32_t fid_{1};
	uint32_t frame_seed_{1};
	uint32_t packet_seed_{1};
	uint16_t trans_seq_{1};
	VideoSource *encoder_{NULL};
	bin_stream_t	stream_;
	std::unique_ptr<webrtc::ProcessThread> pm_;
	std::unique_ptr<SendSideCongestionControllerInterface> cc_;
	webrtc::PacedSender *pacer_{NULL};
	webrtc::RtcEventLogNullImpl m_nullLog;
	webrtc::Clock *clock_{NULL};
	int64_t first_ts_{-1};
	uint32_t uid_{1234};
	uint32_t rtt_{0};
	int64_t sum_rtt_{0};
	int64_t rtt_num_{0};
	uint32_t max_rtt_{0};
	int64_t update_ping_ts_{0};
};
}



#endif /* SENDER_H_ */
