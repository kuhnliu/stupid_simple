#include "quic_arena_scoped_ptr.h"
#include <stdio.h>
#include <stdint.h>
class Test{
public:
    Test(uint32_t i):i_(i){
    }
    ~Test(){
        printf("dtor %d\n",i_);
    }
private:
    uint32_t i_{0};
};
template<typename T>
void test_move(quic::QuicArenaScopedPtr<T> arg){
    quic::QuicArenaScopedPtr<T> a=std::move(arg);

}
quic::QuicArenaScopedPtr<Test> g_alarm;
void test_move2(quic::QuicArenaScopedPtr<Test> test){
    g_alarm=std::move(test);
}
void test_fun(){
    quic::QuicArenaScopedPtr<Test> alarm(new Test(1));
    test_move2(std::move(alarm));
    printf("test fun\n");
}
int main(){
    test_fun();
    printf("you know what\n");
    return 0;
}
