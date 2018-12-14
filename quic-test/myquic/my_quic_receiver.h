#ifndef MY_QUIC_RECEIVER_H_
#define MY_QUIC_RECEIVER_H_
#include "socket.h"
#include "net/quic/core/quic_received_packet_manager.h"
#include "net/quic/core/quic_connection_stats.h"
#include "my_quic_clock.h"
namespace net{
class MyQuicReceiver{
public:
	MyQuicReceiver();
	void OnIncomingData(char *data, int len);
	bool Process();
	void set_socket(zsy::Socket *socket) { socket_=socket;}
	void set_duration(uint32_t ms){ duration_=ms;}
private:
	void SendAck();
	bool first_{true};
	uint32_t counter_{0};
	zsy::Socket *socket_{NULL};
	su_addr peer_;
	QuicConnectionStats stats_;
	QuicReceivedPacketManager recv_packet_manager_;
	MyQuicClock clock_;
	ParsedQuicVersionVector  versions_;
	bool ack_sent_{false};
	uint64_t seq_{1};
	QuicTime stop_;
	uint32_t duration_{0};
};
}




#endif /* MY_QUIC_RECEIVER_H_ */
