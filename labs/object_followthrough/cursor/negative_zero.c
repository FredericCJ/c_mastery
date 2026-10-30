/* CM-SPECIMEN: does not compile by design; predict the diagnostic first
 * (see CONTRACTS.md). Compile-only negative source; never linked or run. */
int *wrong_zero(void)
{
    int zero = 0;
    int *p = zero;
    return p;
}
