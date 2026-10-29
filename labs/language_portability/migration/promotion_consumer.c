#ifdef LP_PROTOTYPE
int lp_promoted(double);
#else
int lp_promoted();
#endif
int main(void)
{
    float value = 1.25f;
    return lp_promoted(value) ? 0 : 1;
}
