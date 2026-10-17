#ifndef CMASTERY_ADT_H
#define CMASTERY_ADT_H
#include <stddef.h>
#include <stdint.h>

typedef enum { AD_EMPTY, AD_NUMBER, AD_PAIR } ADTag;
typedef struct { uint32_t x, y; } ADPair;
typedef struct { ADTag tag; union { uint32_t number; ADPair pair; } data; } ADValue;
int ad_measure(ADValue value, uint64_t *out);
int ad_number(ADValue value, uint32_t *out);
typedef struct { uint32_t value; } ADSeconds;
typedef struct { uint32_t value; } ADMillis;
int ad_milliseconds(ADSeconds value, ADMillis *out);
typedef struct { uint32_t bits; } ADFlags;
int ad_flags(uint32_t raw, ADFlags *out);
typedef struct { const unsigned char *bytes; size_t length, scalars; } ADText;
int ad_text(const unsigned char *bytes, size_t length, ADText *out);

/* Pool address must remain fixed, live, and never reused while handles survive. */
typedef struct { unsigned generation, live, retired; uint32_t value; } ADSlot;
typedef struct { ADSlot slot[4]; } ADPool;
typedef struct { const ADPool *owner; unsigned index, generation; } ADBrand;
typedef struct { unsigned index; } ADNodeIndex;
void ad_pool_init(ADPool *pool);
int ad_pool_acquire(ADPool *pool, uint32_t value, ADBrand *out);
int ad_pool_get(const ADPool *pool, ADBrand handle, uint32_t *out);
int ad_pool_release(ADPool *pool, ADBrand handle);

typedef struct { const uint32_t *data; size_t capacity, rows, columns, row_stride, column_stride; } ADView;
int ad_view(const uint32_t *data, size_t capacity, size_t rows, size_t columns,
            size_t row_stride, size_t column_stride, ADView *out);
int ad_view_get(ADView view, size_t row, size_t column, uint32_t *out);
typedef struct { unsigned kind; uint32_t value; ADNodeIndex left, right; } ADNode;
int ad_eval(const ADNode *nodes, size_t count, ADNodeIndex root, uint32_t *out);
ADPair ad_lens_x(ADPair source, uint32_t replacement);
ADValue ad_review_number(uint32_t value);
int ad_preview_number(ADValue value, uint32_t *out);

/* Original TLV: one-byte tag, one-byte length, exact borrowed payload. */
typedef struct { unsigned tag; const unsigned char *payload; size_t length; } ADEvolving;
int ad_evolve_read(const unsigned char *bytes, size_t length, ADEvolving *out);
int ad_evolve_write(ADEvolving value, unsigned char *bytes, size_t capacity, size_t *used);
typedef enum { AD_PACKET_OK, AD_PACKET_HEADER, AD_PACKET_KIND, AD_PACKET_LENGTH, AD_PACKET_ACK } ADPacketError;
typedef struct { unsigned kind; const unsigned char *payload; size_t length; } ADPacket;
ADPacketError ad_packet(const unsigned char *bytes, size_t length, ADPacket *out);
#endif
