#define A int
#define B separated
#define VALUE 7
#define PREFIX "north:"
A /**/ B = 7;
static const char raw[] = "VALUE";
static const char joined[] = PREFIX "gate";

int main(void)
{
    const char expected[] = "north:gate";
    if (separated != 7 || sizeof raw != 6 || raw[0] != 'V' || raw[4] != 'E' ||
        raw[5] != 0 || sizeof joined != sizeof expected)
    {
        return 1;
    }
    for (unsigned i = 0; i < sizeof expected; i++)
    {
        if (joined[i] != expected[i])
        {
            return 1;
        }
    }
    return 0;
}
