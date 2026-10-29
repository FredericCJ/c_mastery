#ifndef CMASTERY_DOD_DICTIONARY_H
#define CMASTERY_DOD_DICTIONARY_H
#include "dod.h"

struct dh_key
{
    unsigned char bytes[2];
};

struct dh_dictionary
{
    void *context;
    uint32_t (*hash)(void *, struct dh_key);
    int (*equal)(void *, struct dh_key, struct dh_key);
};

struct dh_map_node
{
    unsigned bitmap, count, children[4];
    struct dh_key keys[8];
    int values[8];
};

struct dh_map
{
    struct dh_map_node nodes[513];
    unsigned used, fail_at, attempts;
    struct dh_dictionary dictionary;
};

struct dh_metrics
{
    unsigned visited, comparisons, clones;
};

int d_map_init(struct dh_map *, struct dh_dictionary);
int d_map_update(struct dh_map *, unsigned root, struct dh_key, int value,
                 unsigned *out, struct dh_metrics *);
int d_map_delete(struct dh_map *, unsigned root, struct dh_key, unsigned *out,
                 struct dh_metrics *);
int d_map_get(const struct dh_map *, unsigned root, struct dh_key, int *out,
              struct dh_metrics *);
#endif
