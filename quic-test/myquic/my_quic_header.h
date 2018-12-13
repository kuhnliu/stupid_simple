#ifndef MYQUIC_MY_QUIC_HEADER_H_
#define MYQUIC_MY_QUIC_HEADER_H_
namespace net{
typedef struct{
	uint8_t seq_len;
	uint64_t seq;
}my_quic_header_t;
}
#endif /* MYQUIC_MY_QUIC_HEADER_H_ */
