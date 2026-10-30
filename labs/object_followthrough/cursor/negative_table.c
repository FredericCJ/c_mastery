/* CM-SPECIMEN: does not compile by design; predict the diagnostic first
 * (see CONTRACTS.md). Compile-only negative source; never linked or run. */
void need_table(int **);

void wrong_matrix(void)
{
    int rows[2][3] = {{0}};
    need_table(rows);
}
