#include "bits.h"
#include "plugin.h"
#include <stdio.h>
#include <stdlib.h>
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

int main(int argc, char **argv)
{
    if (argc != 2 || argv[1][0] < '1' || argv[1][0] > '4' || argv[1][1])
        return 2;
    unsigned version = (unsigned)(argv[1][0] - '0');
    unsigned char *bytes = NULL;
    size_t required = fb_plugin_copy(NULL, 0);
    T(required == (version == 1 || version == 4 ? 8u
                   : version == 2               ? 12u
                                                : 4u));
    unsigned char short_out[3] = {91, 92, 93};
    T(fb_plugin_copy(short_out, 3) == required && short_out[0] == 91 &&
      short_out[2] == 93);
    bytes = malloc(required);
    T(bytes != NULL);
    T(fb_plugin_copy(bytes, required) == required);
    struct fb_plugin out = {91, 92, 93, 94};
    enum fb_status s = fb_negotiate((struct fb_span){bytes, required}, &out);
    T(s == (version <= 2 ? FB_OK : version == 3 ? FB_UNSUPPORTED : FB_SHORT));
    if (version <= 2)
    {
        T(out.version == version && out.size == required && out.features == 1 &&
          out.max_items == (version == 1 ? 0u : 1024u));
    }
    else
    {
        T(out.version == 91 && out.size == 92 && out.features == 93 &&
          out.max_items == 94);
    }
    free(bytes);
    printf(
        "FB_PLUGIN_PASS variant=%u exact_available=%zu negotiated_without_tail_read\n",
        version, required);
    return 0;
}
