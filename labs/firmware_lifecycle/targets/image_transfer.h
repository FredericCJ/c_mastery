#ifndef B06_IMAGE_TRANSFER_H
#define B06_IMAGE_TRANSFER_H
#include "lifecycle_extension.h"
typedef struct {unsigned char bytes[32];unsigned unit,completed;} FIImage;
int fi_init(FIImage *,unsigned unit);
/* Program one aligned indexed unit from an immutable32-byte intended image.
 * bits models a prefix of physical bit attempts; complete is controller status.
 * It is not an attacker-supplied claim. No overlapping state/input objects. */
int fi_program(FIImage *,const unsigned char *,unsigned index,unsigned bits,int complete);
int fi_commit(const FIImage *,const unsigned char *,uint32_t sequence,unsigned char record[24]);
int fi_record_matches(const unsigned char image[32],const unsigned char record[24]);
typedef struct {uint32_t epoch,sequence,limit,floor;unsigned key,revoked;} FIRotation;
typedef int (*FIPersist)(void *,const FIRotation *);
int fi_advance(FIRotation *);
/* Recovery authority is a trusted verified-key result, never image metadata.
 * Persist returns exactly1 only after durable atomic policy publication. */
int fi_rotate(FIRotation *,uint32_t epoch,unsigned new_key,uint32_t floor,
 unsigned verified_key,int verdict,FIPersist,void*);
#endif
