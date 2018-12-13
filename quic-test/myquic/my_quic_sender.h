#ifndef MY_QUIC_SENDER_H_
#define MY_QUIC_SENDER_H_
#include "net/quic/core/quic_sent_packet_manager.h"
#include "net/quic/core/quic_connection_stats.h"
#include "net/quic/core/quic_versions.h"
#include "net/quic/core/quic_pending_retransmission.h"
#include "my_quic_framer.h"
#include "my_quic_clock.h"
#include "quic_framer_visitor.h"
#include "socket.h"
namespace net{
class MyQuicSender:public AbstractQuicFramerVisitor{
public:
	MyQuicSender(Perspective pespective);
	~MyQuicSender(){}
	void OnIncomingData(uint8_t *data,int len);
	bool OnAckFrame(const QuicAckFrame& frame) override;
	bool OnAckFrameStart(QuicPacketNumber largest_acked,
		                 QuicTime::Delta ack_delay_time) override;
	bool OnAckRange(QuicPacketNumber start,
		            QuicPacketNumber end,
		            bool last_range) override;
	bool Process();
	void set_socket(zsy::Socket *socket) { socket_=socket;}
	void set_peer(su_addr peer) { peer_=peer; }
private:
	void OnPacketSent(QuicPacketNumber packet_number,
                   QuicPacketNumberLength packet_number_length, QuicPacketLength encrypted_length);
	void SendFakePacket();
	void SendRetransmission();
	void OnRetransPacket(QuicPendingRetransmission pending);
	Perspective pespective_;
	MyQuicClock clock_;
	QuicConnectionStats stats_;
	QuicSentPacketManager sent_packet_manager_;
	ParsedQuicVersionVector  versions_;
	QuicTime time_of_last_received_packet_;
	zsy::Socket *socket_{NULL};
	su_addr peer_;
	uint32_t seq_{1};
	uint32_t counter_{0};
	QuicTime next_;
	QuicTime stop_;
};
}




#endif /* MY_QUIC_SENDER_H_ */
