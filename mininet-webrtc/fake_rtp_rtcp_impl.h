#ifndef FAKE_RTP_RTCP_IMPL_H_
#define FAKE_RTP_RTCP_IMPL_H_
#include "modules/rtp_rtcp/include/rtp_rtcp.h"
#include "modules/rtp_rtcp/include/rtp_rtcp_defines.h"
#include "modules/rtp_rtcp/source/rtcp_sender.h"
#include<iostream>
namespace zsy{
class SendInterface;
class FakeRtpRtcpImpl:public webrtc::RtpRtcp{
public:
	FakeRtpRtcpImpl(SendInterface *sender);
	~FakeRtpRtcpImpl() override;
	  // Returns the number of milliseconds until the module want a worker thread to
	  // call Process.
	  int64_t TimeUntilNextProcess() override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  // Process any pending tasks such as timeouts.
	  void Process() override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  // Receiver part.

	  // Called when we receive an RTCP packet.
	  void IncomingRtcpPacket(const uint8_t* incoming_packet,
	                          size_t incoming_packet_length) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  void SetRemoteSSRC(uint32_t ssrc) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  remote_ssrc_=ssrc;
	  }

	  // Sender part.

	  void RegisterAudioSendPayload(int payload_type,
	                                absl::string_view payload_name,
	                                int frequency,
	                                int channels,
	                                int rate) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }
	  void RegisterVideoSendPayload(int payload_type,
	                                const char* payload_name) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  int32_t DeRegisterSendPayload(int8_t payload_type) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  void SetExtmapAllowMixed(bool extmap_allow_mixed) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  // Register RTP header extension.
	  int32_t RegisterSendRtpHeaderExtension(RTPExtensionType type,
	                                         uint8_t id) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }
	  bool RegisterRtpHeaderExtension(const std::string& uri, int id) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return true;
	  }

	  int32_t DeregisterSendRtpHeaderExtension(webrtc::RTPExtensionType type) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  bool HasBweExtensions() const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return true;
	  }

	  // Get start timestamp.
	  uint32_t StartTimestamp() const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  // Configure start timestamp, default is a random number.
	  void SetStartTimestamp(uint32_t timestamp) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  uint16_t SequenceNumber() const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  // Set SequenceNumber, default is a random number.
	  void SetSequenceNumber(uint16_t seq) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  void SetRtpState(const webrtc::RtpState& rtp_state) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }
	  void SetRtxState(const webrtc::RtpState& rtp_state) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }
	  webrtc::RtpState GetRtpState() const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return webrtc::RtpState();
	  }
	  webrtc::RtpState GetRtxState() const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return webrtc::RtpState();
	  }

	  uint32_t SSRC() const override{
		  return ssrc_;
	  }

	  // Configure SSRC, default is a random number.
	  void SetSSRC(uint32_t ssrc) override{
		  ssrc_=ssrc;
	  }

	  void SetRid(const std::string& rid) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  void SetMid(const std::string& mid) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  void SetCsrcs(const std::vector<uint32_t>& csrcs) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  webrtc::RTCPSender::FeedbackState GetFeedbackState(){
		  std::cout<<__FUNCTION__<<std::endl;
		  return  webrtc::RTCPSender::FeedbackState();
	  }

	  void SetRtxSendStatus(int mode) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }
	  int RtxSendStatus() const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  void SetRtxSsrc(uint32_t ssrc) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  void SetRtxSendPayloadType(int payload_type,
	                             int associated_payload_type) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  absl::optional<uint32_t> FlexfecSsrc() const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return absl::nullopt;
	  }

	  // Sends kRtcpByeCode when going from true to false.
	  int32_t SetSendingStatus(bool sending) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  bool Sending() const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return true;
	  }

	  // Drops or relays media packets.
	  void SetSendingMediaStatus(bool sending) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  bool SendingMedia() const override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  void SetAsPartOfAllocation(bool part_of_allocation) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  // Used by the codec module to deliver a video or audio frame for
	  // packetization.
	  bool SendOutgoingData(FrameType frame_type,
	                        int8_t payload_type,
	                        uint32_t time_stamp,
	                        int64_t capture_time_ms,
	                        const uint8_t* payload_data,
	                        size_t payload_size,
	                        const webrtc::RTPFragmentationHeader* fragmentation,
	                        const webrtc::RTPVideoHeader* rtp_video_header,
	                        uint32_t* transport_frame_id_out) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return true;
	  }

	  bool TimeToSendPacket(uint32_t ssrc,
	                        uint16_t sequence_number,
	                        int64_t capture_time_ms,
	                        bool retransmission,
	                        const webrtc::PacedPacketInfo& pacing_info) override{
		  if(sender_){
			  return sender_->TimeToSendPacket(ssrc,sequence_number,capture_time_ms,
					  retransmission,pacing_info);
		  }else{
			  return true;
		  }
	  }

	  // Returns the number of padding bytes actually sent, which can be more or
	  // less than |bytes|.
	  size_t TimeToSendPadding(size_t bytes,
	                           const webrtc::PacedPacketInfo& pacing_info) override{
		  if(sender_){
			  return sender_->TimeToSendPadding(bytes,pacing_info);
		  }else{
			  return bytes;
		  }
	  }

	  // RTCP part.

	  // Get RTCP status.
	  webrtc::RtcpMode RTCP() const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return webrtc::RtcpMode::kCompound;
	  }

	  // Configure RTCP status i.e on/off.
	  void SetRTCPStatus(webrtc::RtcpMode method) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  // Set RTCP CName.
	  int32_t SetCNAME(const char* c_name) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  // Get remote CName.
	  int32_t RemoteCNAME(uint32_t remote_ssrc,
	                      char c_name[RTCP_CNAME_SIZE]) const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  // Get remote NTP.
	  int32_t RemoteNTP(uint32_t* received_ntp_secs,
	                    uint32_t* received_ntp_frac,
	                    uint32_t* rtcp_arrival_time_secs,
	                    uint32_t* rtcp_arrival_time_frac,
	                    uint32_t* rtcp_timestamp) const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  int32_t AddMixedCNAME(uint32_t ssrc, const char* c_name) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  int32_t RemoveMixedCNAME(uint32_t ssrc) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  // Get RoundTripTime.
	  int32_t RTT(uint32_t remote_ssrc,
	              int64_t* rtt,
	              int64_t* avg_rtt,
	              int64_t* min_rtt,
	              int64_t* max_rtt) const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  if(sender_){
			  sender_->RTT(rtt,avg_rtt,min_rtt,max_rtt);
		  }
	  }

	  // Force a send of an RTCP packet.
	  // Normal SR and RR are triggered via the process function.
	  int32_t SendRTCP(webrtc::RTCPPacketType rtcpPacketType) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }
	  int32_t SendCompoundRTCP(
	      const std::set<webrtc::RTCPPacketType>& rtcpPacketTypes) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  // Statistics of the amount of data sent and received.
	  int32_t DataCountersRTP(size_t* bytes_sent,
	                          uint32_t* packets_sent) const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  void GetSendStreamDataCounters(
	      webrtc::StreamDataCounters* rtp_counters,
	      webrtc::StreamDataCounters* rtx_counters) const override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  void GetRtpPacketLossStats(
	      bool outgoing,
	      uint32_t ssrc,
	      struct webrtc::RtpPacketLossStats* loss_stats) const override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  // Get received RTCP report, report block.
	  int32_t RemoteRTCPStat(
	      std::vector<webrtc::RTCPReportBlock>* receive_blocks) const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  // (REMB) Receiver Estimated Max Bitrate.
	  void SetRemb(int64_t bitrate_bps, std::vector<uint32_t> ssrcs) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }
	  void UnsetRemb() override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  // (TMMBR) Temporary Max Media Bit Rate.
	  bool TMMBR() const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return true;
	  }

	  void SetTMMBRStatus(bool enable) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  void SetTmmbn(std::vector<webrtc::rtcp::TmmbItem> bounding_set) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  size_t MaxRtpPacketSize() const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  void SetMaxRtpPacketSize(size_t max_packet_size) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  // (NACK) Negative acknowledgment part.

	  int SelectiveRetransmissions() const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  int SetSelectiveRetransmissions(uint8_t settings) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  // Send a Negative acknowledgment packet.
	  // TODO(philipel): Deprecate SendNACK and use SendNack instead.
	  int32_t SendNACK(const uint16_t* nack_list, uint16_t size) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  void SendNack(const std::vector<uint16_t>& sequence_numbers) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  // Store the sent packets, needed to answer to a negative acknowledgment
	  // requests.
	  void SetStorePacketsStatus(bool enable, uint16_t number_to_store) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  bool StorePackets() const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return true;
	  }

	  // Called on receipt of RTCP report block from remote side.
	  void RegisterRtcpStatisticsCallback(
	      webrtc::RtcpStatisticsCallback* callback) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }
	  RtcpStatisticsCallback* GetRtcpStatisticsCallback() override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return NULL;
	  }

	  bool SendFeedbackPacket(const webrtc::rtcp::TransportFeedback& packet) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return false;
	  }
	  // (APP) Application specific data.
	  int32_t SetRTCPApplicationSpecificData(uint8_t sub_type,
	                                         uint32_t name,
	                                         const uint8_t* data,
	                                         uint16_t length) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  // (XR) Receiver reference time report.
	  void SetRtcpXrRrtrStatus(bool enable) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  bool RtcpXrRrtrStatus() const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return true;
	  }

	  // Audio part.

	  // Send a TelephoneEvent tone using RFC 2833 (4733).
	  int32_t SendTelephoneEventOutband(uint8_t key,
	                                    uint16_t time_ms,
	                                    uint8_t level) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  // Store the audio level in d_bov for header-extension-for-audio-level-
	  // indication.
	  int32_t SetAudioLevel(uint8_t level_d_bov) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  // Video part.

	  // Set method for requesting a new key frame.
	  int32_t SetKeyFrameRequestMethod(webrtc::KeyFrameRequestMethod method) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  // Send a request for a keyframe.
	  int32_t RequestKeyFrame() override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  void SetUlpfecConfig(int red_payload_type, int ulpfec_payload_type) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  bool SetFecParameters(const webrtc::FecProtectionParams& delta_params,
	                        const webrtc::FecProtectionParams& key_params) override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return true;
	  }
	  void BitrateSent(uint32_t* total_rate,
	                   uint32_t* video_rate,
	                   uint32_t* fec_rate,
	                   uint32_t* nackRate) const override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

	  void RegisterSendChannelRtpStatisticsCallback(
	      webrtc::StreamDataCountersCallback* callback) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }
	  StreamDataCountersCallback* GetSendChannelRtpStatisticsCallback()
	      const override{
		  std::cout<<__FUNCTION__<<std::endl;
		  return 0;
	  }

	  void SetVideoBitrateAllocation(
	      const webrtc::VideoBitrateAllocation& bitrate) override{
		  std::cout<<__FUNCTION__<<std::endl;
	  }

private:
	SendInterface *sender_{NULL};
	uint32_t remote_ssrc_{0};
	uint32_t ssrc_{0};
};
}




#endif /* FAKE_RTP_RTCP_IMPL_H_ */
