#include "receiver.h"
#include "rtc_base/timeutils.h"
#include "rtc_base/location.h"
#include <stdio.h>
#include <memory.h>
namespace zsy{
Receiver::Receiver(){
	bin_stream_init(&stream_);
	clock_=webrtc::Clock::GetRealTimeClock();
	pm_=webrtc::ProcessThread::Create("ReceiverModuleThread");
}
Receiver::~Receiver(){
	bin_stream_destroy(&stream_);
}
void Receiver::Bind(std::string ip,uint16_t port){
	su_udp_create(ip.c_str(),port,&fd_);
}
void Receiver::SetRunDuration(uint32_t delta){
	duration_=delta;
}
bool Receiver::IsStop(){
	bool ret=false;
	if(session_connected_){
		uint32_t now=rtc::TimeMillis();
		if(now>stop_ts_){
			ret=true;
			Stop();
		}
	}
	return ret;
}
void Receiver::Stop(){
	if(receive_side_cc_){
		pm_->DeRegisterModule(receive_side_cc_->GetRemoteBitrateEstimator(true));
		pm_->DeRegisterModule(receive_side_cc_);
		pm_->Stop();
	}
}
void Receiver::Process(){
	if(!running_){
		return ;
	}
	if(session_connected_){
		uint32_t delta=100;
		if(rtt_!=0){
			delta=rtt_;
		}
		if(now-update_ping_ts_>delta){
			SendPing(now);
		}
	}
	su_addr remote;
	bin_stream_t rstream;
	bin_stream_init(&rstream);
	int32_t ret=0;
	ret=su_udp_recv(fd_,&remote,rstream.data,rstream.size,0);
	if(ret>0){
		if(!session_connected_){
			memcpy(&dst_,&remote,sizeof(su_addr));
		}
		memcpy(&dst,&remote,sizeof(su_addr));
		rstream.used=ret;
		ProcessingMsg(&rstream);
	}
	bin_stream_destroy(&rstream);
}
bool Receiver::SendTransportFeedback(rtcp::TransportFeedback* packet){
	packet->SetSenderSsrc(uid);
	rtc::Buffer serialized= packet->Build();
	sim_header_t header;
	sim_feedback_t feedback;
    uint32_t payload_size=serialized.size();
	if (payload_size > SIM_FEEDBACK_SIZE){
		printf("feedback size > SIM_FEEDBACK_SIZE\n");
		return false;
	}
	INIT_SIM_HEADER(header, SIM_FEEDBACK, uid);
	feedback.base_packet_id =base_seq_;
	feedback.feedback_size = payload_size;
	memcpy(feedback.feedback, (uint8_t*)serialized.data(), payload_size);
	sim_encode_msg(&stream_, &header, &feedback);
	SendToNetwork(stream_.data,stream_.used);
    return true;
}
void Receiver::SendToNetwork(uint8_t*data,uint32_t len){
	su_udp_send(fd_,&dst_,data, len);
}
void Receiver::ProcessingMsg(bin_stream_t *stream){
	sim_header_t header;
	if (sim_decode_header(stream, &header) != 0)
		return;
	if (header.mid < MIN_MSG_ID || header.mid > MAX_MSG_ID)
		return;
	if(!session_connected_){
		session_connected_=true;
	}
	switch(header.mid){
	case SIM_PING:{
		sim_header_t h;
		sim_ping_t ping;
		sim_decode_msg(stream, &header, &ping);
		INIT_SIM_HEADER(h, SIM_PONG, uid_);
		sim_encode_msg(stream, &h, &ping);
		SendToNetwork(stream->data,stream->used);
        break;
	}
	case SIM_PONG:{
		sim_pong_t pong;
		sim_decode_msg(stream, &header, &pong);
		int64_t now=rtc::TimeMillis();
		int64_t rtt=now-pong.ts;
		if(rtt>0){
			UpdateRtt(rtt,now);
		}
        break;
	}
	case SIM_SEG:{
		sim_segment_t seg;
		if (sim_decode_msg(stream, &header, &seg) != 0)
			return;
		OnRecvSegment(&header,&seg);
			break;
	}
	case SIM_PAD:{
		sim_pad_t pad;
		if (sim_decode_msg(stream, &header, &pad) != 0)
			return;
		OnRecvPad(&header,&pad);
			break;
	}
	default:{
		break;
	}
	}
}
void Receiver::SendPing(int64_t now){
	sim_header_t header;
	sim_ping_t body;
	INIT_SIM_HEADER(header, SIM_PING, uid_);
	body.ts = now;
	sim_encode_msg(&stream_, &header, &body);
	SendToNetwork(stream_.data,stream_.used);
	update_ping_ts_=now;
}
void Receiver::OnRecvSegment(sim_header_t *header,sim_segment_t *seg){
	ConfigureCongestion();
	uint32_t now=Simulator::Now().GetMilliSeconds();
	uint32_t overhead=seg->data_size + SIM_SEGMENT_HEADER_SIZE;
	webrtc::RTPHeader fakeHeader;
	fakeHeader.ssrc=header->uid;
	fakeHeader.extension.hasTransportSequenceNumber=true;
	fakeHeader.extension.transportSequenceNumber=seg->transport_seq;
	receive_side_cc_->OnReceivedPacket(now,overhead,fakeHeader);
	if(first_packet_){
		first_packet_=false;
		stop_ts_=now+duration_;
	}
}
void Receiver::OnRecvPad(sim_header_t *header,sim_pad_t *body){
	uint32_t now=rtc::TimeMillis();
	uint8_t header_len=sizeof(sim_header_t)+sizeof(sim_pad_t);
	uint32_t overhead=body->data_size + header_len;
	ConfigureCongestion();
	webrtc::RTPHeader fakeHeader;
	fakeHeader.ssrc=header.uid;
	fakeHeader.extension.hasTransportSequenceNumber=true;
	fakeHeader.extension.transportSequenceNumber=body.transport_seq;
	receive_side_cc_->OnReceivedPacket(now,overhead,fakeHeader);

}
void Receiver::ConfigureCongestion(){
	if(receive_side_cc_){
		return ;
	}
	receive_side_cc_=new webrtc::ReceiveSideCongestionController(clock_,this);
	pm_->RegisterModule(receive_side_cc_->GetRemoteBitrateEstimator(true),RTC_FROM_HERE);
	pm_->RegisterModule(receive_side_cc_, RTC_FROM_HERE);
	pm_->Start();
}
void Receiver::UpdateRtt(uint32_t time,int64_t now){
	uint32_t keep_rtt=5;
	uint32_t averageRtt=0;
	if(time>keep_rtt){
		keep_rtt=time;
	}
	if(rtt_==0){
		rtt_=keep_rtt;
	}

	rtt_ = (7 * rtt_ + keep_rtt) / 8;
	if (rtt_ < 10)
		rtt_ = 10;
	if(rtt_num_==0)
	{
		if(receive_side_cc_){
			receive_side_cc_->OnRttUpdate(keep_rtt,keep_rtt);
		}
		sum_rtt_=keep_rtt;
		max_rtt_=keep_rtt;
		rtt_num_++;
		return;
	}
	rtt_num_+=1;
	sum_rtt_+=keep_rtt;
	averageRtt=sum_rtt_/rtt_num_;
	if(keep_rtt>max_rtt_)
	{
		max_rtt_=keep_rtt;
	}
	if(receive_side_cc_){
		receive_side_cc_->OnRttUpdate(averageRtt,max_rtt_);
	}
}
}




