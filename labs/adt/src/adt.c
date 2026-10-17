#include "adt.h"
/* Intentionally incomplete; implement the canonical exercise contracts. */
int ad_measure(ADValue value, uint64_t *out) { (void)value; (void)out; return 0; }
int ad_number(ADValue value, uint32_t *out) { (void)value; (void)out; return 0; }
int ad_milliseconds(ADSeconds value, ADMillis *out) { (void)value; (void)out; return 0; }
int ad_flags(uint32_t raw, ADFlags *out) { (void)raw; (void)out; return 0; }
int ad_text(const unsigned char *bytes, size_t length, ADText *out) { (void)bytes; (void)length; (void)out; return 0; }
void ad_pool_init(ADPool *pool) { (void)pool;  }
int ad_pool_acquire(ADPool *pool, uint32_t value, ADBrand *out) { (void)pool; (void)value; (void)out; return 0; }
int ad_pool_get(const ADPool *pool, ADBrand handle, uint32_t *out) { (void)pool; (void)handle; (void)out; return 0; }
int ad_pool_release(ADPool *pool, ADBrand handle) { (void)pool; (void)handle; return 0; }
int ad_view(const uint32_t *data, size_t capacity, size_t rows, size_t columns,
            size_t row_stride, size_t column_stride, ADView *out) { (void)data; (void)capacity; (void)rows; (void)columns; (void)row_stride; (void)column_stride; (void)out; return 0; }
int ad_view_get(ADView view, size_t row, size_t column, uint32_t *out) { (void)view; (void)row; (void)column; (void)out; return 0; }
int ad_eval(const ADNode *nodes, size_t count, ADNodeIndex root, uint32_t *out) { (void)nodes; (void)count; (void)root; (void)out; return 0; }
ADPair ad_lens_x(ADPair source, uint32_t replacement) { (void)source; (void)replacement; return (ADPair){0,0}; }
ADValue ad_review_number(uint32_t value) { (void)value; return (ADValue){AD_EMPTY,{.number=0}}; }
int ad_preview_number(ADValue value, uint32_t *out) { (void)value; (void)out; return 0; }
int ad_evolve_read(const unsigned char *bytes, size_t length, ADEvolving *out) { (void)bytes; (void)length; (void)out; return 0; }
int ad_evolve_write(ADEvolving value, unsigned char *bytes, size_t capacity, size_t *used) { (void)value; (void)bytes; (void)capacity; (void)used; return 0; }
ADPacketError ad_packet(const unsigned char *bytes, size_t length, ADPacket *out) { (void)bytes; (void)length; (void)out; return AD_PACKET_HEADER; }
