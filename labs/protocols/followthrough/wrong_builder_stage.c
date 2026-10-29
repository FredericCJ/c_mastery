#include "alternatives.h"

int main(void)
{
    struct pa_empty empty = {{0}};
    struct pt_config out;
    return pa_finish(empty, &out);
}
