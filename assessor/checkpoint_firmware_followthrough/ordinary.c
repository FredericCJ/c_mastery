#include <stdio.h>
void cp_test_firmware_ring(void);
void cp_test_firmware_tx(void);
void cp_test_firmware_clock(void);
int main(void){cp_test_firmware_ring();cp_test_firmware_tx();cp_test_firmware_clock();puts("P4_ORDINARY_PASS");}
