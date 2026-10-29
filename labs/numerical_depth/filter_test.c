#include "checkpoint_numeric.h"
#include "test.h"

int main(void)
{
    double impulse[] = {1, 0, 0, 0}, step[] = {1, 1, 1, 1},
           fir[] = {.25, .5, .25, 0}, iir[] = {.5, .75, .875, .9375}, out[4];
    for (unsigned mask = 0; mask < 8; mask++)
    {
        struct cpn_double f;
        cpn_double_init(&f);
        double state = 0;
        size_t start = 0;
        for (size_t i = 0; i < 4; i++)
            if (i == 3 || (mask & (1u << i)))
            {
                size_t n = i + 1 - start;
                CHECK(cpn_double(&f, impulse + start, n, out + start,
                                 4 - start) == 0);
                CHECK(memcmp(out + start, fir + start, n * sizeof(double)) ==
                      0);
                CHECK(cpn_iir_half(&state, step + start, n, out + start,
                                   4 - start) == 0);
                CHECK(memcmp(out + start, iir + start, n * sizeof(double)) ==
                      0);
                start = i + 1;
            }
    }
    double state = 2;
    CHECK(nd_iir(&state, step, 4, .5, .5, out, 4) == ND_OK);
    double expected[] = {1.5, 1.25, 1.125, 1.0625};
    CHECK(memcmp(out, expected, sizeof out) == 0);
    state = 0;
    double snapshot[4];
    memcpy(snapshot, out, sizeof out);
    CHECK(nd_iir(&state, step, 4, .5, 1.01, out, 4) == ND_BAD && state == 0 &&
          memcmp(out, snapshot, sizeof out) == 0);
    for (size_t fail = 1; fail <= 4; fail++)
        for (int atomic = 0; atomic < 2; atomic++)
        {
            struct cpn_double f;
            cpn_double_init(&f);
            size_t done = 99;
            for (size_t i = 0; i < 4; i++)
                out[i] = 99;
            CHECK(cpn_transaction(&f, impulse, 4, out, 4, fail, atomic,
                                  &done) != 0);
            CHECK(done == (atomic ? 0 : fail - 1));
            for (size_t i = 0; i < done; i++)
                CHECK(out[i] == fir[i]);
            for (size_t i = done; i < 4; i++)
                CHECK(out[i] == 99);
            CHECK(cpn_transaction(&f, impulse + done, 4 - done, out + done,
                                  4 - done, 0, atomic, &done) == 0);
            CHECK(memcmp(out, fir, sizeof out) == 0);
        }
    double inplace[] = {1, 0, 0, 0};
    struct cpn_double f;
    cpn_double_init(&f);
    CHECK(cpn_double(&f, inplace, 4, inplace, 4) == 0 &&
          memcmp(inplace, fir, sizeof fir) == 0);
    state = 0;
    CHECK(cpn_iir_half(&state, step, 4, out, 3) != 0 && state == 0);
    int16_t coeff[] = {32, 64, 32}, history[3] = {0}, quantized[4],
            qe[] = {32, 64, 32, 0};
    for (size_t i = 0; i < 4; i++)
    {
        history[2] = history[1];
        history[1] = history[0];
        history[0] = i ? 0 : 127;
        CHECK(nd_qmac(coeff, history, 3, 7, quantized + i) == ND_OK);
        CHECK(quantized[i] == qe[i]);
    }
    int16_t half[] = {64, 64}, qstate = 0, qinput[2];
    int qexpected[] = {64, 96, 112, 120};
    for (size_t i = 0; i < 4; i++)
    {
        qinput[0] = 127;
        qinput[1] = qstate;
        CHECK(nd_qmac(half, qinput, 2, 7, &qstate) == ND_OK &&
              qstate == qexpected[i]);
    }
    double unstable = 1;
    for (size_t i = 0; i < 16; i++)
        unstable = 1.25 * unstable;
    CHECK(unstable > 35 && unstable < 36);
    printf("Bounded original unstable recurrence16steps=%a\n", unstable);
    puts("PASS actual checkpoint FIR/IIR producer all8 partitions, partial "
         "retry, atomic retry and nonzero-state adapter");
}
