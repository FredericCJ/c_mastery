#ifndef FB_PLUGIN_H
#define FB_PLUGIN_H
#include <stddef.h>
/* Hosted ELF/SysV C ABI entry. Returns required size. Copy the entire descriptor
 * only if destination nonnull and capacity sufficient; otherwise writes nothing.
 * Descriptor octets have their own stable version/size contract. */
size_t fb_plugin_copy(unsigned char *destination,size_t capacity);
#endif
