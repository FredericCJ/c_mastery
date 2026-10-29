#include <stdio.h>
static unsigned trace;

static void record(int *value)
{
    trace = trace * 10 + (unsigned)*value;
}

static int sample(int early)
{
    int first __attribute__((cleanup(record))) = 1;
    int second __attribute__((cleanup(record))) = 2;
    (void)first;
    (void)second;
    if (early)
    {
        return 7;
    }
    return 9;
}

int main(void)
{
    if (sample(0) != 9 || trace != 21)
    {
        return 1;
    }
    trace = 0;
    if (sample(1) != 7 || trace != 21)
    {
        return 2;
    }
    puts("LC_CLEANUP normal21 early21");
    return 0;
}
