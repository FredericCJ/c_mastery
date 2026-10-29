#ifndef CMASTERY_PARSER_FOLLOW_H
#define CMASTERY_PARSER_FOLLOW_H
#include "coroutines.h"
enum{CP_LITERAL,CP_SEQUENCE,CP_CHOICE,CP_CUT,CP_EMPTY,CP_COMMITTED=10};
struct cp_node{unsigned kind,left,right;unsigned char literal;};
struct cp_diagnostic{size_t offset;unsigned char expected[4];unsigned count,truncated,steps,committed;};
int cp_parse(const struct cp_node*,size_t,const unsigned char*,size_t,unsigned eof,unsigned budget,size_t*,struct cp_diagnostic*);
struct ce_memo{const unsigned char*input;size_t length;uint64_t input_revision,environment_revision;unsigned mask,valid,state,success;size_t end;int32_t value;};
int cp_left_expression(struct ce_memo*,const unsigned char*,size_t,uint64_t,uint64_t,unsigned digit_mask,unsigned budget,size_t*,int32_t*,unsigned*);
#endif
