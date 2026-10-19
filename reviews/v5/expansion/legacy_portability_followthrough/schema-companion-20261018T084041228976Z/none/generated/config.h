#ifndef LS_GENERATED_CONFIG_H
#define LS_GENERATED_CONFIG_H
#include <stdint.h>
#ifndef LS_RAW
#define LS_RAW 0
#elif LS_RAW != 0
#error LS_CONFIGURATION_IDENTITY_raw
#endif
#ifndef LS_FRAMED
#define LS_FRAMED 0
#elif LS_FRAMED != 0
#error LS_CONFIGURATION_IDENTITY_framed
#endif
#ifndef LS_CRC
#define LS_CRC 0
#elif LS_CRC != 0
#error LS_CONFIGURATION_IDENTITY_crc
#endif
#ifndef LS_DATAGRAM
#define LS_DATAGRAM 0
#elif LS_DATAGRAM != 0
#error LS_CONFIGURATION_IDENTITY_datagram
#endif
#include "rules.h"
#define LS_WIRE_ID "504b192bfac2794c05d257d4e5f7cbcb8a6e7b1e9f0e957af619fd278821fbaf"
#define LS_ABI_ID "6f1cc143cb3274507f01c06984ac3c64a4e3b2d3813bdd02f44b8fe63ff908ae"
#define LS_BUILD_ID "b1f72f0af2456b193794625cc76932c30680644624fe1419f0745b67686d4021"
typedef struct {uint32_t value;
#if LS_FRAMED
uint32_t sequence;
#endif
#if LS_CRC
uint32_t checksum;
#endif
} LsApiRecord;
#endif
