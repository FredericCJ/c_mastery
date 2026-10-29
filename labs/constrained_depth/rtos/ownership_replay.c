#include "ownership.h"
#include <stdio.h>
int main(void){
 for(unsigned word=0;word<6561;word++){
  struct cr_pool pool;struct cr_token normal={1,0,0},fault={1,0,2};unsigned char byte=42;unsigned code=word;int results[4];
  if(cr_pool_init(&pool,1)!=CR_OK)return 2;
  for(unsigned i=0;i<4;i++){unsigned op=code%9;code/=9;switch(op){
   case 0:results[i]=cr_allocate(&pool,0,&byte,1,&normal);break;
   case 1:results[i]=cr_allocate(&pool,1,&byte,1,&fault);break;
   case 2:results[i]=cr_offer(&pool,normal);break;
   case 3:results[i]=cr_claim(&pool,normal);break;
   case 4:results[i]=cr_abort_offer(&pool,normal);break;
   case 5:results[i]=cr_release(&pool,normal);break;
   case 6:results[i]=cr_close(&pool);break;
   case 7:results[i]=cr_drained(&pool);break;
   default:results[i]=cr_release(&pool,fault);break;
  }}
  printf("P,%u,%d,%d,%d,%d,%u,%u,%u,%u,%llu,%llu,%llu\n",word,results[0],results[1],results[2],results[3],pool.closed,pool.slots[0].state,pool.slots[1].state,pool.slots[2].state,(unsigned long long)pool.slots[0].generation,(unsigned long long)pool.slots[1].generation,(unsigned long long)pool.slots[2].generation);
 }
 return 0;
}
