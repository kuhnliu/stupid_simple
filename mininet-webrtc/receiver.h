#ifndef RECEIVER_H_
#define RECEIVER_H_
#include "rtc_base/timeutils.h"
#include "modules/congestion_controller/include/receive_side_congestion_controller.h"
#include "modules/rtp_rtcp/source/rtcp_packet/transport_feedback.h"
#include "modules/pacing/packet_router.h"
#include "system_wrappers/include/clock.h"
#include "modules/utility/include/process_thread.h"
#include "sim_proto.h"
#include "cf_stream.h"
#include "cf_platform.h"

#include <stdlib.h>
#include <string>
namespace zsy{
class Receiver :public webrtc::PacketRouter{
public:
	Receiver();
	~Receiver();
	void Bind(std::string ip,uint16_t port);
	void SetRunDuration(uint32_t delta);
	bool IsStop();
	void Stop();
	void Process();
	uint16_t AllocateSequenceNumber() override{
		return 0;
	}
	void OnReceiveBitrateChanged(const std::vector<uint32_t>& ssrcs,
	                                       uint32_t bitrate) override{

	}
	bool SendTransportFeedback(webrtc::rtcp::TransportFeedback* packet) override;
private:
	void SendToNetwork(uint8_t*data,uint32_t len);
	void ProcessingMsg(bin_stream_t *stream);
	void SendPing(int64_t now);
	void OnRecvSegment(sim_header_t *header,sim_segment_t *seg);
	void OnRecvPad(sim_header_t *header,sim_pad_t *body);
	void UpdateRtt(uint32_t time,int64_t now);
	void ConfigureCongestion();
	bool running_{true};
	bool session_connected_{false};
	bool first_packet_{true};
	su_addr dst_;
	su_socket fd_{0};
	bin_stream_t	stream_;
	std::unique_ptr<webrtc::ProcessThread> pm_;
	webrtc::ReceiveSideCongestionController *receive_side_cc_{NULL};
	webrtc::Clock *clock_{NULL};
	uint32_t uid_{4321};
	uint32_t rtt_{0};
	int64_t sum_rtt_{0};
	int64_t rtt_num_{0};
	uint32_t max_rtt_{0};
	int64_t update_ping_ts_{0};
	uint32_t duration_{0};
	uint32_t stop_ts_{0};
	uint32_t base_seq_{0};
};
}
#endif /* RECEIVER_H_ */
