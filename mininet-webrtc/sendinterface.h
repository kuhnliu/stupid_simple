#ifndef SENDINTERFACE_H_
#define SENDINTERFACE_H_
namespace zsy{
class SendInterface{
public:
	virtual void SendVideo(uint8_t payload_type,int ftype,void *data,uint32_t len)=0;
	virtual ~SendInterface(){}
};
}
#endif /* SENDINTERFACE_H_ */
