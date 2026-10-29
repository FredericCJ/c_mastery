/* Repeatable specialization: policy expression fixed0/1 or actual parameter. */
#if !defined(MPP_NAME) || !defined(MPP_PARAMETERS) || !defined(MPP_SATURATE)
#error MPP_REQUIRED_PARAMETERS
#endif
int MPP_NAME(MPP_PARAMETERS)
{
    if (!out || count > 32 || MPP_SATURATE > 1 || !ops.acquire ||
        !ops.project || !ops.release)
        return MPP_INVALID;
    uint32_t sum = 0;
    for (size_t i = 0; i < count; i++)
    {
        if (MPP_MUTATION == 3 && MPP_SATURATE && sum == limit)
            break;
        struct mpp_item item = {0};
        uint32_t term = 0;
        size_t index = MPP_MUTATION == 4 ? count - 1 - i : i;
        if (ops.acquire(ops.context, index, &item))
        {
            if (MPP_MUTATION != 1)
                ops.release(ops.context, &item);
            return MPP_ACQUIRE;
        }
        int failure = 0;
        if (!(MPP_MUTATION == 5 && MPP_SATURATE && sum == limit))
            failure = ops.project(ops.context, &item, &term);
        if (MPP_MUTATION == 6 && !failure)
            failure = ops.project(ops.context, &item, &term);
        ops.release(ops.context, &item);
        if (failure)
        {
            if (MPP_MUTATION == 2)
                *out = sum;
            return MPP_PROJECT;
        }
        if (term > limit - sum)
        {
            if (!MPP_SATURATE)
                return MPP_RANGE;
            sum = limit;
        }
        else
            sum += term;
    }
    *out = sum;
    return MPP_OK;
}

#undef MPP_NAME
#undef MPP_PARAMETERS
#undef MPP_SATURATE
