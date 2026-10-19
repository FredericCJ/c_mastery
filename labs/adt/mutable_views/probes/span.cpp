#include <span>
#include <type_traits>
int main(){int a[6]={1,2,3,4,5,6};std::span<int> all(a);const std::span<int> copied=all;copied[0]=8;auto sub=all.subspan(2,2);sub[1]=9;std::span<const int> read=all;static_assert(!std::is_constructible_v<std::span<int>,std::span<const int>>);return read[0]==8&&a[3]==9&&sub.size()==2?0:1;}
