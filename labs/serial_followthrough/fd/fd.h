#ifndef CURRENT_FD_DEPTH_H
#define CURRENT_FD_DEPTH_H
#include "serial_depth.h"
enum {FD_OK,FD_INVALID,FD_LENGTH,FD_CAPABILITY,FD_CAPACITY,FD_UNIMPLEMENTED};
struct fd_input {unsigned dlc,length,fdf,brs,esi,remote,capable;};
struct fd_packet {struct sd_fd metadata;unsigned payload_length,padding;uint8_t payload[64];};
/* Caller storage is live/disjoint. Validate raw flags before conversion. */
int fd_admit(const struct fd_input *,const uint8_t *,size_t,struct fd_packet *);
int fd_length(unsigned,const uint8_t *,size_t,unsigned,unsigned,struct fd_packet *);
int fd_esi(unsigned passive,unsigned requested,unsigned *wire_bit);
struct fd_wire {uint8_t bits[720];unsigned count,dynamic_stuff,crc,width,crc_start;};
/* Strict synthetic 11-bit DATA transmitter subset; current=0 is the explicitly
   archived Bosch17April2012 experiment, not an unspecified non-ISO standard. */
int fd_wire(unsigned identifier,const struct fd_packet *,unsigned current,struct fd_wire *);
struct fd_transaction {unsigned command_bytes,payload_bytes,total_bytes,committed_bytes,ignored_bytes;};
enum {FD_TCAN4550,FD_MCP2518FD};
int fd_transaction(unsigned controller,unsigned ram,unsigned address,unsigned payload_bytes,unsigned completed_bytes,struct fd_transaction *);
int fd_header(unsigned controller,unsigned write,unsigned address,unsigned words,uint8_t *,size_t,size_t *);
int fd_power(unsigned controller,unsigned supply_mv,unsigned logic_mv,unsigned transceiver_mv,unsigned transceiver_logic_mv);
struct fd_tdc {unsigned checked,skipped,mismatches,first_reaction;};
/* Discrete minimum-quantum trace, selected prescaler1. The measured delay is
   an input observation, not inferred from a datasheet headline or this model. */
int fd_tdc(const uint8_t *transmitted,size_t count,const uint8_t *received,size_t samples,
           unsigned bit_ticks,unsigned sample_ticks,unsigned measured_delay,unsigned offset,struct fd_tdc *);
int fd_test(unsigned);
#endif
