#define main public_driver_main
#include "../../../labs/peripheral_depth/adc/tests.c"
#undef main
int main(void) {
  for (unsigned hz = 1000000; hz <= 180000000; hz += 1000000)
    for (unsigned sample = 0; sample < 8; sample++)
      for (unsigned n = 1; n <= 2; n++) {
        PuClock c = {hz, 1, 1, 2, 2, 0};
        PaConfig cfg = {sample, n, 0};
        PaTiming t = {0};
        int r = pa_timing(&c, &cfg, &t);
        printf("T %u %u %u %d %u %u %u %u\n", hz, sample, n, r, t.hz,
               t.total_cycles, t.conversion_us, t.prescaler);
      }
  const unsigned points[4][4] = {{0, 4095, 0, 3300},
                                 {64, 4032, 50, 3250},
                                 {1000, 3000, 500, 2500},
                                 {1, 3, 0, 3}};
  for (unsigned k = 0; k < 4; k++)
    for (unsigned code = 0; code < 4096; code++) {
      int32_t mv = 99;
      int r = pa_calibrate(code, points[k][0], points[k][1], points[k][2],
                           points[k][3], &mv);
      printf("C %u %u %d %d\n", k, code, r, (int)mv);
    }
  for (unsigned steps = 2; steps <= 128; steps++)
    for (unsigned i = 0; i < steps; i++) {
      unsigned v = 99;
      int r = pa_ramp(i, steps, 512, 3584, &v);
      printf("R %u %u %d %u\n", steps, i, r, v);
    }
  for (unsigned history = 0; history < 3125; history++) {
    unsigned digits = history, state = 0;
    for (unsigned i = 0; i < 5; i++) {
      unsigned symbol = digits % 5;
      digits /= 5;
      int value = 85 + (int)symbol * 5;
      int r = pa_threshold(&state, value, 90, 100);
      printf("H %u %u %u %d %u\n", history, i, symbol, r, state);
    }
  }
  for (unsigned n = 1; n <= 2; n++)
    for (unsigned flag = 0; flag < 64; flag++)
      for (unsigned late = 0; late < 2; late++)
        for (unsigned timeout = 0; timeout < 2; timeout++) {
          PaModel m = {0};
          PaAdc s = {0};
          PaIo io = io_for(&m);
          PuClock c = {64000000, 1, 1, 2, 2, 0};
          PaConfig cfg = {4, n, 1};
          uint16_t out[2] = {77, 88};
          int a = pa_open(&s, io, &c, &cfg, 0), b = pa_ready(&s, 0),
              z = pa_ready(&s, 4), d = pa_begin(&s, io, 1, 1, out, 4, 10);
          m.r[PA_SR] = flag;
          m.r[PA_DR] = 0xfabc;
          m.late_overrun = late;
          int r = pa_step(&s, io, 1, timeout ? 14 : 5);
          printf("S %u %u %u %u %d %d %d %d %d %u %u %u\n", n, flag, late,
                 timeout, a, b, z, d, r, out[0], out[1], m.reads);
        }
  const unsigned schedules[8][3] = {{2, 4, 1},   {2, 20, 1},  {2, 4, 30},
                                    {1, 4, 13},  {2, 20, 30}, {2, 4, 13},
                                    {2, 20, 13}, {1, 20, 30}};
  for (unsigned k = 0; k < 8; k++) {
    unsigned n = schedules[k][0], period = schedules[k][1],
             service = schedules[k][2];
    PaModel m = {0};
    PaAdc s = {0};
    PaIo io = io_for(&m);
    PuClock c = {64000000, 1, 1, 2, 2, 0};
    PaConfig cfg = {4, n, 1};
    uint16_t out[2] = {77, 88};
    if (pa_open(&s, io, &c, &cfg, 0) != PA_OK || pa_ready(&s, 0) != PA_BUSY ||
        pa_ready(&s, 4) != PA_OK ||
        pa_begin(&s, io, 1, 1, out, 4, 100) != PA_OK)
      return 2;
    unsigned active = 0, accepted = 0, attempted = 0, ignored = 0, next = 0,
             index = 0, t;
    int result = PA_BUSY;
    /* Explicit original event model: trigger, conversion-complete, CPU-service
       order. Busy triggers ignored, six us per channel. Not a silicon
       simulator. */
    for (t = 5; t < 100; t++) {
      if ((t - 5) % period == 0) {
        attempted++;
        if (active)
          ignored++;
        else {
          accepted++;
          active = 1;
          index = 0;
          next = t + 6;
        }
      }
      if (active && t == next) {
        complete(&m, 100 + index);
        if (++index == n)
          active = 0;
        else
          next += 6;
      }
      if (t % service == 0) {
        result = pa_step(&s, io, 1, t);
        if (result != PA_BUSY)
          break;
      }
    }
    printf("U %u %u %u %u %d %u %u %u %u\n", k, attempted, accepted, ignored,
           result, t, m.reads, out[0], out[1]);
  }
  return 0;
}
