#include "cmx_advanced.h"
void cmx_reservation_init(cmx_reservation *q) { (void)q; }
enum cmx_result cmx_reserve(cmx_reservation *q,uint32_t *t) { (void)q;(void)t;return CMX_INVALID; }
enum cmx_result cmx_publish(cmx_reservation *q,uint32_t t,uint32_t v) { (void)q;(void)t;(void)v;return CMX_INVALID; }
enum cmx_result cmx_reserved_pop(cmx_reservation *q,uint32_t *v) { (void)q;(void)v;return CMX_INVALID; }
