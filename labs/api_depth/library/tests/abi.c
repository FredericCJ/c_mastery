#include "lc.h"
#include "test.h"

int main(void)
{
    unsigned char info[24];
    for (unsigned i = 0; i < 24; i++)
        info[i] = 99;
    CHECK(lc_info(9, info, 16) == LC_VERSION && info[0] == 99);
    CHECK(lc_info(1, info, 15) == LC_VERSION && info[0] == 99);
    CHECK(lc_info(1, info, 16) == LC_OK);
    CHECK(info[4] == sizeof(void *) && info[5] == sizeof(size_t) &&
          info[6] == _Alignof(size_t));
    CHECK(lc_info(1, info, 24) == (lc_release() == 1 ? LC_VERSION : LC_OK));
    CHECK(info[23] == 99);
    struct lc_buffer *p = lc_create(9);
    CHECK((p != NULL) == (lc_release() == 2));
    lc_release_buffer(p);
    printf("LC_ABI release=%u pointer=%zu size_t=%zu alignment=%zu\n",
           lc_release(), sizeof(void *), sizeof(size_t), _Alignof(size_t));
    return 0;
}
