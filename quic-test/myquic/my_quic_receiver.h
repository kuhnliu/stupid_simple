#ifndef MY_QUIC_RECEIVER_H_
#define MY_QUIC_RECEIVER_H_
#include "socket.h"
#include "net/quic/core/quic_received_packet_manager.h"
#include "net/quic/core/quic_connection_stats.h"
#include "my_quic_clock.h"
#include <string>
#include <iostream>
#include <fstream>
namespace net{
class MyQuicAlarm{
public:
	MyQuicAlarm() : m_deadline(QuicTime::Zero()) {}
  bool IsSet() const {
	  return m_deadline >QuicTime::Zero();
  }
  bool IsExpired(QuicTime now_ms)
  {
      bool ret = false;
      if (IsSet())
      {
          ret = now_ms >= m_deadline;
          if (ret)
          {
              m_deadline = QuicTime::Zero();
          }
      }
      return ret;
  }
  void Update(QuicTime new_deadline_ms)
  {
      m_deadline = new_deadline_ms;
  }

private:
  QuicTime m_deadline;
};
class MyQuicReceiver{
public:
	MyQuicReceiver();
    ~MyQuicReceiver();
	void OnIncomingData(char *data, int len);
	bool Process();
	void set_socket(zsy::Socket *socket) { socket_=socket;}
	void set_duration(uint32_t ms){ duration_=ms;}
    void SetTimeOffset(int64_t offset){
        offset_=offset;
    }
    void SetRecordPrefix(std::string name);
    void EnableRateRecord();
    void EnableLossRecord();
private:
	void MaybeSendAck();
	void SendAck();
    void RecordLoss(uint64_t seq);
    void RecordRate(QuicTime now);
	bool first_{true};
	zsy::Socket *socket_{NULL};
	su_addr peer_;
	QuicConnectionStats stats_;
	QuicReceivedPacketManager recv_packet_manager_;
	MyQuicClock clock_;
	ParsedQuicVersionVector  versions_;
	bool ack_sent_{false};
	uint64_t seq_{1};
	uint64_t base_seq_{0};
	QuicTime stop_;
	uint32_t duration_{0};
	MyQuicAlarm ack_alarm_;
	uint64_t received_{0};
	uint64_t m_num_packets_received_since_last_ack_sent{0};
    int64_t offset_{0};
    QuicTime ref_time_;
    QuicTime next_output_;
    uint64_t recv_byte_{0};
    bool enable_loss_record_{false};
    bool enable_rate_record_{false};
    std::fstream f_loss_;
    std::fstream f_rate_;
    std::string log_prefix_;
};
}




#endif /* MY_QUIC_RECEIVER_H_ */
