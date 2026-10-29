#include "adc.h"

struct pa_io pa_f439_io(void)
{
    struct pa_io io = {0, 0, 0};
    return io;
}

void pa_f439_pins(void)
{
}

int pa_f439_timer(unsigned a, unsigned b, unsigned c)
{
    (void)a;
    (void)b;
    (void)c;
    return PA_UNIMPLEMENTED;
}

void pa_f439_dac_open(void)
{
}

int pa_f439_dac_write(unsigned a)
{
    (void)a;
    return PA_UNIMPLEMENTED;
}

unsigned pa_f439_dac_read(void)
{
    return 0;
}
