#ifndef MYBBR_MY_PACING_SENDER_H_
#define MYBBR_MY_PACING_SENDER_H_
#include "my_bbr_sender.h"
namespace net{
class MyPacingSender{
public:
	MyPacingSender();
	~MyPacingSender();
	void set_sender(MyBbrSender *sender){
        sender_=sender;
    }
	void OnPacketSent(QuicTime now,
			QuicPacketNumber packet_number,
			 QuicByteCount bytes);
	void OnCongestionEvent(QuicTime now,
			QuicPacketNumber packet_number);
	QuicTime::Delta TimeUntilSend(QuicTime now) const;
private:
	 QuicTime ideal_next_packet_send_time_;
	 QuicTime::Delta alarm_granularity_;
	 MyBbrSender *sender_{NULL};
};
}



#endif /* MYBBR_MY_PACING_SENDER_H_ */
