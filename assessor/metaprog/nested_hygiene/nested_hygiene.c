#include "nested_hygiene.h"
#include <limits.h>
#ifndef MPH_MUTATION
#define MPH_MUTATION 0
#endif
struct mp_engine
{
    const struct mph_program *program;
    const struct mph_arguments *arguments;
    struct mph_budget budget;
    unsigned next;
    struct mph_expansion result;
};

static int invoke(struct mp_engine *e, unsigned definition, unsigned parent,
                  unsigned depth, unsigned bound, unsigned indices[2])
{
    unsigned context;
    size_t i;
    if (depth > e->budget.depth + (MPH_MUTATION == 3 ? 1u : 0u) || depth > 8 ||
        e->result.steps == e->budget.steps)
    {
        return MPH_BUDGET;
    }
    if (e->next == UINT_MAX)
    {
        return MPH_EXHAUSTED;
    }
    context = e->next++;
    ++e->result.steps;
    ++e->result.invocations;
    if (depth > e->result.peak_depth)
    {
        e->result.peak_depth = depth;
    }
    for (i = 0; i < e->program->definitions[definition].count; ++i)
    {
        struct mph_instruction ins =
            e->program->definitions[definition].code[i];
        int status;
        if (e->result.steps == e->budget.steps)
        {
            return MPH_BUDGET;
        }
        ++e->result.steps;
        if (ins.operation == MPH_CALL)
        {
            status = invoke(e, ins.target, context, depth + 1, bound, indices);
            if (status)
            {
                return status;
            }
        }
        else if (ins.operation == MPH_REPEAT)
        {
            unsigned repetitions = 0, first = 1, j, k;
            if (bound & ins.mask)
            {
                return MPH_SHAPE;
            }
            for (j = 0; j < 2; ++j)
            {
                if (ins.mask & (1u << j))
                {
                    if (first)
                    {
                        repetitions = (unsigned)e->arguments[j].count;
                        first = 0;
                    }
                    else if (repetitions != e->arguments[j].count)
                    {
                        if (MPH_MUTATION != 2)
                        {
                            return MPH_SHAPE;
                        }
                        if (e->arguments[j].count < repetitions)
                        {
                            repetitions = (unsigned)e->arguments[j].count;
                        }
                    }
                }
            }
            for (k = 0; k < repetitions; ++k)
            {
                unsigned nested[2] = {indices[0], indices[1]};
                for (j = 0; j < 2; ++j)
                {
                    if (ins.mask & (1u << j))
                    {
                        nested[j] = k;
                    }
                }
                status = invoke(e, ins.target, context, depth + 1,
                                bound | ins.mask, nested);
                if (status)
                {
                    return status;
                }
            }
        }
        else
        {
            struct mp_token pattern = ins.token, argument = {0, 0, 0}, token;
            unsigned local = context;
            size_t used = 0;
            if (e->result.count == e->budget.tokens)
            {
                return MPH_BUDGET;
            }
            if (ins.operation == MPH_ARGUMENT)
            {
                unsigned index =
                    (bound & (1u << ins.target)) ? indices[ins.target] : 0;
                if (!(bound & (1u << ins.target)) &&
                    e->arguments[ins.target].count != 1)
                {
                    return MPH_SHAPE;
                }
                if (index >= e->arguments[ins.target].count)
                {
                    return MPH_SHAPE;
                }
                argument = e->arguments[ins.target].tokens[index];
                pattern = (struct mp_token){1, 0, 0};
                if (MPH_MUTATION == 1)
                {
                    argument.context = context;
                }
            }
            else if (ins.operation == MPH_INTRODUCED)
            {
                pattern.kind = 2;
            }
            else if (ins.operation == MPH_ESCAPE)
            {
                if (!e->budget.allow_capture && MPH_MUTATION != 5)
                {
                    return MPH_CAPTURE;
                }
                pattern.kind = 0;
                pattern.context = parent;
            }
            else
            {
                pattern.kind = 0;
            }
            if (!mp_expand(&pattern, 1, &argument, 1, &local, &token, 1,
                           &used) ||
                used != 1)
            {
                return MPH_INVALID;
            }
            e->result.tokens[e->result.count++] = token;
        }
    }
    return MPH_OK;
}

int mph_expand(const struct mph_program *p, unsigned entry,
               const struct mph_arguments args[2], unsigned caller,
               struct mph_budget budget, unsigned *next,
               struct mph_expansion *output)
{
    struct mp_engine e;
    size_t i, j;
    unsigned indices[2] = {0, 0};
    int status;
    if (!p || !args || !next || !output || !p->count || p->count > 4 ||
        entry >= p->count || !budget.depth || budget.depth > 8 ||
        budget.tokens > 64 || !budget.steps || budget.steps > 256 ||
        budget.allow_capture > 1 || !*next || caller >= *next)
    {
        return MPH_INVALID;
    }
    for (i = 0; i < 2; ++i)
    {
        if (args[i].count > 4)
        {
            return MPH_INVALID;
        }
        for (j = 0; j < args[i].count; ++j)
        {
            if (args[i].tokens[j].kind > 2 ||
                args[i].tokens[j].context >= *next)
            {
                return MPH_INVALID;
            }
        }
    }
    for (i = 0; i < p->count; ++i)
    {
        if (p->definitions[i].count > 16)
        {
            return MPH_INVALID;
        }
        for (j = 0; j < p->definitions[i].count; ++j)
        {
            struct mph_instruction n = p->definitions[i].code[j];
            if (n.operation > MPH_ESCAPE)
            {
                return MPH_INVALID;
            }
            if (n.operation == MPH_ARGUMENT && n.target > 1)
            {
                return MPH_INVALID;
            }
            if ((n.operation == MPH_CALL || n.operation == MPH_REPEAT) &&
                n.target >= p->count)
            {
                return MPH_INVALID;
            }
            if (n.operation == MPH_REPEAT && (!n.mask || n.mask > 3))
            {
                return MPH_INVALID;
            }
            if (n.operation == MPH_LITERAL && n.token.context >= *next)
            {
                return MPH_INVALID;
            }
        }
    }
    e = (struct mp_engine){p, args, budget, *next, {{{0, 0, 0}}, 0, 0, 0, 0}};
    status = invoke(&e, entry, caller, 1, 0, indices);
    if (status)
    {
        if (MPH_MUTATION == 4)
        {
            *next = e.next;
        }
        return status;
    }
    *output = e.result;
    *next = e.next;
    return MPH_OK;
}

int mph_resolve(const struct mph_binding *env, size_t count, struct mp_token id,
                int *output)
{
    if (!output || count > 64 || (count && !env))
    {
        return MPH_INVALID;
    }
    while (count)
    {
        const struct mph_binding *b = &env[--count];
        if (b->identifier.symbol == id.symbol &&
            (MPH_MUTATION == 6 || b->identifier.context == id.context))
        {
            *output = b->value;
            return MPH_OK;
        }
    }
    return MPH_NOT_FOUND;
}
