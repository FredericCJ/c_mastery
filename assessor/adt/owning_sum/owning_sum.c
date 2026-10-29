#include "owning_sum.h"
#ifndef ADO_MUTATION
#define ADO_MUTATION 0
#endif
void ado_init(struct ado_value *v)
{
    v->tag = ADO_EMPTY;
    v->data.number = 0;
}

void ado_drop(struct ado_value *v)
{
    if (v->tag == ADO_TEXT)
    {
        struct ado_text text = v->data.text;
        ado_init(v);
        if (ADO_MUTATION != 4)
            text.ops.drop(text.ops.context, text.bytes, text.length);
    }
    else
        ado_init(v);
}

static void publish(struct ado_value *out, struct ado_value staged)
{
    ado_drop(out);
    *out = staged;
}

enum ado_status ado_from_value(struct ado_value *out, struct ad_value value)
{
    uint64_t ignored;
    if (!out || !ad_measure(value, &ignored))
        return ADO_INVALID;
    struct ado_value staged = {ADO_EMPTY, {.number = 0}};
    switch (value.tag)
    {
        case AD_EMPTY:
            break;
        case AD_NUMBER:
            staged.tag = ADO_NUMBER;
            staged.data.number = value.data.number;
            break;
        case AD_PAIR:
            staged.tag = ADO_PAIR;
            staged.data.pair = value.data.pair;
            break;
        default:
            return ADO_INVALID;
    }
    publish(out, staged);
    return ADO_OK;
}

enum ado_status ado_text(struct ado_value *out, const unsigned char *bytes,
                         size_t n, struct ado_ops ops)
{
    if (!out || n > 65535 || (n && !bytes) || !ops.clone || !ops.drop)
        return ADO_INVALID;
    struct ado_value staged = {ADO_TEXT, {.text = {NULL, n, ops}}};
    if (ADO_MUTATION == 2)
        ado_drop(out);
    if (!ops.clone(ops.context, bytes, n, &staged.data.text.bytes))
        return ADO_NOMEM;
    publish(out, staged);
    return ADO_OK;
}

enum ado_status ado_copy(struct ado_value *out, const struct ado_value *in)
{
    if (!out || !in)
        return ADO_INVALID;
    if (out == in)
        return ADO_OK;
    if (in->tag == ADO_TEXT)
    {
        if (ADO_MUTATION == 1)
        {
            publish(out, *in);
            return ADO_OK;
        }
        return ado_text(out, in->data.text.bytes, in->data.text.length,
                        in->data.text.ops);
    }
    switch (in->tag)
    {
        case ADO_EMPTY:
        case ADO_NUMBER:
        case ADO_PAIR:
            publish(out, *in);
            return ADO_OK;
        case ADO_TEXT:
            break;
    }
    return ADO_INVALID;
}

enum ado_status ado_move(struct ado_value *out, struct ado_value *in)
{
    if (!out || !in)
        return ADO_INVALID;
    if (out == in)
        return ADO_OK;
    struct ado_value staged = *in;
    if (ADO_MUTATION != 5)
        ado_init(in);
    publish(out, staged);
    return ADO_OK;
}

enum ado_status ado_measure(const struct ado_value *in, uint64_t *out)
{
    if (!in || !out)
        return ADO_INVALID;
    struct ad_value value = {AD_EMPTY, {.number = 0}};
    switch (in->tag)
    {
        case ADO_EMPTY:
            break;
        case ADO_NUMBER:
            value.tag = AD_NUMBER;
            value.data.number = in->data.number;
            break;
        case ADO_PAIR:
            value.tag = AD_PAIR;
            value.data.pair = in->data.pair;
            break;
        case ADO_TEXT:
            *out = ADO_MUTATION == 3 ? 0 : in->data.text.length;
            return ADO_OK;
        default:
            return ADO_INVALID;
    }
    return ad_measure(value, out) ? ADO_OK : ADO_INVALID;
}

enum ado_status ado_number(const struct ado_value *in, uint32_t *out)
{
    if (!in || !out)
        return ADO_INVALID;
    if (ADO_MUTATION == 6 && in->tag == ADO_TEXT)
    {
        *out = 0;
        return ADO_OK;
    }
    if (in->tag != ADO_NUMBER)
        return ADO_INVALID;
    return ad_number((struct ad_value){AD_NUMBER, {.number = in->data.number}},
                     out)
               ? ADO_OK
               : ADO_INVALID;
}
