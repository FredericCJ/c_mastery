#include "close_owner.h"
#include <stdio.h>

struct qc_p
{
    struct qc_raw raw;
    unsigned calls;
};

static struct qc_raw call(void *p, int fd)
{
    struct qc_p *v = p;
    (void)fd;
    v->calls++;
    return v->raw;
}

int main(void)
{
    int rc, error;
    while (scanf("%d %d", &rc, &error) == 2)
    {
        struct qc_owner o;
        struct qc_observation v;
        struct qc_p p = {{rc, error}, 0};
        int fd = 99;
        if (qc_init(&o, 7) != QC_OK)
            return 1;
        enum qc_status first = qc_once(&o, call, &p, &v);
        enum qc_status second = qc_handoff(&o, &fd);
        printf("%d %d %d %u %d %d\n", first, v.state, v.error, p.calls, second,
               fd);
    }
    return 0;
}
