#include "errors.h"
#ifndef ER_MUTANT
#define ER_MUTANT 0
#endif

static int valid(er_code code)
{
    return code >= ER_OK && code <= ER_NOT_IMPLEMENTED;
}

er_result er_ok(uint32_t value)
{
    er_result r = {ER_OK, {.value = value}};
    return r;
}

er_result er_error(er_code code, size_t offset)
{
    er_result r = {code, {.offset = offset}};
    if (!valid(code) || code == ER_OK)
        r.tag = ER_INVALID;
    return r;
}

er_code er_optional_pack(uint32_t value, int present, uint32_t *encoded)
{
    if (!encoded || (present != 0 && present != 1))
        return ER_INVALID;
    if (ER_MUTANT == 1)
        *encoded = value;
    if (present && value == 0)
        return ER_RANGE;
    *encoded = (present || ER_MUTANT == 2) ? value : 0;
    return ER_OK;
}

er_result er_optional_unpack(uint32_t encoded)
{
    return encoded ? er_ok(encoded) : er_error(ER_ABSENT, 0);
}

static er_result decimal(const unsigned char *s, size_t n, size_t *position)
{
    size_t i = *position, start = i;
    uint32_t v = 0;
    while (i < n && s[i] >= '0' && s[i] <= '9')
    {
        uint32_t digit = (uint32_t)(s[i] - '0');
        if (v > (UINT32_C(65535) - digit) / 10)
            return er_error(ER_RANGE, i);
        v = v * 10 + digit;
        ++i;
    }
    if (i == start)
        return er_error(ER_INVALID, i);
    *position = i;
    return er_ok(v);
}

er_result er_parse_pair(const unsigned char *s, size_t n, uint32_t *sum)
{
    size_t position = 0;
    er_result a, b;
    if ((!s && n) || !sum)
        return er_error(ER_INVALID, 0);
    a = decimal(s, n, &position);
    if (a.tag != ER_OK)
        return a;
    if (ER_MUTANT == 3)
        *sum = a.as.value;
    if (position == n || s[position] != ':')
        return er_error(ER_INVALID, position);
    ++position;
    b = decimal(s, n, &position);
    if (b.tag != ER_OK)
        return b;
    if (ER_MUTANT != 4 && position != n)
        return er_error(ER_INVALID, position);
    if (a.as.value > UINT32_C(65535) - b.as.value)
        return er_error(ER_RANGE, position);
    *sum = a.as.value + b.as.value;
    return er_ok(*sum);
}

er_progress er_transfer(const unsigned char *bytes, size_t length, size_t next,
                        er_sink sink, void *context)
{
    er_progress p = {next, ER_INVALID}, backend;
    if ((!bytes && length) || next > length || !sink)
        return p;
    if (next == length)
    {
        p.code = ER_OK;
        return p;
    }
    backend = sink(context, bytes + next, length - next);
    if (backend.next > length - next ||
        (backend.code != ER_OK && backend.code != ER_AGAIN &&
         backend.code != ER_IO) ||
        (backend.code == ER_OK && backend.next == 0))
    {
        p.code = ER_CONTRACT;
        return p;
    }
    p.next =
        (ER_MUTANT == 5 && backend.code != ER_OK) ? next : next + backend.next;
    p.code = ER_MUTANT == 6 ? ER_OK : backend.code;
    return p;
}

er_result er_map(er_result input, uint32_t (*fn)(uint32_t, void *),
                 void *context)
{
    if (!valid(input.tag))
        return er_error(ER_INVALID, 0);
    if (input.tag != ER_OK)
    {
        if (ER_MUTANT == 7 && fn)
            (void)fn(0, context);
        return input;
    }
    if (!fn)
        return er_error(ER_INVALID, 0);
    return er_ok(fn(input.as.value, context));
}

er_result er_then(er_result input, er_result (*fn)(uint32_t, void *),
                  void *context)
{
    er_result result;
    if (!valid(input.tag))
        return er_error(ER_INVALID, 0);
    if (input.tag != ER_OK)
        return input;
    if (!fn)
        return er_error(ER_INVALID, 0);
    result = fn(input.as.value, context);
    if (!valid(result.tag))
        return er_error(ER_INVALID, 0);
    return ER_MUTANT == 8 ? er_ok(0) : result;
}

er_validation er_validate(er_config input, er_config *output, unsigned *errors,
                          size_t capacity)
{
    er_validation r = {0, 0, 0};
    int bad[3] = {input.age < 0 || input.age > 150,
                  input.quota < 0 || input.quota > 1024,
                  input.denominator < 1 || input.denominator > 4096};
    size_t i;
    /* Invalid structural arguments use a sentinel; no object is touched. */
    if (!output || capacity > 3 || (capacity && !errors))
    {
        r.total = SIZE_MAX;
        return r;
    }
    if (ER_MUTANT == 9)
        *output = input;
    for (i = 0; i < 3; ++i)
        if (bad[i])
        {
            ++r.total;
            if (r.stored < capacity)
                errors[r.stored++] = (unsigned)i + 1;
            if (ER_MUTANT == 10)
                break;
        }
    r.truncated = r.stored < r.total;
    if (r.total == 0)
        *output = input;
    return r;
}

er_code er_defer(er_frame *frame, er_cleanup fn, void *context)
{
    if (!frame || !frame->active || frame->cleaning || !fn)
        return ER_INVALID;
    if (frame->count == 4)
        return ER_RANGE;
    frame->cleanup[frame->count].fn = fn;
    frame->cleanup[frame->count++].context = context;
    return ER_OK;
}

er_code er_raise(er_frame *frame, er_code code)
{
    if (!frame || !frame->active || frame->cleaning || !valid(code) ||
        code == ER_OK)
        return ER_INVALID;
    frame->thrown = code;
    longjmp(frame->jump, 1);
}

er_result er_run(er_frame *frame, er_result (*body)(er_frame *, void *),
                 void *context)
{
    er_result result;
    if (!frame || !body || frame->active || frame->cleaning)
        return er_error(ER_INVALID, 0);
    frame->count = 0;
    frame->active = 1;
    frame->thrown = ER_OK;
    if (setjmp(frame->jump) == 0)
        result = body(frame, context);
    else
        result = er_error(frame->thrown, 0);
    if (!valid(result.tag))
        result = er_error(ER_INVALID, 0);
    frame->cleaning = 1;
    if (ER_MUTANT == 11)
        frame->count = 0;
    while (frame->count)
    {
        size_t i = ER_MUTANT == 12 ? 0 : frame->count - 1;
        er_cleanup fn = frame->cleanup[i].fn;
        void *ctx = frame->cleanup[i].context;
        --frame->count;
        if (ER_MUTANT == 12)
        {
            size_t j;
            for (j = 0; j < frame->count; ++j)
                frame->cleanup[j] = frame->cleanup[j + 1];
        }
        fn(ctx);
    }
    frame->active = 0;
    frame->cleaning = 0;
    return result;
}

er_code er_add16(uint32_t a, uint32_t b, er_policy policy, uint32_t *output)
{
    uint32_t sum;
    if (!output || a > 65535 || b > 65535 || policy < ER_CHECKED ||
        policy > ER_SATURATING)
        return ER_INVALID;
    sum = a + b;
    if (sum > 65535)
    {
        if (ER_MUTANT == 13)
            *output = sum & 65535;
        if (policy == ER_CHECKED)
            return ER_RANGE;
        sum = policy == ER_WRAPPING ? (sum & 65535) : 65535;
    }
    *output = sum;
    return ER_OK;
}

er_code er_serial16(uint32_t a, uint32_t b, int *order)
{
    uint32_t delta;
    if (!order || a > 65535 || b > 65535)
        return ER_INVALID;
    delta = (a + UINT32_C(65536) - b) & 65535;
    if (ER_MUTANT != 14 && delta == 32768)
        return ER_RANGE;
    *order = delta == 0 ? 0 : delta < 32768 ? 1 : -1;
    return ER_OK;
}

er_code er_contract_counter(er_mode mode, int amount, int *counter,
                            size_t *reports)
{
    int candidate, violation;
    if (!counter || !reports || *counter < 0 || *counter > 100 ||
        amount < -100 || amount > 100 || mode < ER_OFF || mode > ER_ENFORCE)
        return ER_INVALID;
    candidate =
        *counter + amount; /* [-100,200], representable even on 16-bit int. */
    violation = candidate < 0 || candidate > 100;
    if (mode != ER_OFF && violation)
    {
        if (*reports == SIZE_MAX)
            return ER_RANGE;
        if (ER_MUTANT != 15)
            ++*reports;
        if (mode == ER_ENFORCE && ER_MUTANT != 16)
            return ER_CONTRACT;
    }
    *counter = candidate < 0 ? 0 : candidate > 100 ? 100 : candidate;
    return ER_OK;
}

er_code er_refine(uint32_t n, uint32_t d, er_fraction *output)
{
    uint32_t a = n, b = d;
    if (!output || d == 0)
        return ER_INVALID;
    while (b)
    {
        uint32_t t = a % b;
        a = b;
        b = t;
    }
    if (ER_MUTANT == 17)
        a = 1;
    output->numerator = n / a;
    output->denominator = ER_MUTANT == 18 && n == 0 ? d : d / a;
    return ER_OK;
}
