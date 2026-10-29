#include "stream.h"
#ifndef NW_STREAM_MUTANT
#define NW_STREAM_MUTANT 0
#endif
int nw_http_pump(struct nw_rx *s, struct http_parser *p, size_t budget,
                 size_t *consumed)
{
    if (!s || !p || !consumed || !budget)
    {
        return HTTP_ERROR;
    }
    *consumed = 0;
    if (p->state == HTTP_FINISHED)
    {
        return HTTP_DONE;
    }
    if (p->state == HTTP_FAILED)
    {
        return HTTP_ERROR;
    }
    while (*consumed < budget)
    {
        const unsigned char *bytes;
        size_t n, used = 0;
        if (nw_rx_peek(s, &bytes, &n) != NW_OK)
        {
            return HTTP_ERROR;
        }
        if (!n)
        {
            return HTTP_NEED;
        }
        if (n > budget - *consumed)
        {
            n = budget - *consumed;
        }
        int rc = http_feed(p, bytes, n, &used);
        if (used > n)
        {
            return HTTP_ERROR;
        }
        size_t take = NW_STREAM_MUTANT == 1 ? n : used;
        if (nw_rx_consume(s, take) != NW_OK)
        {
            return HTTP_ERROR;
        }
        *consumed += take;
        if (rc != HTTP_NEED || !used)
        {
            return rc;
        }
    }
    return HTTP_NEED;
}
