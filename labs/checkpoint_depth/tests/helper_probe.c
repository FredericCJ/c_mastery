/* Original inspection specimen: a valid large aggregate copy can emit a helper. */
struct cpw_large
{
    unsigned char bytes[65536];
};

void cpw_copy(struct cpw_large *destination, const struct cpw_large *source)
{
    *destination = *source;
}
