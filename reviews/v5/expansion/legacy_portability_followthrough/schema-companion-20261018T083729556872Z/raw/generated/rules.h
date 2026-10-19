#ifndef LS_GENERATED_RULES_H
#define LS_GENERATED_RULES_H
#ifndef LS_RAW
#define LS_RAW 0
#endif
#if LS_RAW != 0 && LS_RAW != 1
#error LS_BOOLEAN_raw
#endif
#ifndef LS_FRAMED
#define LS_FRAMED 0
#endif
#if LS_FRAMED != 0 && LS_FRAMED != 1
#error LS_BOOLEAN_framed
#endif
#ifndef LS_CRC
#define LS_CRC 0
#endif
#if LS_CRC != 0 && LS_CRC != 1
#error LS_BOOLEAN_crc
#endif
#ifndef LS_DATAGRAM
#define LS_DATAGRAM 0
#endif
#if LS_DATAGRAM != 0 && LS_DATAGRAM != 1
#error LS_BOOLEAN_datagram
#endif
#if LS_CRC && !LS_FRAMED
#error LS_REQUIRES_crc_framed
#endif
#if LS_RAW + LS_FRAMED + LS_DATAGRAM > 1
#error LS_EXCLUSIVE_TRANSPORT
#endif
#endif
