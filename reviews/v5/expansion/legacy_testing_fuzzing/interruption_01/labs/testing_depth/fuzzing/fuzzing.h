#ifndef TESTING_FUZZING_H
#define TESTING_FUZZING_H
#include "models.h"
enum {TF_OK,TF_LENGTH,TF_OPCODE,TF_COUNT,TF_FLAGS,TF_INVALID,TF_SESSION,TF_SEQUENCE,TF_RANGE};
typedef struct {TmPacket packet;unsigned has_tag,tag;} TfPacket;
/* profile0 retains opcode1/exact2+2count/cap10. profile1 is documented extension. */
unsigned tf_parse(unsigned,const unsigned char *,size_t,TfPacket *);
typedef struct {uint32_t epoch,next,total;unsigned active;} TfSession;
void tf_session_init(TfSession *);
unsigned tf_session_begin(TfSession *,uint32_t);
unsigned tf_session_apply(TfSession *,uint32_t,uint32_t,const unsigned char *,size_t,TfPacket *);
#endif
