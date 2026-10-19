#ifndef CMASTERY_PARSER_FOLLOW_H
#define CMASTERY_PARSER_FOLLOW_H
#include "coroutines.h"
enum{CP_LITERAL,CP_SEQUENCE,CP_CHOICE,CP_CUT,CP_EMPTY,CP_COMMITTED=10};
typedef struct{unsigned kind,left,right;unsigned char literal;} CPNode;
typedef struct{size_t offset;unsigned char expected[4];unsigned count,truncated,steps,committed;} CPDiagnostic;
int cp_parse(const CPNode*,size_t,const unsigned char*,size_t,unsigned eof,unsigned budget,size_t*,CPDiagnostic*);
typedef struct{const unsigned char*input;size_t length;uint64_t input_revision,environment_revision;unsigned mask,valid,state,success;size_t end;int32_t value;} CEMemo;
int cp_left_expression(CEMemo*,const unsigned char*,size_t,uint64_t,uint64_t,unsigned digit_mask,unsigned budget,size_t*,int32_t*,unsigned*);
#endif
