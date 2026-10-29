#ifndef NV_VARIANT_H
#define NV_VARIANT_H
#include <stddef.h>
#include <stdint.h>
enum { NV_OK=0,NV_INVALID=1,NV_FULL=2,NV_STALE=3,NV_BUSY=4,NV_UNIMPLEMENTED=99 };
/* Uncomplemented, folded, network-order numeric sum and length modulo two.
   The empty summary is {0,0}; output may alias either summary input. */
struct nv_sum { uint16_t sum; unsigned odd; };
int nv_sum(const unsigned char *,size_t,struct nv_sum *);
int nv_combine(struct nv_sum,struct nv_sum,struct nv_sum *);
/* Complete decimal syntax, then declared wire maximum, then storage cap.
   Returns 400 for malformed/out-of-wire-domain, 413 for above cap<=1024. */
int nv_length(const unsigned char *,size_t,uint32_t maximum,uint32_t cap,uint32_t *);
int nv_date(unsigned available,uint32_t days,uint32_t seconds,char *,size_t,size_t *);
enum { NV_ORDINARY_UTC=0,NV_POSITIVE_LEAP=1,NV_TAI=2,NV_SMEARED=3 };
struct nv_stamp { unsigned available,trusted,scale; uint32_t days,seconds; };
/* attested_leap_day is externally trusted UTC calendar authority, not a
   prediction from the Gregorian calendar. UINT32_MAX means no attestation. */
int nv_stamp(struct nv_stamp,uint32_t attested_leap_day,char *,size_t,size_t *);
#endif
