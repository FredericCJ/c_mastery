#include "target.h"
int en_target_open(struct en_target *a){(void)a;return ED_INVALID;}
int en_target_cold(struct en_target *a){(void)a;return ED_INVALID;}
struct en_bus en_target_bus(struct en_target *a){(void)a;struct en_bus b={0};return b;}
int en_target_close(struct en_target *a){(void)a;return ED_INVALID;}
