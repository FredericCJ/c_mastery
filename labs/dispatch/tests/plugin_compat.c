#include "dispatch_plugin.h"
#include <stdio.h>
#include <string.h>
static unsigned constructors;

void ds_constructor_note(void)
{
    ++constructors;
}

#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "DS_CHECK:plugin_compat:%d:%s\n", __LINE__, #x); \
            return 1;                                                        \
        }                                                                    \
    } while (0)

int main(int argc, char **argv)
{
    if (argc != 12)
    {
        return 2;
    }
    unsigned known = ds_client_features();
    for (unsigned variant = 0; variant < 10; ++variant)
    {
        struct ds_loaded module = {0};
        unsigned before = constructors;
        int rc = ds_loaded_open(&module, argv[variant + 2]);
        CHECK(constructors == before + 1);
        if (variant >= 7)
        {
            CHECK(rc == DS_INVALID && module.handle == NULL &&
                  module.api == NULL);
            printf("P %u %u %d 0\n", known, variant, rc);
            continue;
        }
        CHECK(rc == DS_OK);
        struct ds_feature feature = {0};
        rc = ds_feature_acquire(&module, known, &feature);
        int expected = variant == 3 || (variant == 2 && !known) ? DS_UNSUPPORTED
                       : variant == 4 || (variant == 6 && known) ? DS_INVALID
                                                                 : DS_OK;
        CHECK(rc == expected);
        printf("P %u %u %d %u\n", known, variant, rc, feature.features);
        if (rc != DS_OK)
        {
            CHECK(!feature.active && module.borrows == 0);
            CHECK(ds_loaded_close(&module) == DS_OK);
            continue;
        }
        CHECK(feature.owner == &module && feature.active &&
              module.borrows == 1);
        CHECK(ds_loaded_close(&module) == DS_BUSY);
        int out = 91;
        CHECK(ds_feature_apply(&feature, 17, 3, &out) == DS_OK && out == 20);
        out = 91;
        CHECK(ds_feature_apply(&feature, -1, 3, &out) == DS_RANGE && out == 91);
        unsigned char bytes[4] = {91, 92, 93, 94};
        size_t n = 97;
        rc = ds_feature_describe(&feature, bytes, 4, &n);
        if (known && (variant == 1 || variant == 2 || variant == 5))
        {
            CHECK(rc == DS_OK && n == 2 && bytes[0] == 'v' && bytes[1] == '2' &&
                  bytes[2] == 93);
            n = 97;
            bytes[0] = 91;
            CHECK(ds_feature_describe(&feature, bytes, 1, &n) == DS_FULL &&
                  n == 97 && bytes[0] == 91);
        }
        else
        {
            CHECK(rc == DS_UNSUPPORTED && n == 97 && bytes[0] == 91);
        }
        struct ds_feature more[3] = {{0}}, overflow = {0};
        for (unsigned i = 0; i < 3; ++i)
        {
            CHECK(ds_feature_acquire(&module, known, &more[i]) == DS_OK);
        }
        CHECK(ds_feature_acquire(&module, known, &overflow) == DS_FULL &&
              !overflow.active && module.borrows == 4);
        for (unsigned i = 0; i < 3; ++i)
        {
            CHECK(ds_feature_release(&more[i]) == DS_OK);
        }
        CHECK(ds_feature_release(&feature) == DS_OK && !feature.active &&
              module.borrows == 0);
        CHECK(ds_feature_release(&feature) == DS_INVALID);
        CHECK(ds_loaded_close(&module) == DS_OK);
    }
    printf(
        "client features=%u constructors=%u; all10 original descriptor variants checked\n",
        known, constructors);
    return 0;
}
