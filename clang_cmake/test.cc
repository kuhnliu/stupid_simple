#include <iostream>
#include <vector>
#include <string>
using namespace std;

int main(){
std::string out=std::string("hello world");
std::cout<<out<<std::endl;
std::vector<int> vec;
vec.push_back(1);
printf("size %zu\n",vec.size());
return 0;
}
