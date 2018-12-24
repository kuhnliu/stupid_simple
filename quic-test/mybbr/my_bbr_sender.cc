#include "my_bbr_sender.h"
#include <iostream>
namespace net{
namespace{
const float kPacingGain[] = {1.25, 0.75, 1, 1, 1, 1, 1, 1};

// The length of the gain cycle.
const size_t kGainCycleLength = sizeof(kPacingGain) / sizeof(kPacingGain[0]);
// The size of the bandwidth filter window, in round-trips.
const uint64_t kBandwidthWindowSize = kGainCycleLength + 2;

// The time after which the current min_rtt value expires.
const QuicTime::Delta kMinRttExpiry = QuicTime::Delta::FromSeconds(10);
// The minimum time the connection can spend in PROBE_RTT mode.
const QuicTime::Delta kProbeRttTime = QuicTime::Delta::FromMilliseconds(200);
const float kSimilarMinRttThreshold = 1.125;
//10 cluster;
const uint64_t kAverageAckRateWindowSize=10;
}
SentClusterInfo::SentClusterInfo(uint64_t cluster_id,BandwidthEstimateInteface *target)
:cluster_id_(cluster_id)
,first_send_ts_(QuicTime::Zero())
,last_send_ts_(QuicTime::Zero())
,first_ack_ts_(QuicTime::Zero())
,last_ack_ts_(QuicTime::Zero())
,bw_target_(target)
,bw_(QuicBandwidth::Zero()){}
SentClusterInfo::~SentClusterInfo(){
	while(!packet_infos_.empty()){
		auto it=packet_infos_.begin();
		SentPacketInfo *packet_info=(*it);
		delete packet_info;
        packet_infos_.erase(it);
	}
	std::vector<SentPacketInfo*> null_vec;
	null_vec.swap(packet_infos_);
}
void SentClusterInfo::OnPacketSent(QuicTime now,
		QuicPacketNumber seq,
		uint64_t len){
	if(first_send_ts_==QuicTime::Zero()){
		first_send_ts_=now;
	}
	last_send_ts_=now;
	SentPacketInfo *packet_info=new SentPacketInfo(seq,now,QuicTime::Zero(),len);
	packet_infos_.push_back(packet_info);
	sent_packets_+=len;
}
void SentClusterInfo::OnAck(QuicPacketNumber seq,
		QuicTime now){
	if(first_ack_ts_==QuicTime::Zero()){
		first_ack_ts_=now;
		if(prev_){
			prev_->TriggerBandwidthEstimate();
		}
	}
	last_ack_ts_=now;
	SentPacketInfo *head=packet_infos_[0];
	SentPacketInfo *target=NULL;
	QuicPacketNumber head_seq=head->seq;
    if(seq>=head->seq){
	uint32_t offset=seq-head_seq;
	    if(offset<packet_infos_.size()){
	    	target=packet_infos_[offset];
	    }
	    if(target){
		    target->ack_ts=now;
		    acked_packets_+=target->len;
	    }    
    }
}
void SentClusterInfo::GetSentInfo(uint64_t *ms,QuicByteCount *sent){
	QuicTime::Delta delta=last_send_ts_-first_send_ts_;
	*ms=delta.ToMilliseconds();
	*sent=sent_packets_;
}
void SentClusterInfo::GetAckedInfo(uint64_t *ms,QuicByteCount *acked){
	QuicTime::Delta delta=last_ack_ts_-first_ack_ts_;
	*ms=delta.ToMilliseconds();
	*acked=acked_packets_;
}
void SentClusterInfo::TriggerBandwidthEstimate(){
	if(acked_packets_==0||last_ack_ts_==first_ack_ts_){
		return ;
	}
	QuicBandwidth send_rate = QuicBandwidth::Infinite();
	if(last_send_ts_>first_send_ts_){
		send_rate=QuicBandwidth::FromBytesAndTimeDelta(sent_packets_,
				last_send_ts_-first_send_ts_);
	}
	 QuicBandwidth ack_rate=QuicBandwidth::FromBytesAndTimeDelta(acked_packets_,
			 last_ack_ts_-first_ack_ts_);
	bw_=std::min(send_rate,ack_rate);
	if(bw_>QuicBandwidth::Zero()){
		bw_target_->OnEstimateBandwidth(this,cluster_id_,bw_);
	}
}
MyBbrSender::MyBbrSender(uint64_t min_bps,
		uint64_t start_bps,
		uint64_t max_bps)
:min_bps_(min_bps)
,start_bps_(start_bps)
,max_bps_(max_bps)
,min_rtt_(QuicTime::Delta::Zero())
,recored_min_rtt_(QuicTime::Delta::Zero())
,min_rtt_timestamp_(QuicTime::Zero())
,exit_probe_rtt_at_(QuicTime::Zero())
,pacing_rate_(QuicBandwidth::FromBitsPerSecond(start_bps))
,last_cycle_start_(QuicTime::Zero())
,max_bandwidth_(kBandwidthWindowSize,QuicBandwidth::Zero(),0)
,avergae_bandwidth_(kAverageAckRateWindowSize,QuicBandwidth::Zero(),0)
,random_(QuicRandom::GetInstance()){

}
MyBbrSender::~MyBbrSender(){
	seq_round_map_.clear();
	acked_clusters_.clear();
	while(!connection_info_.empty()){
		auto it=connection_info_.begin();
		SentClusterInfo* cluster=it->second;
		delete cluster;
		connection_info_.erase(it);
	}
}
QuicBandwidth MyBbrSender::PaceRate(){
	return pacing_rate_;
}
QuicBandwidth MyBbrSender::BandwidthEstimate(){
	return max_bandwidth_.GetBest();
}
QuicBandwidth MyBbrSender::AverageBandwidthEstimate(){
	return avergae_bandwidth_.GetBest();
}
bool MyBbrSender::ShouldSendProbePacket() const{
	if(pacing_gain_<=1){
		return false;
	}
	return true;
}
void MyBbrSender::OnAck(QuicTime event_time,
		QuicPacketNumber packet_number){
	if(packet_number<=last_ack_seq_){
		return;
	}
	last_ack_seq_=packet_number;
	UpdateRtt(event_time,packet_number);
	if(mode_==START_UP){
		EnterProbeBandwidthMode(event_time);
	}
	/* to avoid void cluster, without packets sent
	if(mode_==PROBE_BW){
		UpdateGainCyclePhase(event_time);
	}*/
	bool min_rtt_expired=false;
	min_rtt_expired=event_time>(min_rtt_timestamp_ + kMinRttExpiry);
	MaybeEnterOrExitProbeRtt(event_time,min_rtt_expired);
    CalculatePacingRate();
	auto seq_round_it=seq_round_map_.find(packet_number);
	if(seq_round_it!=seq_round_map_.end()){
		uint64_t round=seq_round_it->second;
		seq_round_map_.erase(seq_round_it);
		auto connection_info_it=connection_info_.find(round);
		if(connection_info_it!=connection_info_.end()){
			SentClusterInfo *cluster=connection_info_it->second;
			cluster->OnAck(packet_number,event_time);
		}
	}
	uint64_t total=acked_clusters_.size();
	if(total>=kAverageAckRateWindowSize){
		uint64_t total_sent_bytes=0;
		uint64_t total_acked_bytes=0;
		uint64_t acc_sent_delta=0;
		uint64_t acc_ack_delta=0;
		uint64_t cluster_id=0;
		for(auto acked_clusters_it=acked_clusters_.begin();
				acked_clusters_it!=acked_clusters_.end();
				acked_clusters_it++){
			uint64_t sent_bytes=0;
			uint64_t acked_bytes=0;
			uint64_t sent_delta=0;
			uint64_t ack_delta=0;
			SentClusterInfo *cluster=(*acked_clusters_it);
			cluster->GetSentInfo(&sent_delta,&sent_bytes);
			cluster->GetAckedInfo(&ack_delta,&acked_bytes);
			if(sent_delta!=0){
				acc_sent_delta+=sent_delta;
				total_sent_bytes+=sent_bytes;
			}
			if(ack_delta!=0){
				acc_ack_delta+=ack_delta;
				total_acked_bytes+=acked_bytes;
				cluster_id=cluster->GetClusterId();
			}
		}
		QuicBandwidth sent_rate=QuicBandwidth::Infinite();
		QuicBandwidth acked_rate=QuicBandwidth::Infinite();
		if(acc_sent_delta>0){
			sent_rate=QuicBandwidth::FromBytesAndTimeDelta(total_sent_bytes,
					QuicTime::Delta::FromMilliseconds(acc_sent_delta));
		}
		if(acc_ack_delta>0){
			acked_rate=QuicBandwidth::FromBytesAndTimeDelta(total_acked_bytes,
					QuicTime::Delta::FromMilliseconds(acc_ack_delta));
		}
		QuicBandwidth average_rate=std::min(sent_rate,acked_rate);
		average_round_=average_round_+cluster_id%kAverageAckRateWindowSize;
		if(average_rate==QuicBandwidth::Infinite()){
			average_rate=QuicBandwidth::FromBitsPerSecond(min_bps_);
		}
		avergae_bandwidth_.Update(average_rate,average_round_);
		uint64_t stale=total-(kAverageAckRateWindowSize-1);
		uint64_t i=0;
		for(i=0;i<stale;i++){
			auto acked_clusters_it=acked_clusters_.begin();
			SentClusterInfo *cluster=(*acked_clusters_it);
			cluster_id=cluster->GetClusterId();
			acked_clusters_.erase(acked_clusters_it);
            RemoveClusterLE(cluster_id);
		}
	}
}
void MyBbrSender::OnPacketSent(QuicTime event_time,
		QuicPacketNumber packet_number,
		QuicByteCount bytes){
	AddSeqAndTimestamp(event_time,packet_number);
	if(mode_==PROBE_BW){
		UpdateGainCyclePhase(event_time);
		AddPacketInfoSampler(event_time,packet_number,bytes);
	}
}
void MyBbrSender::OnEstimateBandwidth(SentClusterInfo *cluster,uint64_t cluter_id,QuicBandwidth bw){
	acked_clusters_.push_back(cluster);
	max_bandwidth_.Update(bw,cluter_id);
    //std::cout<<cluter_id<<" bw "<<bw.ToKBitsPerSecond()<<std::endl;
}
void MyBbrSender::UpdateRtt(QuicTime now,
		QuicPacketNumber packet_number){
	auto it=seq_ts_map_.find(packet_number);
	if(it!=seq_ts_map_.end()){
		QuicTime::Delta rtt=now-it->second;
		if(min_rtt_==QuicTime::Delta::Zero()){
			min_rtt_=rtt;
			min_rtt_timestamp_=now;
		}else {
			if(rtt<min_rtt_){
				min_rtt_=rtt;
				min_rtt_timestamp_=now;
			}
			if(rtt>min_rtt_&&rtt<=min_rtt_*kSimilarMinRttThreshold){
				min_rtt_timestamp_=now;
			}
		}
	}
	while(!seq_ts_map_.empty()){
		auto it=seq_ts_map_.begin();
		if(it->first<=packet_number){
			seq_ts_map_.erase(it);
		}else{
			break;
		}
	}
}
void MyBbrSender::EnterProbeBandwidthMode(QuicTime now){
	if(mode_==PROBE_BW){
		return;
	}
	mode_ = PROBE_BW;
	cycle_current_offset_ = random_->RandUint64() % (kGainCycleLength - 1);
	if (cycle_current_offset_ >= 1) {
	   cycle_current_offset_ += 1;
	 }
	 last_cycle_start_ = now;
	 pacing_gain_ = kPacingGain[cycle_current_offset_];
	 round_trip_count_++;
}
void MyBbrSender::UpdateGainCyclePhase(QuicTime now){
	 bool should_advance_gain_cycling = (now - last_cycle_start_)>min_rtt_;
	 if(should_advance_gain_cycling){
		 cycle_current_offset_ = (cycle_current_offset_ + 1) % kGainCycleLength;
		 last_cycle_start_ = now;
		 pacing_gain_ = kPacingGain[cycle_current_offset_];
		 round_trip_count_++;
	 }
}
void MyBbrSender::MaybeEnterOrExitProbeRtt(QuicTime now,
                              bool min_rtt_expired){
	if (min_rtt_expired&& mode_ != PROBE_RTT) {
		mode_ = PROBE_RTT;
	    pacing_gain_ = 1;
	    // Do not decide on the time to exit PROBE_RTT until the |bytes_in_flight|
	    // is at the target small value.
	    exit_probe_rtt_at_ = QuicTime::Zero();
	    round_trip_count_=0;
	    recored_min_rtt_=min_rtt_;
	    min_rtt_=QuicTime::Delta::Zero();
	    seq_ts_map_.clear();
		ClusterInfoClear();
	  }
	if(mode_==PROBE_RTT){
		if(exit_probe_rtt_at_==QuicTime::Zero()){
			exit_probe_rtt_at_=now+kProbeRttTime;
		}
		if(now>exit_probe_rtt_at_||HasSampledLastMinRtt()){
			recored_min_rtt_=QuicTime::Delta::Zero();
			EnterProbeBandwidthMode(now);
		}
	}
}
void MyBbrSender::CalculatePacingRate(){
	QuicBandwidth target_rate=QuicBandwidth::Zero();
	QuicBandwidth min_rate=QuicBandwidth::FromBitsPerSecond(min_bps_);
	if(mode_==PROBE_RTT||mode_==START_UP){
		pacing_rate_=min_rate;
	}
	if(mode_==PROBE_BW){
		 //QuicBandwidth average_bw=AverageBandwidthEstimate();
		 target_rate = pacing_gain_*BandwidthEstimate();
		 pacing_rate_=target_rate;
         //if(pacing_gain_>1){
         //   std::cout<<" bw "<<pacing_rate_.ToKBitsPerSecond()<<std::endl;
         //}
		 if(pacing_rate_<min_rate){
			 pacing_rate_=min_rate;
		 }
	}
}
void MyBbrSender::AddPacketInfoSampler(QuicTime now,
		QuicPacketNumber packet_number,
		QuicByteCount bytes){
	auto it=connection_info_.find(round_trip_count_);
	SentClusterInfo *cluster=NULL;
	if(it!=connection_info_.end()){
		cluster=it->second;
	}else{
		SentClusterInfo *prev=NULL;
		if(connection_info_.size()>0){
			auto r_it=connection_info_.rbegin();
			prev=r_it->second;
		}
		cluster=new SentClusterInfo(round_trip_count_,this);
		cluster->SetPrev(prev);
		if(prev){
			prev->SetNext(cluster);
		}
		connection_info_.insert(std::make_pair(round_trip_count_,cluster));
	}
	cluster->OnPacketSent(now,packet_number,bytes);
	seq_round_map_.insert(std::make_pair(packet_number,round_trip_count_));
}
void MyBbrSender::ClusterInfoClear(){
	max_bandwidth_.Reset(QuicBandwidth::Zero(),0);
	avergae_bandwidth_.Reset(QuicBandwidth::Zero(),0);
	seq_round_map_.clear();
	acked_clusters_.clear();
	average_round_=0;
	while(!connection_info_.empty()){
		SentClusterInfo *cluster=NULL;
		auto it=connection_info_.begin();
		cluster=it->second;
		delete cluster;
		connection_info_.erase(it);
	}
}
void MyBbrSender::RemoveClusterLE(uint64_t round){
	while(!connection_info_.empty()){
        uint32_t total=connection_info_.size();
		auto it=connection_info_.begin();
		uint64_t id=it->first;
		SentClusterInfo *cluster=NULL;
		if(id<=round){
            cluster=it->second;
			SentClusterInfo *next=cluster->GetNext();
			if(next){
				next->SetPrev(NULL);
			}
            delete cluster;
			connection_info_.erase(it);	
		}else{
			break;
		}
	}
}
void MyBbrSender::AddSeqAndTimestamp(QuicTime now,QuicPacketNumber packet_number){
	seq_ts_map_.insert(std::make_pair(packet_number,now));
}
bool MyBbrSender::HasSampledLastMinRtt(){
	bool ret=false;
	if(min_rtt_!=QuicTime::Delta::Zero()&&recored_min_rtt_!=QuicTime::Delta::Zero()){
		if(min_rtt_<recored_min_rtt_*kSimilarMinRttThreshold){
			ret=true;
		}
	}
	return ret;
}
}
