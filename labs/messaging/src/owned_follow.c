#include "owned.h"

void mfo_drop(struct mfo_value *v)
{
    (void)v;
}

enum msg_status mfc_init(struct mfc_hub *h, size_t a, size_t b)
{
    (void)h;
    (void)a;
    (void)b;
    return MSG_FAILED;
}

enum msg_status mfc_send(struct mfc_hub *h, unsigned c, struct mfo_value *v,
                         bool w, mfc_after_submit_fn f, void *x)
{
    (void)h;
    (void)c;
    (void)v;
    (void)w;
    (void)f;
    (void)x;
    return MSG_FAILED;
}

enum msg_status mfc_select(struct mfc_hub *h, unsigned m, bool w, unsigned *c,
                           struct mfo_value *v)
{
    (void)h;
    (void)m;
    (void)w;
    (void)c;
    (void)v;
    return MSG_FAILED;
}

enum msg_status mfc_close(struct mfc_hub *h, unsigned c)
{
    (void)h;
    (void)c;
    return MSG_FAILED;
}

void mfc_cancel(struct mfc_hub *h)
{
    (void)h;
}

enum msg_status mfc_destroy(struct mfc_hub *h)
{
    (void)h;
    return MSG_FAILED;
}

enum msg_status mfa_init(struct mfa_actor *a, mfa_handler_fn f, void *c,
                         mfa_dead_letter_fn d, void *x)
{
    (void)a;
    (void)f;
    (void)c;
    (void)d;
    (void)x;
    return MSG_FAILED;
}

enum msg_status mfa_send(struct mfa_actor *a, uint64_t g, struct mfo_value *v)
{
    (void)a;
    (void)g;
    (void)v;
    return MSG_FAILED;
}

enum msg_status mfa_step(struct mfa_actor *a)
{
    (void)a;
    return MSG_FAILED;
}

enum msg_status mfa_restart(struct mfa_actor *a)
{
    (void)a;
    return MSG_FAILED;
}

enum msg_status mfa_destroy(struct mfa_actor *a)
{
    (void)a;
    return MSG_FAILED;
}
