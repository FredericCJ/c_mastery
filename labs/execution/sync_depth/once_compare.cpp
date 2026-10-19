// Hosted C++23 comparison only. Recursive local-static initialization is
// undefined; no runtime recursion experiment is performed.
#include <atomic>
#include <cstdio>
#include <mutex>
#include <thread>
#include <type_traits>
#ifndef CDO_COMPARE_MUTANT
#define CDO_COMPARE_MUTANT 0
#endif
#define CHECK(c) do{if(!(c)){std::fprintf(stderr,"CDOCPP_CHECK:%d:%s\n",__LINE__,#c);return 1;}}while(0)
struct Table{unsigned a,b,sum;};
static unsigned attempts,effects,live,once_attempts,once_effects;
static Table once_table;
static std::once_flag flag;
static std::atomic<unsigned> start{0};
struct Temporary{
  int *p;
  Temporary():p(new int(7)){live++;}
  ~Temporary(){delete p;live--;}
};
static Table make_table(){
  attempts++;Temporary temporary;
  effects++; // A synthetic external effect is not rolled back by the language.
  if(attempts==1)throw 17;
  return {17,23,40};
}
static const Table &local_table(){static const Table value=make_table();return value;}
static void initialize_once(){
  once_attempts++;once_effects++;
  if(once_attempts==1)throw 23;
  once_table={17,23,40};
}
int main(){
  static_assert(!std::is_copy_constructible_v<std::once_flag>);
  bool caught=false;
  try{(void)local_table();}catch(int v){caught=v==17;}
  CHECK(caught&&attempts==1&&effects==1&&live==0);
  caught=false;
  try{std::call_once(flag,initialize_once);}catch(int v){caught=v==23;}
  CHECK(caught&&once_attempts==1&&once_effects==1);
  int result[3]={-1,-1,-1};std::thread threads[3];
  for(unsigned i=0;i<3;i++)threads[i]=std::thread([i,&result]{
    while(!start.load(std::memory_order_acquire))std::this_thread::yield();
    const Table &a=local_table();std::call_once(flag,initialize_once);
    result[i]=(a.a!=17||a.b!=23||a.sum!=40||once_table.sum!=40);
  });
  start.store(1,std::memory_order_release);
  for(auto &t:threads)t.join();
  for(int r:result)CHECK(r==0);
  std::printf("CDOCPP_POLICY:local_attempts=%u;local_effects=%u;once_attempts=%u;once_effects=%u;live=%u\n",attempts,effects,once_attempts,once_effects,live);
  CHECK(attempts==(CDO_COMPARE_MUTANT?1u:2u)&&effects==2&&once_attempts==2&&once_effects==2&&live==0);
  std::puts("CDOCPP_READY:threads3;exceptions-retry;one-returning-initialization;ordinary-table;RAII-cleanup;external-effect-not-rolled-back");return 0;
}
