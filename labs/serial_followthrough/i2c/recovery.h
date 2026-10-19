#ifndef SERIAL_I2C_RECOVERY_H
#define SERIAL_I2C_RECOVERY_H
#include "i2c.h"
/* Idle is observed without manufacturing a STOP. Active recovery is delegated. */
int ia_clear_begin(PiRecovery *,PiLines,unsigned exclusive,unsigned other_controllers,
                    uint32_t now,unsigned budget,unsigned half);
#endif
