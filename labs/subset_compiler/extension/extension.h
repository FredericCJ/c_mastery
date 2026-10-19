#ifndef CM_B06_EXTENSION_H
#define CM_B06_EXTENSION_H
#include <stddef.h>
#include <stdint.h>
#define CE_CODE 512
#define CE_FUNCS 4
#define CE_LOCALS 16
#define CE_STACK 32
#define CE_ARRAY 8
enum { CE_CONST=1,CE_LOAD,CE_STORE,CE_ADD,CE_SUB,CE_MUL,CE_LT,CE_EQ,CE_ADDR,CE_PADD,CE_READ,CE_WRITE,CE_JZ,CE_JMP,CE_CALL,CE_RET,CE_TO_I32,CE_TO_U32,CE_SADD,CE_SSUB,CE_SMUL,CE_SLT,CE_SEQ,CE_TICK };
enum { CE_U32=1,CE_PTR=2,CE_I32=3 };
typedef struct { unsigned op,arg; } CEIns;
typedef struct { unsigned begin,end,locals; unsigned types[CE_LOCALS]; char name[16]; } CEFunc;
typedef struct { unsigned count,functions; CEIns code[CE_CODE]; CEFunc fn[CE_FUNCS]; } CEProgram;
typedef struct { size_t begin,end; unsigned reason; } CEDiagnostic;
/* Compile publishes program only on success; diagnostics identify rejected span.
   Verify checks every instruction, reachable joins, type and call graph bounds.
   Run uses shared instruction fuel; failure preserves output, consumes fuel.
   Emit is hosted author tool: target0 SysV x86-64, target1 Thumb-2 AAPCS32, target2 Thumb-1 AAPCS32. */
int ce_compile(const char*,size_t,CEProgram*,CEDiagnostic*);
int ce_verify(const CEProgram*,unsigned heights[CE_CODE]);
int ce_run(const CEProgram*,uint32_t,unsigned*,uint32_t*);
int ce_emit(const CEProgram*,unsigned,char*,size_t);
/* Canonical CEIR1 byte format; no native struct layout or executable addresses.
 * Exact extent, valid typed CFG, no ignored/reserved bits. Failure preserves
 * output and written count. 4252-byte maximum; see README byte layout. */
int ce_pack(const CEProgram*,unsigned char*,size_t,size_t*);
int ce_unpack(const unsigned char*,size_t,CEProgram*);
#endif
