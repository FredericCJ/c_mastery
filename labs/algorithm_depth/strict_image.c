/* Linux x86-64 custom-entry closure probe; not an ISO C startup claim. */
#include "algorithms.h"

__attribute__((noreturn)) void ad_image(void)
{
    unsigned char input[] = {32, '1', '2', 32}, copy[4];
    size_t length = 0, used = 0;
    int32_t value = 0;
    long status = 0;
    if (ad_byte_copy(copy, 4, input, 4) != AD_OK ||
        ad_byte_trim_in_place(copy, 4, &length) != AD_OK || length != 2 ||
        ad_parse_i32(copy, length, 10, 0, 1, &value, &used) != AD_OK ||
        value != 12 || used != 2)
    {
        status = 1;
    }
    struct ad_record records[] = {{3, 0}, {1, 1}, {2, 2}};
    struct ad_work work = {0};
    if (ad_heap_sort(records, 3, &work) != AD_OK || records[0].key != 1 ||
        records[2].key != 3)
    {
        status = 2;
    }
    __asm__ volatile("syscall" ::"a"(60L), "D"(status)
                     : "rcx", "r11", "memory");
    __builtin_unreachable();
}
