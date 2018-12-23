#ifndef MYBBR_MY_BBR_SENDER_H_
#define MYBBR_MY_BBR_SENDER_H_
#include "net/quic/core/quic_types.h"
#include "net/quic/core/quic_time.h"
#include "net/quic/core/quic_bandwidth.h"
#include "net/quic/core/congestion_control/windowed_filter.h"
#include "net/quic/core/congestion_control/bandwidth_sampler.h"
#include "net/quic/core/crypto/quic_random.h"
#include <map>
#include <vector>
#include <list>
namespace net{
struct SentPacketInfo{
	SentPacketInfo(QuicPacketNumber seq,
			QuicTime sent_ts,
			QuicTime ack_ts,
			uint64_t len)
	:seq(seq)
	,sent_ts(sent_ts)
	,ack_ts(ack_ts)
	,len(len){}
	QuicPacketNumber seq;
	QuicTime sent_ts;
	QuicTime ack_ts;
	uint64_t len;
};
class SentClusterInfo;
class BandwidthEstimateInteface{
public:
	virtual ~BandwidthEstimateInteface(){}
	virtual	void OnEstimateBandwidth(SentClusterInfo *cluster,uint64_t cluster_id,QuicBandwidth bw)=0;
};
class SentClusterInfo{
public:
	SentClusterInfo(uint64_t cluster_id,BandwidthEstimateInteface *target);
	~SentClusterInfo();
	void SetPrev(SentClusterInfo *prev){
		prev_=prev;
	}
	void SetNext(SentClusterInfo *next){
		next_=next;
	}
	SentClusterInfo* GetNext(){
		return next_;
	}
	uint64_t GetClusterId(){return cluster_id_;}
	void OnPacketSent(QuicTime now,
			QuicPacketNumber seq,
			uint64_t len);
	void OnAck(QuicPacketNumber seq,
			QuicTime now);
	void GetSentInfo(uint64_t *ms,QuicByteCount *sent);
	void GetAckedInfo(uint64_t *ms,QuicByteCount *acked);
private:
	void TriggerBandwidthEstimate();
	uint64_t cluster_id_{0};
	QuicTime first_send_ts_;
	QuicTime last_send_ts_;
	QuicTime first_ack_ts_;
	QuicTime last_ack_ts_;
	QuicByteCount acked_packets_{0};
	QuicByteCount sent_packets_{0};
	std::vector<SentPacketInfo*> packet_infos_;
	BandwidthEstimateInteface *bw_target_;
	SentClusterInfo *prev_{NULL};
	SentClusterInfo *next_{NULL};
	QuicBandwidth bw_;
};
class MyBbrSender:public BandwidthEstimateInteface{
public:
	//in mode START_UP PROBE_RTT,the minimal packet sent rate
	//was maintained.
	enum Mode{
		START_UP,
		PROBE_BW,
		PROBE_RTT,
	};
	MyBbrSender(uint64_t min_bps,
			uint64_t start_bps,
			uint64_t max_bps);
	~MyBbrSender();
	QuicBandwidth PaceRate();
	QuicBandwidth BandwidthEstimate();
	QuicBandwidth AverageBandwidthEstimate();
	bool ShouldSendProbePacket() const;
	void OnAck(QuicTime event_time,
			QuicPacketNumber packet_number);
	void OnPacketSent(QuicTime event_time,
			QuicPacketNumber packet_number,
			QuicByteCount bytes);
	void OnEstimateBandwidth(SentClusterInfo *cluster,uint64_t cluster_id,QuicBandwidth bw) override;
private:
	typedef WindowedFilter<QuicBandwidth,
	                       MaxFilter<QuicBandwidth>,
	                       uint64_t,
	                       uint64_t>
	      MaxBandwidthFilter;
	//may be 20 rtt;
	typedef WindowedFilter<QuicBandwidth,
	                       MaxFilter<QuicBandwidth>,
	                       uint64_t,
	                       uint64_t>
	      AverageBandwidthFilter;
	void UpdateRtt(QuicTime now,
			QuicPacketNumber packet_number);
	void EnterProbeBandwidthMode(QuicTime now);
	void UpdateGainCyclePhase(QuicTime now);
	  // Decides whether to enter or exit PROBE_RTT.
	void MaybeEnterOrExitProbeRtt(QuicTime now,
	                              bool min_rtt_expired);
	void CalculatePacingRate();
	void AddPacketInfoSampler(QuicTime now,
			QuicPacketNumber packet_number,
			QuicByteCount bytes);
	void ClusterInfoClear();
	void RemoveClusterLE(uint64_t round);
	void AddSeqAndTimestamp(QuicTime now,QuicPacketNumber packet_number);
	uint64_t min_bps_;
	uint64_t start_bps_;
	uint64_t max_bps_;
	Mode mode_{START_UP};
	uint64_t round_trip_count_{0};
	std::map<uint64_t,SentClusterInfo*> connection_info_;
	//for handle loss packet
	std::map<QuicPacketNumber,uint64_t> seq_round_map_;
	std::map<QuicPacketNumber,QuicTime> seq_ts_map_;
	QuicTime::Delta min_rtt_;
	QuicTime min_rtt_timestamp_;
	QuicTime exit_probe_rtt_at_;
	QuicBandwidth pacing_rate_;
	// The gain currently applied to the pacing rate.
	float pacing_gain_{1.0};
	int cycle_current_offset_{0};
	QuicTime last_cycle_start_;
	QuicPacketNumber last_ack_seq_{0};
	MaxBandwidthFilter max_bandwidth_;
	//get the average of the last 10 round.
	AverageBandwidthFilter avergae_bandwidth_;
	uint64_t average_round_{0};
	std::list<SentClusterInfo*> acked_clusters_;
	QuicRandom *random_;
};
}
#endif /* MYBBR_MY_BBR_SENDER_H_ */
