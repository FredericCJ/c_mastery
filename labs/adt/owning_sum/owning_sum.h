#ifndef CMASTERY_AD_OWNING_SUM_H
#define CMASTERY_AD_OWNING_SUM_H
#include "adt.h"

enum ado_status
{
    ADO_OK,
    ADO_INVALID,
    ADO_NOMEM,
    ADO_UNIMPLEMENTED
};

enum ado_tag
{
    ADO_EMPTY,
    ADO_NUMBER,
    ADO_PAIR,
    ADO_TEXT
};

typedef int (*ado_clone_fn)(void *, const unsigned char *, size_t,
                            unsigned char **);
typedef void (*ado_drop_fn)(void *, unsigned char *, size_t);

struct ado_ops
{
    ado_clone_fn clone;
    ado_drop_fn drop;
    void *context;
};

struct ado_text
{
    unsigned char *bytes;
    size_t length;
    struct ado_ops ops;
};

struct ado_value
{
    enum ado_tag tag;

    union
    {
        uint32_t number;
        struct ad_pair pair;
        struct ado_text text;
    } data;
};

/* Init only unowned storage. All other controls are live, admitted owners. */
void ado_init(struct ado_value *);
enum ado_status ado_from_value(struct ado_value *, struct ad_value);
enum ado_status ado_text(struct ado_value *, const unsigned char *, size_t,
                         struct ado_ops);
enum ado_status ado_copy(struct ado_value *, const struct ado_value *);
enum ado_status ado_move(struct ado_value *, struct ado_value *);
void ado_drop(struct ado_value *);
enum ado_status ado_measure(const struct ado_value *, uint64_t *);
enum ado_status ado_number(const struct ado_value *, uint32_t *);
#endif
