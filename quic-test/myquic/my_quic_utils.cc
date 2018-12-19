#include "my_quic_utils.h"
namespace net{
uint16_t FrameSplit(uint16_t splits[], size_t size){
	uint16_t ret, i;
	uint16_t remain_size;

	if (size <= QUIC_MAX_VIDEO_SIZE){
		ret = 1;
		splits[0] = size;
	}
	else{
		ret = size / QUIC_MAX_VIDEO_SIZE;
		for (i = 0; i < ret; i++)
			splits[i] = QUIC_MAX_VIDEO_SIZE;

		remain_size = size % QUIC_MAX_VIDEO_SIZE;
		if (remain_size > 0){
			splits[ret] = remain_size;
			ret++;
		}
	}
	return ret;
}
}




