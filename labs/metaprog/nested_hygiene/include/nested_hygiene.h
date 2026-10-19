#ifndef CMASTERY_NESTED_HYGIENE_H
#define CMASTERY_NESTED_HYGIENE_H
#include "metaprog.h"
enum { MPH_OK,MPH_INVALID,MPH_SHAPE,MPH_BUDGET,MPH_EXHAUSTED,MPH_CAPTURE,MPH_NOT_FOUND };
enum { MPH_LITERAL,MPH_ARGUMENT,MPH_INTRODUCED,MPH_CALL,MPH_REPEAT,MPH_ESCAPE };
typedef struct {unsigned operation,target,mask;MPToken token;} MphInstruction;
typedef struct {MphInstruction code[16];size_t count;} MphDefinition;
typedef struct {MphDefinition definitions[4];size_t count;} MphProgram;
typedef struct {MPToken tokens[4];size_t count;} MphArguments;
typedef struct {unsigned depth,tokens,steps,allow_capture;} MphBudget;
typedef struct {MPToken tokens[64];size_t count;unsigned invocations,steps,peak_depth;} MphExpansion;
typedef struct {MPToken identifier;int value;} MphBinding;
int mph_expand(const MphProgram *program,unsigned entry,const MphArguments arguments[2],unsigned caller_context,
               MphBudget budget,unsigned *next_context,MphExpansion *output);
int mph_resolve(const MphBinding *environment,size_t count,MPToken identifier,int *output);
#endif
