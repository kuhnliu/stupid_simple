#ifndef MYBBR_MY_BBR_RECEIVER_APP_H_
#define MYBBR_MY_BBR_RECEIVER_APP_H_
#include "socket.h"
#include "my_quic_clock.h"
#include <string>
#include <iostream>
#include <fstream>
namespace net{
class MyBbrReceiverApp{
public:
	MyBbrReceiverApp();
	~MyBbrReceiverApp();
	void set_socket(zsy::Socket *socket) {
		socket_=socket;
	}
	void set_duration(uint32_t ms){
		duration_=ms;
	}
    void SetTimeOffset(int64_t offset){
        offset_=offset;
    }
    bool Process();
    void SetRecordPrefix(std::string name);
    void EnableRateRecord();
    void EnableLossRecord();
private:
    void OnIncomingData(char *data, int len);
    void SendAck(uint64_t packet_number);
    void RecordLoss(uint64_t seq);
    void RecordRate(QuicTime now);
    bool first_{true};
	zsy::Socket *socket_{NULL};
	su_addr peer_;
	MyQuicClock clock_;
	uint64_t base_seq_{0};
	QuicTime stop_;
	uint32_t duration_{0};
	QuicTime ref_time_;
	QuicTime next_;
    uint64_t recv_byte_{0};
    int64_t offset_{0};
    bool enable_loss_record_{false};
    bool enable_rate_record_{false};
    std::fstream f_loss_;
    std::fstream f_rate_;
    std::string log_prefix_;
};
}




#endif /* MYBBR_MY_BBR_RECEIVER_APP_H_ */
