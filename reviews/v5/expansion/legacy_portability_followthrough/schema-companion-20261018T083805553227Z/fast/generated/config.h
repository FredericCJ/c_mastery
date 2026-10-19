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
#define LS_CRC 1
#elif LS_CRC != 1
#error LS_CONFIGURATION_IDENTITY_crc
#endif
#ifndef LS_DATAGRAM
#define LS_DATAGRAM 0
#elif LS_DATAGRAM != 0
#error LS_CONFIGURATION_IDENTITY_datagram
#endif
#include "rules.h"
#define LS_WIRE_ID "d6593335ccbafbd66eda40d841e9cf5767c5e6568eb06c843e9ccd6fb84a80fc"
#define LS_ABI_ID "ab51d981d8e07ea556ceefad4c350a01255ed6f9a5819c9328f4b9979dd6faf4"
#define LS_BUILD_ID "7b77fdf5cd4f30e4bf6cd772b29375c7f81d3cd7a41df70cea3cea9141eff75e"
typedef struct {uint32_t value;
#if LS_FRAMED
uint32_t sequence;
#endif
#if LS_CRC
uint32_t checksum;
#endif
} LsApiRecord;
#endif
