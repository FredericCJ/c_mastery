#include "integer.h"
#include <limits.h>
static void emit(const char *s) {
 register unsigned r0 __asm__("r0")=4;register const char *r1 __asm__("r1")=s;
 __asm__ volatile("bkpt 0xab":"+r"(r0),"+r"(r1)::"memory");
}
_Noreturn static void stop(unsigned rc) {
 unsigned args[2]={0x20026,rc};register unsigned r0 __asm__("r0")=0x20;register unsigned *r1 __asm__("r1")=args;
 __asm__ volatile("bkpt 0xab":"+r"(r0),"+r"(r1)::"memory");for(;;){}
}
#define T(x) do{if(!(x)){emit("FI_TARGET_FAIL\n");stop(1);}}while(0)
void fi_fault(void){emit("FI_TARGET_FAULT\n");stop(2);}
void fi_target_main(void) {
 _Static_assert(CHAR_BIT==8 && INT_MAX==2147483647 && SIZE_MAX==4294967295U,"recorded M4 data model");
 _Static_assert(INTMAX_MAX>SIZE_MAX,"actual wide-input/narrow-size boundary");
 struct fi_range r={INT_MIN,INT_MAX};int value=917,q=918,rem=919;unsigned clipped=9;size_t out=917;
 T(fi_binary(r,FI_ADD,INT_MAX,1,&value)==FI_RANGE&&value==917);
 T(fi_binary(r,FI_MULTIPLY,INT_MIN,-1,&value)==FI_RANGE&&value==917);
 T(fi_binary(r,FI_SUBTRACT,INT_MIN,1,&value)==FI_RANGE&&value==917);
 T(fi_saturate(r,FI_ADD,INT_MAX,1,&value,&clipped)==FI_OK&&value==INT_MAX&&clipped==1);
 T(fi_divide(r,FI_TRUNCATE,INT_MIN,-1,&q,&rem)==FI_RANGE&&q==918&&rem==919);
 T(fi_divide(r,FI_TRUNCATE,1,0,&q,&rem)==FI_RANGE&&q==918&&rem==919);
 T(fi_divide(r,FI_FLOOR,7,-3,&q,&rem)==FI_OK&&q==-3&&rem==-2);
 T(fi_divide(r,FI_EUCLIDEAN,-1,INT_MIN,&q,&rem)==FI_OK&&q==1&&rem==INT_MAX);
 T(fi_mod_positive(INT_MIN,INT_MAX,&value)==FI_OK&&value==INT_MAX-1);
 T(fi_to_size((intmax_t)SIZE_MAX,SIZE_MAX,&out)==FI_OK&&out==SIZE_MAX);
 out=917;T(fi_to_size((intmax_t)SIZE_MAX+1,SIZE_MAX,&out)==FI_RANGE&&out==917);
 T(fi_layout(12,-1,9,180,&out)==FI_RANGE&&out==917);
 T(fi_layout(12,18,9,180,&out)==FI_OK&&out==174);
 out=917;T(fi_extent(SIZE_MAX,1,1,SIZE_MAX,&out)==FI_RANGE&&out==917);
 uintmax_t base=(uintmax_t)PTRDIFF_MAX+1;int local[]={11,22,33,44};
 T(fi_window(base,base+2,4,&out)==FI_OK&&local[out]==33);
 unsigned int cells[4];unsigned char octets[4];uint_least32_t wire=0;
 T(fi_encode_cells(UINT32_C(0x12345678),cells)==FI_OK&&cells[0]==18&&cells[3]==120);
 T(fi_decode_cells(cells,&wire)==FI_OK&&wire==UINT32_C(0x12345678));
 T(fi_encode_octets(wire,octets)==FI_OK&&octets[1]==52&&octets[2]==86);
 emit("FI_TARGET_PASS manual_arithmetic no_wider_guards size32_input64 logical_offsets wire_octets\n");stop(0);
}
