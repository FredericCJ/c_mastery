#ifndef SF_HEX_H
#define SF_HEX_H
#include <stddef.h>
#include <stdint.h>
enum SfMode { SF_PLAIN, SF_SEGMENT, SF_LINEAR };
enum SfStatus { SF_OK, SF_INVALID, SF_FORMAT, SF_CHECKSUM, SF_UNSUPPORTED, SF_CAPACITY, SF_DUPLICATE, SF_RANGE };
struct SfByte { uint32_t address; unsigned char value; };
struct SfImage { struct SfByte bytes[64]; size_t count; unsigned entry_kind; uint32_t entry; };
enum SfStatus sf_hex(const char *text,size_t length,enum SfMode mode,struct SfImage *out);
enum SfStatus sf_window(const struct SfImage *image,uint64_t begin,uint64_t end);
#endif
