#ifndef SERIAL_VARIANTS_H
#define SERIAL_VARIANTS_H
#include <stdint.h>
#include <stddef.h>
#include "can.h"
enum {SV_OK,SV_INVALID,SV_IO,SV_STALE,SV_MORE,SV_FRAME,SV_UNIMPLEMENTED};
typedef struct {uint32_t integer,fraction,units;} SvBaud;
int sv_baud(uint32_t clock,uint32_t baud,uint32_t maximum_ppm,SvBaud *out);
typedef struct {uint8_t wire[13],clock[18],sample[18];unsigned length,edges,payload;} SvWord;
int sv_word(unsigned m,unsigned parity,unsigned stop,unsigned cpol,unsigned cpha,unsigned lbcl,uint16_t value,SvWord *out);
typedef struct {unsigned level,sample;uint32_t setup_ns,hold_ns;} SvEdge;
typedef struct {unsigned index,defects;} SvCapture;
int sv_capture(unsigned cpol,unsigned cpha,uint32_t period_ns,const SvEdge *,size_t,SvCapture *);
enum {SV_LOW,SV_HIGH,SV_UNKNOWN,SV_HIGHZ};
int sv_receiver(int32_t a_mv,int32_t b_mv,unsigned enabled,unsigned open,unsigned characterized,int low_mv,int high_mv,unsigned previous,unsigned *out);
typedef struct {CnStream inner;uint32_t generation;} SvStream;
int sv_stream_reset(SvStream *,uint32_t generation);
int sv_stream_feed(SvStream *,uint32_t generation,const uint8_t *,size_t,size_t *,CnDecoded *);
enum {SV_ACCEPT,SV_PROTOCOL_EXCEPTION,SV_FORM_ERROR};
typedef struct {unsigned action,overload;} SvPolicy;
/* Valid preceding CRC/stuffing, six EOF bits and enabled error signalling are
   premises. This is a selected policy decision, not a full receiver. */
int sv_policy(unsigned extended,unsigned transmitter,unsigned srr,unsigned r1,unsigned r0,unsigned last_eof,SvPolicy *);
typedef struct {uint32_t first,second;unsigned list;} SvBank;
int sv_bank(unsigned list,const SDCan *first,const SDCan *second,uint32_t id_mask,SvBank *);
int sv_filter(const SvBank *,const SDCan *,unsigned *);
int sv_test(unsigned);
#endif
