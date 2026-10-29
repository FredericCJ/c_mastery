#include "checkpoint_depth.h"
#include "protocol.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CPA_POLICY:%s\n",#x);return 41;}}while(0)
int main(void)
{
    struct cpx_message message={0};message.version=1;message.sequence=258;message.length=33;memset(message.payload,7,33);unsigned char wire[CPX_WIRE_MAX];size_t n,used;
    CHECK(cpx_protocol_encode(&message,wire,sizeof wire,&n)==CPX_P_OK && n==43);
    struct cp_parser original;cp_parser_init(&original);CHECK(cp_feed(&original,wire,n,&used)==CP_REJECT && used==6 && !original.ready);
    struct cpx_parser configured;CHECK(cpx_protocol_init(&configured,32)==CPX_P_OK && cpx_protocol_feed(&configured,wire,n,&used)==CPX_P_REJECT && used==6);
    CHECK(cpx_protocol_init(&configured,64)==CPX_P_OK && cpx_protocol_feed(&configured,wire,n,&used)==CPX_P_FRAME && used==n && configured.message.length==33);
    puts("CPA_POLICY_PASS original32/restricted32 refuse; configured64 accepts version1 length33");return 0;
}
