/*
 * Consumer of the independently built plugin descriptor (plugin.c). Usage:
 * plugin_client VERSION, with VERSION a decimal variant number 1..4; exit
 * status 2 for a usage error, 1 for a failed check, 0 for success.
 *
 * The consumer treats the plugin as untrusted foreign code: it asks for the
 * required size first, copies only into a buffer of exactly that size, and
 * then lets fb_negotiate decide from the declared size without reading any
 * tail the descriptor does not provide (run under AddressSanitizer).
 */
#include "bits.h"
#include "cm_parse.h"
#include "plugin.h"
#include <stdio.h>
#include <stdlib.h>

#define PLUGIN_VARIANT_MIN 1u
#define PLUGIN_VARIANT_MAX 4u

/* Sentinel values proving fb_negotiate left its output untouched. */
#define SENTINEL_VERSION 91u
#define SENTINEL_SIZE 92u
#define SENTINEL_FEATURES 93u
#define SENTINEL_MAX_ITEMS 94u

/* Release the copy buffer before reporting a failed check. */
#define T(x)                                                         \
    do                                                               \
    {                                                                \
        if (!(x))                                                    \
        {                                                            \
            fprintf(stderr, "FB_PLUGIN_FAIL %d %s\n", __LINE__, #x); \
            free(bytes);                                             \
            return 1;                                                \
        }                                                            \
    } while (0)

/* Octets the variant's descriptor provides. Variant 4 declares 12 but
 * provides 8; variant 3 is header only. */
static size_t provided_octets(unsigned version)
{
    if ((version == 1u) || (version == 4u))
    {
        return 8u;
    }
    if (version == 2u)
    {
        return 12u;
    }
    return 4u;
}

/* The status fb_negotiate must return for each variant: 1 and 2 are valid,
 * 3 is an unknown version, 4 declares more octets than are available. */
static enum fb_status expected_status(unsigned version)
{
    if (version <= 2u)
    {
        return FB_OK;
    }
    return (version == 3u) ? FB_UNSUPPORTED : FB_SHORT;
}

int main(int argc, char **argv)
{
    unsigned version = 0;
    if ((argc != 2) ||
        (cm_parse_unsigned(argv[1], PLUGIN_VARIANT_MIN, PLUGIN_VARIANT_MAX,
                           &version) != CM_PARSE_OK))
    {
        return 2;
    }

    unsigned char *bytes = NULL; /* owned here; freed by T() or at the end */
    size_t required = fb_plugin_copy(NULL, 0);
    T(required == provided_octets(version));

    /* Too small a buffer must receive nothing, but the required size is still
     * reported. */
    unsigned char short_out[3] = {91, 92, 93};
    T(fb_plugin_copy(short_out, sizeof short_out) == required &&
      short_out[0] == 91 && short_out[2] == 93);

    bytes = malloc(required);
    T(bytes != NULL);
    T(fb_plugin_copy(bytes, required) == required);

    struct fb_plugin out = {SENTINEL_VERSION, SENTINEL_SIZE, SENTINEL_FEATURES,
                            SENTINEL_MAX_ITEMS};
    const struct fb_span descriptor = {bytes, required};
    enum fb_status status = fb_negotiate(descriptor, &out);
    T(status == expected_status(version));
    if (version <= 2u)
    {
        T(out.version == version && out.size == required && out.features == 1 &&
          out.max_items == ((version == 1u) ? 0u : 1024u));
    }
    else
    {
        T(out.version == SENTINEL_VERSION && out.size == SENTINEL_SIZE &&
          out.features == SENTINEL_FEATURES &&
          out.max_items == SENTINEL_MAX_ITEMS);
    }
    free(bytes);
    printf(
        "FB_PLUGIN_PASS variant=%u exact_available=%zu negotiated_without_tail_read\n",
        version, required);
    return 0;
}
