/*
 * fakevideogenerator.h
 *
 *  Created on: 2018Äê12ÔÂ19ÈÕ
 *      Author: LENOVO
 */

#ifndef MYQUIC_FAKEVIDEOGENERATOR_H_
#define MYQUIC_FAKEVIDEOGENERATOR_H_
#include "net/quic/core/quic_time.h"
#include <stdint.h>
namespace  net{
class FakeVideoGenerator{
public:
	FakeVideoGenerator(int fs,int start_rate);
	~FakeVideoGenerator(){}
	void UpdataRate(int64_t bps);
	void SetRateLimit(int64_t max);
	int64_t GetMaxRate(){
		return max_rate_;
	}
	int64_t GetFrame(QuicTime now);
private:
	int64_t min_rate_{1000000};//500kbps;
	int64_t rate_{1000000};
	int64_t max_rate_{2000000};//5Mbps;
	int64_t duration_{33};//33ms
	QuicTime next_;
};
}
#endif /* MYQUIC_FAKEVIDEOGENERATOR_H_ */
