#ifndef CMASTERY_ALGORITHM_DEPTH_H
#define CMASTERY_ALGORITHM_DEPTH_H
#include <stddef.h>
#include <stdint.h>
#include "containers.h"

enum ad_status
{
    AD_OK,
    AD_INVALID,
    AD_SPACE,
    AD_OVERFLOW,
    AD_MISSING,
    AD_INCOMPLETE,
    AD_READY,
    AD_FAILED
};

struct ad_record
{
    int32_t key;
    unsigned label;
};

struct ad_work
{
    uint64_t comparisons, writes;
    unsigned max_depth, fallbacks;
};
enum ad_status ad_bounds(const int32_t *, size_t, int32_t, unsigned, size_t *,
                         size_t *, size_t *);
enum ad_status ad_linear(const int32_t *, size_t, int32_t, size_t *);
enum ad_status ad_partition(struct ad_record *, size_t, const int32_t *,
                            unsigned, struct ad_record *, size_t, size_t *,
                            struct ad_work *);
enum ad_status ad_insertion(struct ad_record *, size_t, struct ad_work *);
enum ad_status ad_merge(struct ad_record *, size_t, size_t, struct ad_record *,
                        size_t, struct ad_work *);
enum ad_status ad_merge_sort(struct ad_record *, size_t, struct ad_record *,
                             size_t, struct ad_work *);
enum ad_status ad_heap_sort(struct ad_record *, size_t, struct ad_work *);
enum ad_status ad_three_way(struct ad_record *, size_t, const int32_t *,
                            size_t *, size_t *, struct ad_work *);
enum ad_status ad_quick_sort(struct ad_record *, size_t, unsigned, unsigned,
                             struct ad_work *);
enum ad_status ad_select(struct ad_record *, size_t, size_t, unsigned,
                         int32_t *, size_t *, size_t *, struct ad_work *);
enum ad_status ad_counting(struct ad_record *, size_t, struct ad_record *,
                           size_t, struct ad_work *);
enum ad_status ad_radix(struct ad_record *, size_t, unsigned,
                        struct ad_record *, size_t, struct ad_work *);

struct ad_traversal
{
    unsigned order[8], finish[8], root[8], count, finished, roots, peak;
    uint32_t distance[8];
    uint64_t edges;
};

struct ad_frame
{
    unsigned vertex, next;
};
enum ad_status ad_bfs(const struct cd_csr *, unsigned, unsigned, unsigned *,
                      size_t, struct ad_traversal *);
enum ad_status ad_dfs(const struct cd_csr *, unsigned, unsigned,
                      struct ad_frame *, size_t, struct ad_traversal *);
enum ad_status ad_reachable_stack(const struct cd_csr *, unsigned, unsigned *,
                                  size_t, unsigned *);
enum ad_status ad_topological(const struct cd_csr *, unsigned[8]);
enum ad_status ad_dijkstra(const struct cd_csr *, unsigned, uint64_t,
                           uint64_t[8], unsigned[8]);

struct ad_interval
{
    int start, end;
    unsigned label;
};
enum ad_status ad_intervals(const struct ad_interval *, size_t, unsigned[8],
                            size_t *);

struct ad_item
{
    unsigned weight;
    uint32_t value;
};
enum ad_status ad_knapsack(const struct ad_item *, size_t, unsigned, unsigned,
                           unsigned, uint32_t *);

struct ad_matcher
{
    unsigned char pattern[16];
    size_t length, matched, failure[16];
    uint64_t consumed, comparisons, fallbacks;
};
enum ad_status ad_match_naive(const unsigned char *, size_t,
                              const unsigned char *, size_t, uint64_t *, size_t,
                              size_t *);
enum ad_status ad_match_init(struct ad_matcher *, const unsigned char *,
                             size_t);
enum ad_status ad_match_feed(struct ad_matcher *, const unsigned char *, size_t,
                             uint64_t *, size_t, size_t *);
enum ad_status ad_match_hash(const unsigned char *, size_t,
                             const unsigned char *, size_t, uint64_t *, size_t,
                             size_t *, uint64_t *);

struct ad_span
{
    size_t offset, length;
};
enum ad_status ad_byte_copy(unsigned char *, size_t, const unsigned char *,
                            size_t);
enum ad_status ad_byte_move(unsigned char *, size_t, size_t, size_t, size_t);
enum ad_status ad_byte_compare(const unsigned char *, size_t,
                               const unsigned char *, size_t, int *);
enum ad_status ad_byte_find(const unsigned char *, size_t, unsigned char,
                            size_t *);
enum ad_status ad_byte_split(const unsigned char *, size_t, unsigned char,
                             unsigned, struct ad_span *, size_t, size_t *);
enum ad_status ad_byte_trim(const unsigned char *, size_t, struct ad_span *);
enum ad_status ad_byte_trim_in_place(unsigned char *, size_t, size_t *);
enum ad_status ad_parse_i32(const unsigned char *, size_t, unsigned, unsigned,
                            unsigned, int32_t *, size_t *);
enum ad_status ad_parse_u32(const unsigned char *, size_t, unsigned, unsigned,
                            unsigned, uint32_t *, size_t *);
enum ad_status ad_format_i32(int32_t, unsigned, unsigned char *, size_t,
                             size_t *);

struct ad_wire_record
{
    unsigned char flags;
    uint16_t count;
    int32_t value;
};
enum ad_status ad_record_encode(const struct ad_wire_record *, unsigned char *,
                                size_t);
enum ad_status ad_record_decode(const unsigned char *, size_t,
                                struct ad_wire_record *);
enum ad_status ad_record_extensions(const unsigned char *, size_t,
                                    struct ad_wire_record *, unsigned char *,
                                    unsigned *);
unsigned char ad_crc8(const unsigned char *, size_t, unsigned, unsigned char,
                      unsigned char);
unsigned char ad_crc8_table(const unsigned char *, size_t);

struct ad_framer
{
    unsigned char bytes[8];
    unsigned mode, state, used, expected, escaped;
};
enum ad_status ad_frame_init(struct ad_framer *, unsigned);
enum ad_status ad_frame_feed(struct ad_framer *, const unsigned char *, size_t,
                             size_t *);
enum ad_status ad_frame_peek(const struct ad_framer *, const unsigned char **,
                             size_t *);
enum ad_status ad_frame_ack(struct ad_framer *);
enum ad_status ad_frame_end(struct ad_framer *);
void ad_frame_cancel(struct ad_framer *);
enum ad_status ad_nested_frames(const unsigned char *, size_t, unsigned, size_t,
                                unsigned char *, size_t, size_t *);

enum ad_token_kind
{
    AD_T_END,
    AD_T_NUMBER,
    AD_T_NAME,
    AD_T_PLUS,
    AD_T_STAR,
    AD_T_POWER,
    AD_T_LEFT,
    AD_T_RIGHT
};

struct ad_token
{
    enum ad_token_kind kind;
    size_t offset, length;
    int32_t value;
    unsigned char name;
};

struct ad_diagnostic
{
    size_t offset;
    enum ad_status status;
};
enum ad_status ad_lex(const unsigned char *, size_t, struct ad_token *, size_t,
                      size_t *, struct ad_diagnostic *);
enum ad_status ad_token_copy(const unsigned char *, size_t, struct ad_token,
                             unsigned char *, size_t, size_t *);

struct ad_node
{
    enum ad_token_kind kind;
    unsigned left, right;
    int32_t value;
    unsigned char name;
    size_t offset;
};

struct ad_ast
{
    struct ad_node nodes[32];
    unsigned count, root;
};
enum ad_status ad_parse(const struct ad_token *, size_t, unsigned, unsigned,
                        unsigned, unsigned, struct ad_ast *,
                        struct ad_diagnostic *);
typedef enum ad_status (*ADLookup)(void *, unsigned char, int32_t *);
enum ad_status ad_evaluate(const struct ad_ast *, ADLookup, void *, int32_t *,
                           struct ad_diagnostic *);
enum ad_status ad_rle_encode(const unsigned char *, size_t, unsigned char *,
                             size_t, size_t *);
enum ad_status ad_rle_decode(const unsigned char *, size_t, unsigned,
                             unsigned char *, size_t, size_t *);
enum ad_status ad_tagged_encode(const unsigned char *, size_t, unsigned char *,
                                size_t, size_t *);
enum ad_status ad_tagged_decode(const unsigned char *, size_t, unsigned char *,
                                size_t, size_t *);

struct ad_rle_stream
{
    unsigned remaining, waiting, run, last_count, last_value, have_last, failed,
        canonical;
    unsigned char byte;
    size_t produced;
};
enum ad_status ad_rle_stream_init(struct ad_rle_stream *, unsigned);
enum ad_status ad_rle_stream_feed(struct ad_rle_stream *, const unsigned char *,
                                  size_t, size_t *, unsigned char *, size_t,
                                  size_t *);
enum ad_status ad_rle_stream_end(struct ad_rle_stream *);
#endif
