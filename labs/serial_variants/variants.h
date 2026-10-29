#ifndef SERIAL_VARIANTS_H
#define SERIAL_VARIANTS_H
#include <stdint.h>
#include <stddef.h>
#include "can.h"
enum {SV_OK,SV_INVALID,SV_IO,SV_STALE,SV_MORE,SV_FRAME,SV_UNIMPLEMENTED};
struct sv_baud {uint32_t integer,fraction,units;};
int sv_baud(uint32_t clock,uint32_t baud,uint32_t maximum_ppm,struct sv_baud *out);
struct sv_word {uint8_t wire[13],clock[18],sample[18];unsigned length,edges,payload;};
int sv_word(unsigned m,unsigned parity,unsigned stop,unsigned cpol,unsigned cpha,unsigned lbcl,uint16_t value,struct sv_word *out);
struct sv_edge {unsigned level,sample;uint32_t setup_ns,hold_ns;};
struct sv_capture {unsigned index,defects;};
int sv_capture(unsigned cpol,unsigned cpha,uint32_t period_ns,const struct sv_edge *,size_t,struct sv_capture *);
enum {SV_LOW,SV_HIGH,SV_UNKNOWN,SV_HIGHZ};
int sv_receiver(int32_t a_mv,int32_t b_mv,unsigned enabled,unsigned open,unsigned characterized,int low_mv,int high_mv,unsigned previous,unsigned *out);
struct sv_stream {struct cn_stream inner;uint32_t generation;};
int sv_stream_reset(struct sv_stream *,uint32_t generation);
int sv_stream_feed(struct sv_stream *,uint32_t generation,const uint8_t *,size_t,size_t *,struct cn_decoded *);
enum {SV_ACCEPT,SV_PROTOCOL_EXCEPTION,SV_FORM_ERROR};
struct sv_policy {unsigned action,overload;};
/* Valid preceding CRC/stuffing, six EOF bits and enabled error signalling are
   premises. This is a selected policy decision, not a full receiver. */
int sv_policy(unsigned extended,unsigned transmitter,unsigned srr,unsigned r1,unsigned r0,unsigned last_eof,struct sv_policy *);
struct sv_bank {uint32_t first,second;unsigned list;};
int sv_bank(unsigned list,const struct sd_can *first,const struct sd_can *second,uint32_t id_mask,struct sv_bank *);
int sv_filter(const struct sv_bank *,const struct sd_can *,unsigned *);
int sv_test(unsigned);
#endif
