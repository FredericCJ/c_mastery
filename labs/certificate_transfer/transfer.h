#ifndef CMASTERY_CERTIFICATE_TRANSFER_H
#define CMASTERY_CERTIFICATE_TRANSFER_H
#include "frontier.h"
#include "tree.h"

enum CTStatus {CT_OK,CT_MORE,CT_INVALID,CT_TRUNCATED,CT_CERTIFICATE,CT_STATE};
typedef struct {unsigned char bytes[102];size_t used,expected;unsigned state;} CTStream;
void ct_init(CTStream*);
enum CTStatus ct_feed(CTStream*,const unsigned char*,size_t,unsigned final,
                      const AGCertificate*,CDCSR*);
typedef struct {int node,parent,phase,left_height,right_height,left_black,right_black;
                int64_t low,high;} CTFrame;
typedef struct {CTFrame frames[64];unsigned seen[64],high_water;} CTWorkspace;
enum {CT_SPACE=99};
int ct_tree(const ATCertificate*,unsigned policy,CTWorkspace*,unsigned capacity,int*);
int ct_tests(unsigned);
int ct_failure(const char*,unsigned);
void ct_observation(const char*,unsigned,unsigned);
#endif
