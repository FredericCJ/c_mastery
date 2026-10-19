#include <chrono>
#include <cstdio>
#include <cstdint>
#include <ratio>
int main(){
    using ticks=std::chrono::duration<std::int64_t,std::ratio<3,2000000>>;
    auto a=std::chrono::duration_cast<ticks>(std::chrono::microseconds(5));
    auto b=std::chrono::duration_cast<ticks>(std::chrono::microseconds(-5));
    auto c=std::chrono::duration_cast<std::chrono::microseconds>(ticks(3));
    if(a.count()!=3||b.count()!=-3||c.count()!=4)return 1;
    std::puts("ADQ_CPP admitted-positive3 negative-3 reverse4");
}
