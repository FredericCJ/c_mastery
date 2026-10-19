#include <algorithm>
#include <cstdio>
#include <memory>
#include <vector>
struct Item{unsigned id;int value;static unsigned destroyed;~Item(){destroyed|=1u<<id;}};
unsigned Item::destroyed=0;
int main(){std::vector<int>a{1,2,1,3};int needle=a[0];auto end=std::remove(a.begin(),a.end(),needle);if(a.size()!=4||end-a.begin()!=2||a[0]!=2||a[1]!=3)return 1;a.erase(end,a.end());if(a.size()!=2)return 2;
 std::vector<std::unique_ptr<Item>> owned;for(unsigned i=0;i<4;++i)owned.emplace_back(new Item{i+1,i%2?2:1});auto tail=std::remove_if(owned.begin(),owned.end(),[](const auto&p){return p->value==1;});if(owned.size()!=4||tail-owned.begin()!=2||owned[0]->id!=2||owned[1]->id!=4)return 3;unsigned before=Item::destroyed;owned.erase(tail,owned.end());if(Item::destroyed!=((1u<<1)|(1u<<3)))return 4;owned.clear();if(Item::destroyed!=30)return 5;
 std::vector<int> alias{1,2,1,3};auto changed=std::remove(alias.begin(),alias.end(),alias.front());std::printf("ER_CPP_PASS size_before4 size_after2 resources_before_erase_mask%u alias_prefix",before);for(auto i=alias.begin();i!=changed;++i)std::printf(" %d",*i);std::puts("; observed library behavior, tail states unspecified");return 0;}
