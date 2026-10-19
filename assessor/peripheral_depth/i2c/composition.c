#define main pi_public_test_main
#include "../../../labs/peripheral_depth/i2c/tests.c"
#undef main
#include "firmware_io.h"
#include "serial_depth.h"
static Lines old_lines;
static uint32_t fwread(void *p, unsigned r) {
  (void)p;
  return line_read(&old_lines, r == FW_SCL ? PI_SCL : PI_SDA);
}
static void fwwrite(void *p, unsigned r, uint32_t v) {
  (void)p;
  line_release(&old_lines, r == FW_SCL ? PI_SCL : PI_SDA, v);
}
int main(void) {
  for (unsigned address = 8; address < 120; address++) {
    SDI2c sd = {0};
    PiI2c pi = {0};
    Model m = {0};
    PiIo io = {&m, rd, wr};
    PuClock c = {16000000, 1, 1, 1, 1, 0};
    uint8_t action = 0, wire = 0, out = 0;
    CHECK(sd_i2c_begin(&sd, 1, (uint8_t)address, 0x10, &action, &wire) ==
              SD_OK &&
          action == SD_START_WRITE);
    CHECK(!pi_open(&pi, io, &c));
    CHECK(!pi_begin(&pi, io, 1, 1, address, 0x10, 1, &out, 0, 100));
    CHECK(event(&pi, &m, PI_SB, 1) == PI_BUSY && m.tx[0] == wire);
    CHECK(sd_i2c_event(&sd, 1, SD_I2C_ACK, 0, &action, &wire) == SD_OK &&
          action == SD_WRITE_REGISTER && wire == 0x10);
    CHECK(event(&pi, &m, PI_ADDR, 2) == PI_BUSY);
    CHECK(event(&pi, &m, PI_TXE, 3) == PI_BUSY && m.tx[1] == wire);
    CHECK(sd_i2c_event(&sd, 1, SD_I2C_ACK, 0, &action, &wire) == SD_OK &&
          action == SD_RESTART_READ);
    CHECK(event(&pi, &m, PI_BTF, 4) == PI_BUSY);
    CHECK(event(&pi, &m, PI_SB, 5) == PI_BUSY && m.tx[2] == (2 * address + 1));
    CHECK(sd_i2c_event(&sd, 1, SD_I2C_ARB, 0, &action, &wire) ==
              SD_ARBITRATION_LOST &&
          action == SD_RELEASE);
    CHECK(event(&pi, &m, PI_ARLO, 6) == PI_ARBITRATION && !m.stop);
  }
  for (unsigned needed = 1; needed <= 9; needed++) {
    PiRecovery p = {0};
    Lines l = {{1, 1}, 0, needed, 0, 0};
    PiLines io = {&l, line_read, line_release};
    unsigned old = 99;
    int rc = PI_BUSY;
    old_lines = (Lines){{1, 1}, 0, needed, 0, 0};
    FwIo fw = {0, fwread, fwwrite};
    CHECK(sd_i2c_clear(1, 0, needed, &old) == SD_OK && old == needed);
    CHECK(fw_i2c_recover(fw, 9) == 0);
    CHECK(!pi_recover_begin(&p, io, 1, 0, 0, 200, 2));
    for (unsigned t = 0; t <= 200 && rc == PI_BUSY; t++)
      rc = pi_recover_step(&p, io, t);
    CHECK(rc == PI_OK && p.pulses == old);
  }
  /* Old primitives assume exclusive access; explicit second-master policy is
   * new. */
  {
    PiRecovery p = {0};
    Lines l = {{1, 1}, 0, 3, 0, 0};
    PiLines io = {&l, line_read, line_release};
    CHECK(pi_recover_begin(&p, io, 1, 1, 0, 100, 2) == PI_FORBIDDEN &&
          !l.writes);
  }
  puts("PI_COMPOSE_PASS 112 actual SDI2c address/arbitration paths;9 "
       "firmware/serial recovery comparisons; explicit multi-master refusal");
  return 0;
}
