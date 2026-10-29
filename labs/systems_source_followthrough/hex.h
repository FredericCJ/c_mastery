#ifndef SF_HEX_H
#define SF_HEX_H
#include <stddef.h>
#include <stdint.h>
enum sf_mode { SF_PLAIN, SF_SEGMENT, SF_LINEAR };
enum sf_status { SF_OK, SF_INVALID, SF_FORMAT, SF_CHECKSUM, SF_UNSUPPORTED, SF_CAPACITY, SF_DUPLICATE, SF_RANGE };
struct sf_byte { uint32_t address; unsigned char value; };
struct sf_image { struct sf_byte bytes[64]; size_t count; unsigned entry_kind; uint32_t entry; };
enum sf_status sf_hex(const char *text,size_t length,enum sf_mode mode,struct sf_image *out);
enum sf_status sf_window(const struct sf_image *image,uint64_t begin,uint64_t end);
#endif
