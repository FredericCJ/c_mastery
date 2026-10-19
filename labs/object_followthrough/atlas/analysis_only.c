/* SOURCE ANALYSIS ONLY. Never link or execute these original defective examples. */
int *oa_expired_block(void)
{
    int values[2]={7,8};
    return values;
}
int *oa_expired_literal(void)
{
    return (int[]){7,8};
}
/* Given an actually misaligned byte address, even conversion to the stricter
 * pointer is outside the C alignment requirement. No experiment evaluates it.
 * A target instruction accepting unaligned operands does not change this rule.
 * double *bad = (double *)(byte_base + 1);
 */
