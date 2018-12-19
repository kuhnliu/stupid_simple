#ifndef MYQUIC_MY_QUIC_UTILS_H_
#define MYQUIC_MY_QUIC_UTILS_H_
#include <stdint.h>
#include <stddef.h>
namespace net{
#define MAX_SPLIT_NUMBER	1024
#define QUIC_MAX_VIDEO_SIZE 1000
uint16_t FrameSplit(uint16_t splits[], size_t size);
}
#endif /* MYQUIC_MY_QUIC_UTILS_H_ */
