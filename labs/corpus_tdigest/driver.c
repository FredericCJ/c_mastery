#include "tdigest.h"
#include <stdio.h>

int main(void)
{
    unsigned leaf, target, batch, parts, mode, n;
    static const double queries[] = {0,   .001, .01, .1,   .25, .5,
                                     .75, .9,   .99, .999, 1};
    while (scanf("%u%u%u%u%u%u", &leaf, &target, &batch, &parts, &mode, &n) ==
           6)
    {
        double values[2048];
        struct td_td digests[8], result;
        unsigned i, j;
        if (!parts || parts > 8 || !batch || batch > 64 || n > 2048 || mode > 2)
        {
            return 2;
        }
        for (i = 0; i < n; i++)
        {
            if (scanf("%lf", &values[i]) != 1)
            {
                return 2;
            }
        }
        for (i = 0; i < parts; i++)
        {
            unsigned begin = n * i / parts, end = n * (i + 1) / parts;
            if (td_init(&digests[i], leaf))
            {
                return 3;
            }
            for (j = begin; j < end; j += batch)
            {
                unsigned length = end - j < batch ? end - j : batch;
                if (td_add(&digests[i], values + j, length))
                {
                    return 3;
                }
            }
        }
        if (mode == 2)
        {
            unsigned stride;
            for (stride = 1; stride < parts; stride *= 2)
            {
                for (i = 0; i + stride < parts; i += 2 * stride)
                {
                    if (td_merge(&digests[i], &digests[i + stride]))
                    {
                        return 3;
                    }
                }
            }
            result = digests[0];
            if (td_recompress(&result, target))
            {
                return 3;
            }
        }
        else
        {
            if (td_init(&result, target))
            {
                return 3;
            }
            for (i = 0; i < parts; i++)
            {
                unsigned ix = mode ? parts - 1 - i : i;
                if (td_merge(&result, &digests[ix]))
                {
                    return 3;
                }
            }
        }
        printf("%u %u", result.used, (unsigned)result.count);
        for (i = 0; i < result.used; i++)
        {
            printf(" %.17g %u", result.centroids[i].mean,
                   (unsigned)result.centroids[i].weight);
        }
        for (i = 0; i < sizeof queries / sizeof queries[0]; i++)
        {
            double value, cdf;
            if (td_quantile(&result, queries[i], &value) ||
                td_cdf(&result, value, &cdf))
            {
                return 3;
            }
            printf(" %.17g %.17g", value, cdf);
        }
        puts("");
    }
    return 0;
}
