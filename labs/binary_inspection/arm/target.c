#include "record.h"

volatile unsigned initial_size = 5;
volatile unsigned observed;

void result_ready(void)
{
    for (;;) { }
}

void run_record(void)
{
    observed = record_score(3u, initial_size);
    result_ready();
}
