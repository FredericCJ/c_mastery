#ifndef LS_GENERATED_CONFIG_H
#define LS_GENERATED_CONFIG_H
#include <stdint.h>
#ifndef LS_RAW
#define LS_RAW 0
#elif LS_RAW != 0
#error LS_CONFIGURATION_IDENTITY_raw
#endif
#ifndef LS_FRAMED
#define LS_FRAMED 1
#elif LS_FRAMED != 1
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
#define LS_WIRE_ID "0cf59ac3ee690852c03c616a84bad480c8df899f7acc3c0025978f724e491c1d"
#define LS_ABI_ID "662b25e447967a772dd32ab3ff19dfc34982b63cb1e596bf5e87f92ef62aa26d"
#define LS_BUILD_ID "97f0c02316d80fd422631db41bbc9716d7fe48b7df500aaeb601c6e38eaf63ac"
typedef struct {uint32_t value;
#if LS_FRAMED
uint32_t sequence;
#endif
#if LS_CRC
uint32_t checksum;
#endif
} LsApiRecord;
#endif
