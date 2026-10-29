#ifndef CMASTERY_ABSTRACTION_GENERIC_H
#define CMASTERY_ABSTRACTION_GENERIC_H
#include "session.h"

enum av_tag
{
    AV_INT,
    AV_TEXT,
    AV_ERROR,
    AV_OWNED
};

struct av_text
{
    const unsigned char *data;
    size_t length;
};

struct av_error
{
    int code;
    size_t offset;
};

struct av_owned
{
    unsigned char *data;
    size_t length;
    struct ac_allocate allocator;
};

struct as_value
{
    enum av_tag tag;

    union
    {
        int integer;
        struct av_text text;
        struct av_error error;
        struct av_owned owned;
    } as;
};
enum as_status av_text(const unsigned char *, size_t, struct as_value *);
enum as_status av_owned(const unsigned char *, size_t, struct ac_allocate,
                        struct as_value *);
enum as_status av_clone(const struct as_value *, struct as_value *);
enum as_status av_move(struct as_value *, struct as_value *, int fail_before);
enum as_status av_drop(struct as_value *);
enum as_status av_valid(const struct as_value *);
struct as_value av_parse(const unsigned char *, size_t);
enum as_status av_validate_fields(const struct av_text *, size_t,
                                  struct as_value results[4]);

struct as_ints
{
    int values[4];
    size_t count;
};
enum as_status as_int_push(struct as_ints *, int);
#define AS_GENERATE_PUSH(struct ac_name, Type)                            \
    struct ac_name                                                        \
    {                                                                     \
        Type values[4];                                                   \
        size_t count;                                                     \
    };                                                                    \
    static inline enum as_status struct ac_name##_push(struct ac_name *b, \
                                                       Type value)        \
    {                                                                     \
        if (!b || b->count > 4)                                           \
            return AS_INVALID;                                            \
        if (b->count == 4)                                                \
            return AS_FULL;                                               \
        b->values[b->count++] = value;                                    \
        return AS_OK;                                                     \
    }
AS_GENERATE_PUSH(AsGenerated, int)
#define as_generic_push(buffer, value) \
    _Generic((value), int: as_int_push)((buffer), (value))
typedef enum as_status (*as_copy_element_fn)(void *, const void *, void *);
typedef void (*as_drop_element_fn)(void *, void *);

struct as_elements
{
    size_t size, alignment;
    as_copy_element_fn copy;
    as_drop_element_fn drop;
    void *context;
};

struct as_erased
{
    void *data;
    size_t count, capacity;
    struct as_elements elements;
};
/* Actual caller storage and stage must be arrays of the descriptor's real type,
   mutually disjoint from source and each other. Alignment/size facts are caller
   supplied capabilities, not proof that an arbitrary address is valid. */
enum as_status as_erased_append(struct as_erased *, const void *, size_t,
                                void *stage, size_t stage_capacity,
                                size_t storage_alignment);
enum as_status as_erased_clear(struct as_erased *);
enum as_status as_value_copy_element(void *, const void *, void *);
void as_value_drop_element(void *, void *);
enum as_status as_int_copy_element(void *, const void *, void *);
void as_int_drop_element(void *, void *);
#endif
