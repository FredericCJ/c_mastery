#include "acpi.h"
#include "capture_cases.h"
#include <stdio.h>

int main(void)
{
    if (sa_capture_cases())
    {
        fputs("SA_CAPTURE_CHECK:discovery\n", stderr);
        return 1;
    }
    puts("SA_CAPTURE_PASS:6-actual-root-and-MADT-inventories");
    return 0;
}
