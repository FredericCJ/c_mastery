/* Compile for diagnostics only. Never execute the counterfactual unsafe write. */
void qualifier_probe(char **source)
{
    const char **destination = source;
    (void)destination;
}
