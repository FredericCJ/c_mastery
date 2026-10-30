/* CM-SPECIMEN: plugin descriptor whose declared size exceeds the octets it
 * provides (FB_PLUGIN_VERSION 4).
 *
 * Independent producer of the plugin descriptor, built as a shared object by
 * a separate compiler invocation so the consumer sees only the wire format and
 * the fb_plugin_copy entry point. FB_PLUGIN_VERSION selects the teaching
 * variant; each descriptor is big-endian: octets 0..1 version, 2..3 total
 * size, 4..7 feature bits, and for version 2 octets 8..11 maximum items. */
#include "plugin.h"
#ifndef FB_PLUGIN_VERSION
#define FB_PLUGIN_VERSION 1
#endif
#if FB_PLUGIN_VERSION == 1
static const unsigned char descriptor[] = {0, 1, 0, 8, 0, 0, 0, 1};
#elif FB_PLUGIN_VERSION == 2
/* max_items = 0x0400 = 1024 */
static const unsigned char descriptor[] = {0, 2, 0, 12, 0, 0, 0, 1, 0, 0, 4, 0};
#elif FB_PLUGIN_VERSION == 3
/* A version the consumer does not know; header only. */
static const unsigned char descriptor[] = {0, 3, 0, 4};
#elif FB_PLUGIN_VERSION == 4
/* Deliberate defect: declares 12 octets but provides only 8, so a consumer
 * that trusts the declared size would read past the object. */
static const unsigned char descriptor[] = {0, 2, 0, 12, 0, 0, 0, 1};
#else
#error unsupported_teaching_plugin_variant
#endif

/* See plugin.h for the contract. */
size_t fb_plugin_copy(unsigned char *destination, size_t capacity)
{
    if ((destination != NULL) && (capacity >= sizeof descriptor))
    {
        for (size_t i = 0; i < sizeof descriptor; ++i)
        {
            destination[i] = descriptor[i];
        }
    }
    return sizeof descriptor;
}
