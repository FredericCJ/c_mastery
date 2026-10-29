#include "service.h"
#include <string.h>

/* State:0 idle,1 running,2 result retained,3 cancelled. Only owner accesses. */
int sfv_open(struct sfv_service *s, unsigned c, uint64_t session)
{
    if (!s || c >= 2 || !session)
    {
        return SFV_ARGUMENT;
    }
    struct sfv_slot *v = &s->slots[c];
    if (v->state == 1)
    {
        return SFV_BUSY;
    }
    if (session <= v->session)
    {
        return SFV_STALE;
    }
    memset(v, 0, sizeof *v);
    v->session = session;
    return SFV_OK;
}

int sfv_submit(struct sfv_service *s, unsigned c, const struct sfv_request *r)
{
    if (!s || !r || c >= 2 || !r->session || !r->id || r->reserved ||
        r->length > SFV_IMAGE || !r->length || !r->fuel || r->fuel > 100000 ||
        r->gc > 100000)
    {
        return SFV_ARGUMENT;
    }
    struct sfv_slot *v = &s->slots[c];
    if (r->session != v->session)
    {
        return SFV_STALE;
    }
    if (r->id == v->high)
    {
        if (r->length != v->request.length || r->fuel != v->request.fuel ||
            r->gc != v->request.gc ||
            memcmp(r->image, v->request.image, r->length))
        {
            return SFV_CONFLICT;
        }
        return v->state == 3 ? SFV_CANCELLED
                             : (v->state == 1 ? SFV_RUNNING : SFV_OK);
    }
    if (r->id < v->high)
    {
        return SFV_STALE;
    }
    if (v->state == 1 || v->state == 2)
    {
        return SFV_BUSY;
    }
    struct cvm_program image;
    struct vv_report report;
    if (vv_load(r->image, r->length, 0, 1, &s->scratch, &image, &report) !=
        VV_OK)
    {
        return CVM_VERIFY;
    }
    int result = vs_start(&v->execution, &image, VS_CHECKED_V1, r->fuel, r->gc,
                          0, 0, &s->scratch);
    if (result != VS_RUNNING)
    {
        return result;
    }
    v->execution.vm.arena = 1;
    v->request = *r;
    v->high = r->id;
    v->state = 1;
    v->length = 0;
    v->status = SFV_RUNNING;
    return SFV_RUNNING;
}

static int encode(struct cvm_cvm *vm, struct cvm_value value, uint8_t *out,
                  size_t *length, unsigned depth)
{
    if (depth > CVM_HEAP || *length >= SFV_RESULT)
    {
        return SFV_CAPACITY;
    }
    out[(*length)++] = (uint8_t)value.tag;
    if (value.tag == CVM_INT || value.tag == CVM_BOOL)
    {
        if (SFV_RESULT - *length < 4)
        {
            return SFV_CAPACITY;
        }
        for (unsigned i = 0; i < 4; i++)
        {
            out[(*length)++] = (uint8_t)((uint32_t)value.integer >> (8 * i));
        }
        return SFV_OK;
    }
    if (value.tag == CVM_NIL)
    {
        return SFV_OK;
    }
    if (value.tag != CVM_REF)
    {
        return CVM_TYPE;
    }
    struct cvm_value left, right;
    if (cvm_field(vm, value, 0, &left) || cvm_field(vm, value, 1, &right))
    {
        return CVM_STALE;
    }
    int r = encode(vm, left, out, length, depth + 1);
    return r ? r : encode(vm, right, out, length, depth + 1);
}

int sfv_step(struct sfv_service *s, unsigned c)
{
    if (!s || c >= 2)
    {
        return SFV_ARGUMENT;
    }
    struct sfv_slot *v = &s->slots[c];
    if (v->state != 1)
    {
        return v->state == 3 ? SFV_CANCELLED : SFV_STALE;
    }
    struct cvm_value out;
    int r = vs_step(&v->execution, &out);
    if (r == VS_RUNNING)
    {
        return SFV_RUNNING;
    }
    v->state = 2;
    v->status = r == VS_DONE ? SFV_OK : (unsigned)r;
    v->length = 0;
    if (r == VS_DONE)
    {
        v->result = out;
        uint8_t bytes[SFV_RESULT];
        size_t length = 0;
        int e = encode(&v->execution.vm, out, bytes, &length, 0);
        if (e)
        {
            v->status = (unsigned)e;
        }
        else
        {
            memcpy(v->bytes, bytes, length);
            v->length = length;
        }
    }
    return (int)v->status;
}

int sfv_cancel(struct sfv_service *s, unsigned c, uint64_t session, uint64_t id)
{
    if (!s || c >= 2)
    {
        return SFV_ARGUMENT;
    }
    struct sfv_slot *v = &s->slots[c];
    if (session != v->session || !id || id != v->high)
    {
        return SFV_STALE;
    }
    if (v->state == 0)
    {
        return SFV_STALE;
    }
    v->execution.state = 1;
    v->state = 3;
    v->status = SFV_CANCELLED;
    v->length = 0;
    return SFV_CANCELLED;
}

int sfv_reply(struct sfv_service *s, unsigned c, uint64_t session, uint64_t id,
              void *out, size_t capacity, size_t *length, unsigned *status)
{
    if (!s || c >= 2 || !length || !status || (!out && capacity))
    {
        return SFV_ARGUMENT;
    }
    struct sfv_slot *v = &s->slots[c];
    if (session != v->session || !id || id != v->high)
    {
        return SFV_STALE;
    }
    if (v->state == 1)
    {
        return SFV_RUNNING;
    }
    if (v->state == 3)
    {
        return SFV_CANCELLED;
    }
    if (v->state != 2)
    {
        return SFV_STALE;
    }
    if (capacity < v->length)
    {
        return SFV_CAPACITY;
    }
    if (v->length && !out)
    {
        return SFV_ARGUMENT;
    }
    if (v->length)
    {
        memcpy(out, v->bytes, v->length);
    }
    *length = v->length;
    *status = v->status;
    v->state = 0;
    return SFV_OK;
}

int sfv_handle(struct sfv_service *s, unsigned c, struct sfv_handle *out)
{
    if (!s || c >= 2 || !out)
    {
        return SFV_ARGUMENT;
    }
    struct sfv_slot *v = &s->slots[c];
    if (v->state != 2 || v->status || v->result.tag != CVM_REF)
    {
        return SFV_STALE;
    }
    *out = (struct sfv_handle){v->session, v->high, c + 1, 0, v->result};
    return SFV_OK;
}

int sfv_field(struct sfv_service *s, unsigned c, const struct sfv_handle *h,
              int right, struct cvm_value *out)
{
    if (!s || c >= 2 || !h || !out || (right != 0 && right != 1))
    {
        return SFV_ARGUMENT;
    }
    struct sfv_slot *v = &s->slots[c];
#ifndef SFV_MUTANT_FOREIGN
    if (h->instance != c + 1 || h->session != v->session || h->id != v->high ||
        h->reserved)
    {
        return SFV_FOREIGN;
    }
#endif
    if (v->state != 2)
    {
        return SFV_STALE;
    }
    return cvm_field(&v->execution.vm, h->value, right, out);
}
