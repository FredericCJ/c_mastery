#include "http.h"
#include "flow.h"
#include <string.h>
#ifndef NH_MUTANT
#define NH_MUTANT 0
#endif
static int fail(struct nh_http *s, uint64_t now)
{
    s->state = NH_FAILED;
    (void)nb_stop(s->net, now);
    return NH_ABORT;
}

int nh_init(struct nh_http *s, struct nb_net *n, const char *authority,
            nh_date_fn date, void *context, uint64_t now)
{
    if (!s || !n || !authority || n->phase != NB_RUNNING || now < n->now ||
        now > UINT64_MAX - 240000 ||
        (n->tcp.state != NC_LISTEN && n->tcp.state != NC_SYN_RECEIVED &&
         n->tcp.state != NC_ESTABLISHED))
    {
        return NH_INVALID;
    }
    memset(s, 0, sizeof *s);
    s->net = n;
    s->epoch = n->tcp.epoch;
    s->date = date;
    s->date_context = context;
    s->now = now;
    http_init(&s->parser, authority);
    return NH_OK;
}

static int output(struct nh_http *s, unsigned interim, uint64_t now)
{
    s->offered = 0;
    s->output_started = now;
    if (interim)
    {
        static const char text[] = "HTTP/1.1 100 Continue\r\n\r\n";
        memcpy(s->response, text, sizeof text - 1);
        s->length = sizeof text - 1;
        s->state = NH_INTERIM;
    }
    else
    {
        const char *date = s->date ? s->date(s->date_context) : 0;
        if (NH_MUTANT == 2)
        {
            date = "Sat, 10 Oct 2026 00:00:00 GMT";
        }
        if (hf_response(&s->parser.request, date, 1, 0, 0, s->response,
                        sizeof s->response, &s->length))
        {
            return fail(s, now);
        }
        s->requests++;
        s->state = NH_FINAL;
    }
    return NH_WAIT;
}

int nh_step(struct nh_http *s, uint64_t now)
{
    if (!s || !s->net || now < s->now || now > UINT64_MAX - 240000)
    {
        return NH_INVALID;
    }
    if (s->state == NH_FAILED)
    {
        return NH_ABORT;
    }
    if (s->state == NH_FINISHED)
    {
        return NH_DONE;
    }
    s->now = now;
    if (s->epoch != s->net->tcp.epoch)
    {
        s->state = NH_FAILED;
        return NH_ABORT;
    }
    int e = nb_step(s->net, now);
    if (e == NB_FAULT || e == NB_STOPPED || s->net->phase != NB_RUNNING)
    {
        return fail(s, now);
    }
    struct nc_stack *t = &s->net->tcp;
    if (s->state == NH_CLOSING &&
        (t->state == NC_CLOSED || t->state == NC_TIME_WAIT))
    {
        s->state = NH_FINISHED;
        return NH_DONE;
    }
    if (t->state == NC_CLOSED)
    {
        return fail(s, now);
    }
    if (s->state == NH_HANDSHAKE)
    {
        if (t->state != NC_ESTABLISHED && t->state != NC_CLOSE_WAIT)
        {
            return NH_WAIT;
        }
        s->started = s->progress = now;
        s->state = NH_PARSE;
    }
    if (s->state == NH_PARSE)
    {
        if (now - s->started >= 15000 || now - s->progress >= 5000)
        {
            s->parser.request.status = 408;
            s->parser.state = HTTP_FAILED;
            return output(s, 0, now);
        }
        size_t used = 0;
        int event = HTTP_NEED;
        if (nb_http_pump(s->net, &s->parser, 64, &used, &event) == NB_BUSY)
        {
            return NH_WAIT;
        }
        if (used)
        {
            s->progress = now;
        }
        if (event == HTTP_NEED && t->peer_fin && !t->rx.count)
        {
            event = nc_http_eof(t, &s->parser);
        }
        if (NH_MUTANT == 1 && t->peer_fin && event == HTTP_ERROR)
        {
            s->parser.request.status = 200;
            event = HTTP_DONE;
        }
        if (event == HTTP_CONTINUE)
        {
            return output(s, 1, now);
        }
        if (event == HTTP_DONE || event == HTTP_ERROR)
        {
            return output(s, 0, now);
        }
        return NH_WAIT;
    }
    if (now - s->output_started >= 15000)
    {
        return fail(s, now);
    }
    if (s->state == NH_CLOSING)
    {
        return NH_WAIT;
    }
    if (!s->offered)
    {
        if (nb_offer(s->net, s->response, s->length) != NB_OK)
        {
            return NH_WAIT;
        }
        s->offered = 1;
        if (s->state == NH_FINAL)
        {
            if (nb_close(s->net) != NB_OK)
            {
                return fail(s, now);
            }
            s->state = NH_CLOSING;
        }
        return NH_WAIT;
    }
    /* Interim output must be accepted and acknowledged before resuming parser.
     * DMA itself owns the lower copy independently; no application release. */
    if (s->state == NH_INTERIM && !t->flight && t->tx_at == t->tx_length &&
        !s->net->pending_tcp)
    {
        s->state = NH_PARSE;
        s->offered = 0;
    }
    return NH_WAIT;
}
