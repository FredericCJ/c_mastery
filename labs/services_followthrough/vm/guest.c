#include "abi.h"
#include "wire.h"
static volatile unsigned image_word;
static struct sfv_service service;

static void done(unsigned n)
{
    cmk_call(CMK_EXIT, n, 0, 0);
    for (;;)
    {
    }
}

static void check(int ok)
{
    if (!ok)
    {
        cmk_write("SF_VM_CHECK\n", 12);
        done(91);
    }
}

static int64_t receive(uint64_t q, void *p, size_t n, uint64_t *sender)
{
    struct sf_receive r = {(uintptr_t)p, n, 0};
    int64_t s;
    do
    {
        s = cmk_call(SF_RECEIVE, q, (uintptr_t)&r, 0);
    } while (s == SF_RETRY);
    if (sender)
    {
        *sender = r.sender;
    }
    return s;
}

static int64_t send(uint64_t q, const void *p, size_t n)
{
    struct sf_span s = {(uintptr_t)p, n};
    return cmk_call(SF_SEND, q, (uintptr_t)&s, 0);
}

void _start(void)
{
    image_word = 1;
    check(image_word == 1);
    struct sf_bootstrap b;
    while (cmk_call(SF_BOOTSTRAP, (uintptr_t)&b, 0, 0) == SF_FULL)
    {
        cmk_call(CMK_YIELD, 0, 0, 0);
    }
    struct sfv_config config;
    uint64_t sender = 0;
    check(receive(b.request, &config, sizeof config, &sender) ==
              sizeof config &&
          sender == SF_PROCESS_TAG + 65537);
    check(!sfv_open(&service, 0, b.session) &&
          !sfv_open(&service, 1, b.session));
    unsigned abandoned = 0;
    for (;;)
    {
        struct sfv_wire w;
        int64_t size = receive(b.request, &w, sizeof w, &sender);
        if (size == SF_CLOSED)
        {
            break;
        }
        check(size == sizeof w);
        unsigned client = sender == config.clients[0]
                              ? 0
                              : (sender == config.clients[1] ? 1 : 2);
        check(client < 2);
        int r;
        if (w.operation == 1)
        {
            r = sfv_cancel(&service, client, w.request.session, w.request.id);
        }
        else
        {
            r = sfv_submit(&service, client, &w.request);
            for (unsigned i = 0; i < 100001 && r == SFV_RUNNING; i++)
            {
                r = sfv_step(&service, client);
            }
        }
        struct sfv_response reply = {
            w.request.session, w.request.id, (unsigned)r, 0, {0}};
        size_t length = 0;
        unsigned status = 0;
        if (!r || r == CVM_FUEL || r == SFV_CAPACITY)
        {
            int e = sfv_reply(&service, client, w.request.session, w.request.id,
                              reply.bytes, w.capacity, &length, &status);
            reply.status = e ? (unsigned)e : status;
            reply.length = (uint32_t)length;
        }
        int64_t sent = send(config.reply[client], &reply, sizeof reply);
        if (sent == SF_CLOSED || sent == SF_DENIED)
        {
            (void)sfv_cancel(&service, client, w.request.session, w.request.id);
            abandoned++;
            if (sent == SF_CLOSED)
            {
                check(
                    !cmk_call(SF_CHANNEL_DESTROY, config.reply[client], 0, 0));
            }
        }
        else
        {
            check(!sent);
        }
    }
#if SF_CASE == 20
    check(abandoned == 1);
    cmk_write("PASS sf-vm-client-gone\n", 23);
#else
    check(!abandoned);
#endif
    done(0);
}
