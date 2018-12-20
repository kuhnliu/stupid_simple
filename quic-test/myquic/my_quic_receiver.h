#ifndef MY_QUIC_RECEIVER_H_
#define MY_QUIC_RECEIVER_H_
#include "socket.h"
#include "net/quic/core/quic_received_packet_manager.h"
#include "net/quic/core/quic_connection_stats.h"
#include "my_quic_clock.h"
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
	void OnIncomingData(char *data, int len);
	bool Process();
	void set_socket(zsy::Socket *socket) { socket_=socket;}
	void set_duration(uint32_t ms){ duration_=ms;}
private:
	void MaybeSendAck();
	void SendAck();
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
};
}




#endif /* MY_QUIC_RECEIVER_H_ */
