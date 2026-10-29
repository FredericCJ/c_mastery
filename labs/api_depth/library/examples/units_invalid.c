#include <lc.h>

int main(void)
{
    struct lc_bytes b = {4}, out = {0};
    return lc_extent(b, 2, b, &out);
}
