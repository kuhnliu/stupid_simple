#ifndef NULL_ALARM_H
#define NULL_ALARM_H
#include <stdio.h>
#include "quic_alarm.h"
namespace quic{
class NullAlarm:public QuicAlarm{
public:
     NullAlarm(QuicArenaScopedPtr<Delegate> delegate)
     :QuicAlarm(std::move(delegate)){

     }
    ~NullAlarm(){}
    void Trigger(){
        Fire();
    }
private:
    void SetImpl() override{}
    void CancelImpl() override{}
    void UpdateImpl() override{}
};
class NullDelegate:public QuicAlarm::Delegate{
public:
    NullDelegate(){}
    ~NullDelegate(){
        printf("dor nulldelegate\n");
    }
    void OnAlarm(){
        printf("alarm %d\n",counter_);
        counter_++;
    }
private:
    int counter_{0};
};
}
#endif // NULL_ALARM_H
