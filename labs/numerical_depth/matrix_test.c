#include "test.h"
#include <stdint.h>

int main(void)
{
    double a[] = {1, 2, 3, 99, 4, 5, 6, 99},
           b[] = {7, 8, 99, 9, 10, 99, 11, 12, 99}, c[6];
    struct nd_view av = {a, 8, 2, 3, 4, 1}, bv = {b, 9, 3, 2, 3, 1},
                   cv = {c, 6, 2, 2, 3, 1};
    for (unsigned m = 0; m < 3; m++)
    {
        for (size_t i = 0; i < 6; i++)
            c[i] = -99;
        CHECK(nd_matrix(av, bv, cv, m) == ND_OK);
        CHECK(c[0] == 58 && c[1] == 64 && c[3] == 139 && c[4] == 154 &&
              c[2] == -99 && c[5] == -99);
    }
    struct nd_view bad = av;
    bad.extent = 6;
    CHECK(nd_matrix(bad, bv, cv, 0) == ND_BAD);
    bad = av;
    bad.rs = SIZE_MAX;
    CHECK(nd_view(bad) == ND_BAD);
    bad = av;
    bad.rs = 1;
    CHECK(nd_view(bad) == ND_BAD);
    struct nd_view trans = {a, 8, 3, 2, 1, 4};
    CHECK(nd_view(trans) == ND_OK);
    double d[9];
    struct nd_view dv = {d, 9, 3, 3, 3, 1};
    CHECK(nd_matrix(trans, av, dv, 2) == ND_OK && d[0] == 17 && d[8] == 45);
    struct nd_view row = {a, 8, 1, 3, 4, 1};
    double dot;
    CHECK(nd_dot(row, row, &dot) == ND_OK && dot == 14);
    CHECK(nd_scale(row, 2) == ND_OK && a[0] == 2 && a[2] == 6 && a[3] == 99);
    struct nd_view overlap = {a, 8, 2, 2, 4, 1};
    double saved[8];
    memcpy(saved, a, sizeof a);
    CHECK(nd_matrix(av, bv, overlap, 0) == ND_BAD &&
          memcmp(saved, a, sizeof a) == 0);
    double tall[] = {2, 3, 4}, wide[] = {5}, out[3];
    CHECK(nd_matrix((struct nd_view){tall, 3, 3, 1, 1, 1},
                    (struct nd_view){wide, 1, 1, 1, 1, 1},
                    (struct nd_view){out, 3, 3, 1, 1, 1}, 2) == ND_OK &&
          out[2] == 20);
    puts(
        "PASS padded, transposed, ragged block, overflow, overlap and in-place "
        "scale");
}
