#ifndef CMASTERY_ADT_H
#define CMASTERY_ADT_H
#include <stddef.h>
#include <stdint.h>

enum ad_tag { AD_EMPTY, AD_NUMBER, AD_PAIR };
struct ad_pair { uint32_t x, y; };
struct ad_value { enum ad_tag tag; union { uint32_t number; struct ad_pair pair; } data; };
int ad_measure(struct ad_value value, uint64_t *out);
int ad_number(struct ad_value value, uint32_t *out);
struct ad_seconds { uint32_t value; };
struct ad_millis { uint32_t value; };
int ad_milliseconds(struct ad_seconds value, struct ad_millis *out);
struct ad_flags { uint32_t bits; };
int ad_flags(uint32_t raw, struct ad_flags *out);
struct ad_text { const unsigned char *bytes; size_t length, scalars; };
int ad_text(const unsigned char *bytes, size_t length, struct ad_text *out);

/* Pool address must remain fixed, live, and never reused while handles survive. */
struct ad_slot { unsigned generation, live, retired; uint32_t value; };
struct ad_pool { struct ad_slot slot[4]; };
struct ad_brand { const struct ad_pool *owner; unsigned index, generation; };
struct ad_node_index { unsigned index; };
void ad_pool_init(struct ad_pool *pool);
int ad_pool_acquire(struct ad_pool *pool, uint32_t value, struct ad_brand *out);
int ad_pool_get(const struct ad_pool *pool, struct ad_brand handle, uint32_t *out);
int ad_pool_release(struct ad_pool *pool, struct ad_brand handle);

struct ad_view { const uint32_t *data; size_t capacity, rows, columns, row_stride, column_stride; };
int ad_view(const uint32_t *data, size_t capacity, size_t rows, size_t columns,
            size_t row_stride, size_t column_stride, struct ad_view *out);
int ad_view_get(struct ad_view view, size_t row, size_t column, uint32_t *out);
struct ad_node { unsigned kind; uint32_t value; struct ad_node_index left, right; };
int ad_eval(const struct ad_node *nodes, size_t count, struct ad_node_index root, uint32_t *out);
struct ad_pair ad_lens_x(struct ad_pair source, uint32_t replacement);
struct ad_value ad_review_number(uint32_t value);
int ad_preview_number(struct ad_value value, uint32_t *out);

/* Original TLV: one-byte tag, one-byte length, exact borrowed payload. */
struct ad_evolving { unsigned tag; const unsigned char *payload; size_t length; };
int ad_evolve_read(const unsigned char *bytes, size_t length, struct ad_evolving *out);
int ad_evolve_write(struct ad_evolving value, unsigned char *bytes, size_t capacity, size_t *used);
enum ad_packet_error { AD_PACKET_OK, AD_PACKET_HEADER, AD_PACKET_KIND, AD_PACKET_LENGTH, AD_PACKET_ACK };
struct ad_packet { unsigned kind; const unsigned char *payload; size_t length; };
enum ad_packet_error ad_packet(const unsigned char *bytes, size_t length, struct ad_packet *out);
#endif
