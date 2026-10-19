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
#define LS_DATAGRAM 1
#elif LS_DATAGRAM != 1
#error LS_CONFIGURATION_IDENTITY_datagram
#endif
#include "rules.h"
#define LS_WIRE_ID "e64fd83df690f9292d291e2146398d8bfd2e5ea15581f30959ce138b222b627a"
#define LS_ABI_ID "6f1cc143cb3274507f01c06984ac3c64a4e3b2d3813bdd02f44b8fe63ff908ae"
#define LS_BUILD_ID "e9c4c92ef5f4d4b2643a90b4d702cba65d2ce2ad499303b90d1cf83d2dc0bdb2"
typedef struct {uint32_t value;
#if LS_FRAMED
uint32_t sequence;
#endif
#if LS_CRC
uint32_t checksum;
#endif
} LsApiRecord;
#endif
