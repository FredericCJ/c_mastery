#ifndef CMASTERY_DEBUG_CONNECTION_H
#define CMASTERY_DEBUG_CONNECTION_H
#include <stdint.h>
enum dc_status {DC_OK,DC_INVALID,DC_UNKNOWN,DC_ORDER,DC_MISMATCH,DC_UNIMPLEMENTED};
enum {DC_DISCONNECTED,DC_IDENTIFIED,DC_HALTED,DC_RESET_TESTED};
enum {DC_CONNECTION,DC_FLASH,DC_RESET_APPLICATION,DC_SUCCESS};
enum {DC_BOARD=1,DC_PROBE=2,DC_FIRMWARE=4,DC_SERVER=8,DC_CONFIG=16,DC_TARGET=32,DC_RESET=64,DC_POWER=128,DC_FLASH_DRIVER=256,DC_TRANSPORT=512};
/* Nonzero IDs name immutable caller-owned evidence records. They are identity
 * handles, not hashes, authentication, actual wiring or a silicon diagnosis. */
typedef struct {unsigned known,board,probe,firmware,server,config,target,reset,power,flash_driver,transport;uint64_t latency_ns;} DCManifest;
typedef struct {DCManifest manifest;unsigned phase,observed_target;} DCSession;
enum dc_status dc_init(DCSession *,const DCManifest *);
enum dc_status dc_identify(DCSession *,unsigned actual_target);
enum dc_status dc_halt(DCSession *,unsigned observed_halted);
enum dc_status dc_reset(DCSession *,unsigned observed_reset_and_halted);
enum dc_status dc_program_authorized(const DCSession *);
/* Any changed environment identity revokes all previous session evidence. */
enum dc_status dc_reconfigure(DCSession *,const DCManifest *);
enum dc_status dc_classify(unsigned connected,unsigned programmed,unsigned reset_ok,unsigned *stage);
enum dc_status dc_vector(uint32_t base,uint32_t bytes,uint32_t vector,unsigned *valid);
#endif
