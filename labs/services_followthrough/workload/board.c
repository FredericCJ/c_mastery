/* Learner target body intentionally incomplete; explicit source selection. */
void af_entry(void)
{
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}

void af_hsem(void)
{
}

void af_systick(void)
{
}

void af_fault(void)
{
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}
