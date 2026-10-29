/* Compile only: this function is never called in the laboratory. */
int ob_warning(int initialized)
{
    int value;
    if (initialized)
    {
        value = 17;
    }
    return value;
}
