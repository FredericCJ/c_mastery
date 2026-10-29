/* Included at the materializer boundary; all metadata calls hold kernel_lock. */
static int sf_fault_remaining = -1;
static unsigned sf_seed_frames;
static _Atomic unsigned sf_fault_gate, sf_fault_arrivals;
static uint64_t sf_fault_address;

static void sf_fault_before_lock(struct frame *f)
{
    if (f->vector != 14 || (f->cs & 3) != 3 ||
        !atomic_load_explicit(&sf_fault_gate, memory_order_acquire))
        return;
    uint64_t address;
    __asm__ volatile("mov %%cr2,%0" : "=r"(address));
    if ((address & ~UINT64_C(4095)) != sf_fault_address)
        return;
    atomic_fetch_add_explicit(&sf_fault_arrivals, 1, memory_order_acq_rel);
    unsigned attempts = 0;
    while (atomic_load_explicit(&sf_fault_arrivals, memory_order_acquire) < 2)
    {
        if (++attempts == 100000000)
            panic("sf fault gate missing peer");
        __asm__ volatile("pause");
    }
}

static uint32_t sf_fault_take(void)
{
    if (sf_fault_remaining == 0)
        return 0;
    if (sf_fault_remaining > 0)
        sf_fault_remaining--;
    uint32_t p = cmk_frame_take(&frames);
    if (p && sf_seed_frames)
        memset((void *)(uintptr_t)p, 0xa5, 4096);
    return p;
}

/* Classification uses the actual saved x86 error bits and current PTE. */
static unsigned sf_fault_class(uint64_t error, uint64_t entry)
{
    if (!(error & 4) || (error & 8))
        return 0;
    if (!(error & 1) && (entry & PTE_ZERO) && !(error & 16))
        return 1;
    if ((error & 7) == 7 && (entry & PTE_COW) && !(error & 16))
        return 2;
    if ((entry & 5) == 5 && (!(error & 2) || (entry & 2)) &&
        (!(error & 16) || !(entry & PTE_NX)))
        return 3;
    return 0;
}
