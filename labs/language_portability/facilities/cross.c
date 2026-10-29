volatile unsigned observed;

void entry(void)
{
    int *p = nullptr;
    observed = (p == nullptr);
    for (;;)
    {
    }
}
