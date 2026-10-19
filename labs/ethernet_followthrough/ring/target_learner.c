#include "target.h"
int en_target_open(EnTarget *a){(void)a;return ED_INVALID;}
int en_target_cold(EnTarget *a){(void)a;return ED_INVALID;}
EnBus en_target_bus(EnTarget *a){(void)a;EnBus b={0};return b;}
int en_target_close(EnTarget *a){(void)a;return ED_INVALID;}
