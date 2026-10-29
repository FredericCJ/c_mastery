#include "generic.h"
#include <stdio.h>
#ifndef AS_FORM
#define AS_FORM 0
#endif
int main(void)
{
    unsigned total = 0;
    size_t instance = 0, descriptor = 0, temporary = 0;
    for (unsigned round = 0; round < 1000; ++round)
    {
#if AS_FORM == 0
        struct as_ints b = {0};
        instance = sizeof b;
        for (int i = 1; i <= 4; ++i)
        {
            if (as_int_push(&b, i) != AS_OK)
            {
                return 1;
            }
        }
        if (as_int_push(&b, 9) != AS_FULL)
        {
            return 2;
        }
        for (size_t i = 0; i < b.count; ++i)
        {
            total += (unsigned)b.values[i];
        }
#elif AS_FORM == 1
        AsGenerated b = {0};
        instance = sizeof b;
        for (int i = 1; i <= 4; ++i)
        {
            if (AsGenerated_push(&b, i) != AS_OK)
            {
                return 1;
            }
        }
        if (AsGenerated_push(&b, 9) != AS_FULL)
        {
            return 2;
        }
        for (size_t i = 0; i < b.count; ++i)
        {
            total += (unsigned)b.values[i];
        }
#elif AS_FORM == 2
        int values[4], stage[4];
        struct as_erased b = {values,
                              0,
                              4,
                              {sizeof(int), _Alignof(int), as_int_copy_element,
                               as_int_drop_element, 0}};
        instance = sizeof b + sizeof values;
        descriptor = sizeof(struct as_elements);
        temporary = sizeof stage;
        for (int i = 1; i <= 4; ++i)
        {
            if (as_erased_append(&b, &i, 1, stage, 4, _Alignof(int)) != AS_OK)
            {
                return 1;
            }
        }
        int extra = 9;
        if (as_erased_append(&b, &extra, 1, stage, 4, _Alignof(int)) != AS_FULL)
        {
            return 2;
        }
        for (size_t i = 0; i < b.count; ++i)
        {
            total += (unsigned)values[i];
        }
        if (as_erased_clear(&b) != AS_OK)
        {
            return 3;
        }
#else
        struct as_value values[4], stage[4];
        struct as_erased b = {values,
                              0,
                              4,
                              {sizeof(struct as_value),
                               _Alignof(struct as_value), as_value_copy_element,
                               as_value_drop_element, 0}};
        instance = sizeof b + sizeof values;
        descriptor = sizeof(struct as_elements);
        temporary = sizeof stage;
        for (int i = 1; i <= 4; ++i)
        {
            struct as_value value = {.tag = AV_INT, .as.integer = i};
            if ((round + (unsigned)i) % 2 &&
                av_text((const unsigned char *)"abcd", (size_t)i, &value) !=
                    AS_OK)
            {
                return 4;
            }
            if (as_erased_append(&b, &value, 1, stage, 4,
                                 _Alignof(struct as_value)) != AS_OK)
            {
                return 1;
            }
        }
        struct as_value extra = {.tag = AV_INT, .as.integer = 9};
        if (as_erased_append(&b, &extra, 1, stage, 4,
                             _Alignof(struct as_value)) != AS_FULL)
        {
            return 2;
        }
        for (size_t i = 0; i < b.count; ++i)
        {
            total += values[i].tag == AV_INT
                         ? (unsigned)values[i].as.integer
                         : (unsigned)values[i].as.text.length;
        }
        if (as_erased_clear(&b) != AS_OK)
        {
            return 3;
        }
#endif
    }
    if (total != 10000)
    {
        return 5;
    }
    printf(
        "GENERIC-COST:form=%d;total=%u;instance=%zu;descriptor_in_instance=%zu;temporary=%zu;rounds=1000;allocations=0\n",
        AS_FORM, total, instance, descriptor, temporary);
    return 0;
}
