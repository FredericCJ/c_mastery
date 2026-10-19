#ifndef LS_SCHEMA_H
#define LS_SCHEMA_H
#include <stddef.h>
#include <stdint.h>
typedef struct {uint32_t tag,length,flags;} LsFields;
int ls_pack(const LsFields *,uint16_t *);
int ls_unpack(uint16_t,LsFields *);
int ls_require(const char *wire,const char *abi);
const char *ls_wire_identity(void);
const char *ls_abi_identity(void);
const char *ls_build_identity(void);
size_t ls_record_size(void);
#endif
