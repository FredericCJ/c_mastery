#ifndef CMASTERY_NESTED_HYGIENE_H
#define CMASTERY_NESTED_HYGIENE_H
#include "metaprog.h"
enum { MPH_OK,MPH_INVALID,MPH_SHAPE,MPH_BUDGET,MPH_EXHAUSTED,MPH_CAPTURE,MPH_NOT_FOUND };
enum { MPH_LITERAL,MPH_ARGUMENT,MPH_INTRODUCED,MPH_CALL,MPH_REPEAT,MPH_ESCAPE };
struct mph_instruction {unsigned operation,target,mask;struct mp_token token;};
struct mph_definition {struct mph_instruction code[16];size_t count;};
struct mph_program {struct mph_definition definitions[4];size_t count;};
struct mph_arguments {struct mp_token tokens[4];size_t count;};
struct mph_budget {unsigned depth,tokens,steps,allow_capture;};
struct mph_expansion {struct mp_token tokens[64];size_t count;unsigned invocations,steps,peak_depth;};
struct mph_binding {struct mp_token identifier;int value;};
int mph_expand(const struct mph_program *program,unsigned entry,const struct mph_arguments arguments[2],unsigned caller_context,
               struct mph_budget budget,unsigned *next_context,struct mph_expansion *output);
int mph_resolve(const struct mph_binding *environment,size_t count,struct mp_token identifier,int *output);
#endif
