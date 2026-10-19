#include "cmk2.h"
/* Implement MADT validation and bounded channel ownership before SMP entry. */
int cmk2_madt_parse(const void *p,size_t n,struct cmk2_topology *t){(void)p;(void)n;(void)t;return -1;}
int cmk2_channel_send(struct cmk2_channel *q,struct cmk2_message m){(void)q;(void)m;return CMK2_INVALID;}
int cmk2_channel_receive(struct cmk2_channel *q,struct cmk2_message *m){(void)q;(void)m;return CMK2_INVALID;}
void cmk2_channel_close(struct cmk2_channel *q){(void)q;}
