#include "capacity.h"
#include "metaprog.h"
_Static_assert(sizeof(size_t)==4,"MPC32_SIZE_T");
extern void mpc32_write(const char *,size_t);
#define REQUIRE(x) do{if(!(x)){mpc32_write("MPC32_FAIL\n",11);return 1;}}while(0)
int mpc32_test(void){
 size_t out=99;REQUIRE(!mp_storage_bound(SIZE_MAX,2,SIZE_MAX,&out)&&out==99);
 REQUIRE(mp_storage_bound(SIZE_MAX/4,4,SIZE_MAX,&out)&&out==UINT32_C(4294967292));
 REQUIRE(!mp_storage_bound(SIZE_MAX/4+1,4,SIZE_MAX,&out)&&out==UINT32_C(4294967292));
 REQUIRE(!mp_storage_bound(4,4,15,&out)&&out==UINT32_C(4294967292));
 uint32_t data[3],value;struct mpc_runtime ring;REQUIRE(mpc_init(&ring,data,3,sizeof data)==MPC_OK);
 for(unsigned n=0;n<20;n++){
  REQUIRE(mpc_push(&ring,n)==MPC_OK&&mpc_push(&ring,n+1)==MPC_OK&&mpc_push(&ring,n+2)==MPC_OK&&mpc_push(&ring,99)==MPC_FULL);
  REQUIRE(mpc_pop(&ring,&value)==MPC_OK&&value==n&&mpc_pop(&ring,&value)==MPC_OK&&value==n+1&&mpc_pop(&ring,&value)==MPC_OK&&value==n+2);
 }
 mpc32_write("MPC32_PASS size_t32 checked-product runtime3\n",sizeof "MPC32_PASS size_t32 checked-product runtime3\n"-1);return 0;
}
