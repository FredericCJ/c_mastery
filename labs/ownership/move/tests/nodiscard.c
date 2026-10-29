[[nodiscard("ownership outcome")]] int acquire_outcome(void)
{
    return 7;
}

void compare_discard(void)
{
#ifdef OMM_CAST
    (void)acquire_outcome();
#else
    acquire_outcome();
#endif
}
