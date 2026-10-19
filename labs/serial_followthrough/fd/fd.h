#ifndef CURRENT_FD_DEPTH_H
#define CURRENT_FD_DEPTH_H
#include "serial_depth.h"
enum {FD_OK,FD_INVALID,FD_LENGTH,FD_CAPABILITY,FD_CAPACITY,FD_UNIMPLEMENTED};
typedef struct {unsigned dlc,length,fdf,brs,esi,remote,capable;} FdInput;
typedef struct {SDFd metadata;unsigned payload_length,padding;uint8_t payload[64];} FdPacket;
/* Caller storage is live/disjoint. Validate raw flags before conversion. */
int fd_admit(const FdInput *,const uint8_t *,size_t,FdPacket *);
int fd_length(unsigned,const uint8_t *,size_t,unsigned,unsigned,FdPacket *);
int fd_esi(unsigned passive,unsigned requested,unsigned *wire_bit);
typedef struct {uint8_t bits[720];unsigned count,dynamic_stuff,crc,width,crc_start;} FdWire;
/* Strict synthetic 11-bit DATA transmitter subset; current=0 is the explicitly
   archived Bosch17April2012 experiment, not an unspecified non-ISO standard. */
int fd_wire(unsigned identifier,const FdPacket *,unsigned current,FdWire *);
typedef struct {unsigned command_bytes,payload_bytes,total_bytes,committed_bytes,ignored_bytes;} FdTransaction;
enum {FD_TCAN4550,FD_MCP2518FD};
int fd_transaction(unsigned controller,unsigned ram,unsigned address,unsigned payload_bytes,unsigned completed_bytes,FdTransaction *);
int fd_header(unsigned controller,unsigned write,unsigned address,unsigned words,uint8_t *,size_t,size_t *);
int fd_power(unsigned controller,unsigned supply_mv,unsigned logic_mv,unsigned transceiver_mv,unsigned transceiver_logic_mv);
typedef struct {unsigned checked,skipped,mismatches,first_reaction;} FdTdc;
/* Discrete minimum-quantum trace, selected prescaler1. The measured delay is
   an input observation, not inferred from a datasheet headline or this model. */
int fd_tdc(const uint8_t *transmitted,size_t count,const uint8_t *received,size_t samples,
           unsigned bit_ticks,unsigned sample_ticks,unsigned measured_delay,unsigned offset,FdTdc *);
int fd_test(unsigned);
#endif
