#ifndef CORPUS_ESCAPE_H
#define CORPUS_ESCAPE_H
#include <stddef.h>

enum
{
    ES_OK,
    ES_INVALID,
    ES_RANGE,
    ES_SYNTAX,
    ES_CAPACITY
};

enum
{
    ES_PLAIN = 64,
    ES_ENCODED = 384
};

struct es_text
{
    unsigned char bytes[ES_ENCODED];
    size_t length;
};

/* Live disjoint output/input; explicit lengths, no string terminator required.
   On failure every output byte is unchanged. See CONTRACTS.md version1. */
int es_encode(const unsigned char *, size_t, size_t, struct es_text *);
int es_decode(const unsigned char *, size_t, size_t, struct es_text *);
/* Restricted generated-HTML-text dialect -> canonical named-entity dialect.
   This validates a language, not provenance, authorization or arbitrary HTML. */
int es_recode_html(const unsigned char *, size_t, size_t, struct es_text *);
#endif
