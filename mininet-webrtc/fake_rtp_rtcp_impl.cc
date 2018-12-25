#include "fake_rtp_rtcp_impl.h"
#include "sendinterface.h"
namespace zsy{
FakeRtpRtcpImpl::FakeRtpRtcpImpl(SendInterface *sender){
	sender_=sender;
}
	  bool FakeRtpRtcpImpl::TimeToSendPacket(uint32_t ssrc,
	                        uint16_t sequence_number,
	                        int64_t capture_time_ms,
	                        bool retransmission,
	                        const webrtc::PacedPacketInfo& pacing_info){
		  if(sender_){
			  //std::cout<<"send "<<sequence_number<<std::endl;
			  return sender_->TimeToSendPacket(ssrc,sequence_number,capture_time_ms,
					  retransmission,pacing_info);
		  }else{
			  return true;
		  }
	  }

	  // Returns the number of padding bytes actually sent, which can be more or
	  // less than |bytes|.
	  size_t FakeRtpRtcpImpl::TimeToSendPadding(size_t bytes,
	                           const webrtc::PacedPacketInfo& pacing_info){
		  if(sender_){
			 std::cout<<"send padding"<<bytes<<std::endl;
			  return sender_->TimeToSendPadding(bytes,pacing_info);
		  }else{
			  return bytes;
		  }
	  }

	  int32_t FakeRtpRtcpImpl::RTT(uint32_t remote_ssrc,
	              int64_t* rtt,
	              int64_t* avg_rtt,
	              int64_t* min_rtt,
	              int64_t* max_rtt) const{
		  std::cout<<__FUNCTION__<<std::endl;
		  if(sender_){
			  sender_->RTT(rtt,avg_rtt,min_rtt,max_rtt);
		  }
	  return 0;
	  }	
}
