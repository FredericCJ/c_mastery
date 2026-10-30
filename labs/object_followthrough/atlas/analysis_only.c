/*
 * CM-SPECIMEN: defective by design; classify each defect from the source
 * (see CONTRACTS.md).
 * SOURCE ANALYSIS ONLY: never link, call or execute these original examples.
 */
int *oa_expired_block(void)
{
    int values[2] = {7, 8};
    return values;
}

int *oa_expired_literal(void)
{
    return (int[]){7, 8};
}

/* Given an actually misaligned byte address, even conversion to the stricter
 * pointer is outside the C alignment requirement. No experiment evaluates it.
 * A target instruction accepting unaligned operands does not change this rule.
 * double *bad = (double *)(byte_base + 1);
 */
