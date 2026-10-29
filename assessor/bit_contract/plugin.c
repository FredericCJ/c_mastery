#include "plugin.h"
#ifndef FB_PLUGIN_VERSION
#define FB_PLUGIN_VERSION 1
#endif
#if FB_PLUGIN_VERSION == 1
static const unsigned char descriptor[] = {0, 1, 0, 8, 0, 0, 0, 1};
#elif FB_PLUGIN_VERSION == 2
static const unsigned char descriptor[] = {0, 2, 0, 12, 0, 0, 0, 1, 0, 0, 4, 0};
#elif FB_PLUGIN_VERSION == 3
static const unsigned char descriptor[] = {0, 3, 0, 4};
#elif FB_PLUGIN_VERSION == 4
static const unsigned char descriptor[] = {
    0, 2, 0, 12, 0, 0, 0, 1}; /* lies about available tail */
#else
#error unsupported_teaching_plugin_variant
#endif
size_t fb_plugin_copy(unsigned char *out, size_t capacity)
{
    if (out && capacity >= sizeof descriptor)
        for (size_t i = 0; i < sizeof descriptor; ++i)
            out[i] = descriptor[i];
    return sizeof descriptor;
}
