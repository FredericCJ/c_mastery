#include "test.h"
#include <stdint.h>

#define POISON -99

/*
 * a is 2x3 inside a buffer with one pad element per row (row stride 4),
 * b is 3x2 with a pad element after each pair (row stride 3). Neither is
 * contiguous. 99 marks padding so a stray read or write changes a result.
 */
static double a_data[] = {1, 2, 3, 99, 4, 5, 6, 99};
static double b_data[] = {7, 8, 99, 9, 10, 99, 11, 12, 99};

static struct nd_view a_view(void)
{
    struct nd_view view = {a_data, 8, 2, 3, 4, 1};
    return view;
}

static struct nd_view b_view(void)
{
    struct nd_view view = {b_data, 9, 3, 2, 3, 1};
    return view;
}

/* All three traversal methods must give identical products. */
static void check_padded_product(void)
{
    double c_data[6];
    struct nd_view c = {c_data, 6, 2, 2, 3, 1};
    for (unsigned method = 0; method < 3; method++)
    {
        for (size_t i = 0; i < 6; i++)
        {
            c_data[i] = POISON;
        }
        CHECK(nd_matrix(a_view(), b_view(), c, method) == ND_OK);
        /* Only the 2x2 logical block is written; the pad column is not. */
        CHECK(c_data[0] == 58 && c_data[1] == 64 && c_data[3] == 139 &&
              c_data[4] == 154 && c_data[2] == POISON && c_data[5] == POISON);
    }
}

static void check_view_validation(void)
{
    double c_data[6] = {0};
    struct nd_view c = {c_data, 6, 2, 2, 3, 1};
    struct nd_view bad = a_view();
    bad.extent = 6;
    CHECK(nd_matrix(bad, b_view(), c, 0) == ND_BAD);
    bad = a_view();
    bad.rs = SIZE_MAX;
    CHECK(nd_view(bad) == ND_BAD);
    bad = a_view();
    bad.rs = 1;
    CHECK(nd_view(bad) == ND_BAD);
}

static void check_transposed_view(void)
{
    struct nd_view transposed = {a_data, 8, 3, 2, 1, 4};
    CHECK(nd_view(transposed) == ND_OK);
    double d_data[9];
    struct nd_view d = {d_data, 9, 3, 3, 3, 1};
    CHECK(nd_matrix(transposed, a_view(), d, 2) == ND_OK && d_data[0] == 17 &&
          d_data[8] == 45);
}

/* One-row views: dot product, then in-place scaling that spares the pad. */
static void check_dot_and_scale(void)
{
    struct nd_view row = {a_data, 8, 1, 3, 4, 1};
    double dot;
    CHECK(nd_dot(row, row, &dot) == ND_OK && dot == 14);
    CHECK(nd_scale(row, 2) == ND_OK && a_data[0] == 2 && a_data[2] == 6 &&
          a_data[3] == 99);
}

/* An output that shares elements with an input is rejected before any write. */
static void check_overlap_rejected(void)
{
    struct nd_view overlap = {a_data, 8, 2, 2, 4, 1};
    double saved[8];
    memcpy(saved, a_data, sizeof a_data);
    CHECK(nd_matrix(a_view(), b_view(), overlap, 0) == ND_BAD &&
          memcmp(saved, a_data, sizeof a_data) == 0);
}

/* A 3x1 times 1x1 product exercises edges that do not divide the block. */
static void check_ragged_block(void)
{
    double tall[] = {2, 3, 4};
    double wide[] = {5};
    double out[3];
    CHECK(nd_matrix((struct nd_view){tall, 3, 3, 1, 1, 1},
                    (struct nd_view){wide, 1, 1, 1, 1, 1},
                    (struct nd_view){out, 3, 3, 1, 1, 1}, 2) == ND_OK &&
          out[2] == 20);
}

int main(void)
{
    check_padded_product();
    check_view_validation();
    check_transposed_view();
    check_dot_and_scale();
    check_overlap_rejected();
    check_ragged_block();
    puts(
        "PASS padded, transposed, ragged block, overflow, overlap and in-place "
        "scale");
}
