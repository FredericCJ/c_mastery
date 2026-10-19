#include "board.h"
#include "stm32f4xx_hal.h"
#include "test.h"
typedef struct {
  uint32_t words[10];
} Model;
static uint32_t read_word(void *p, uintptr_t a) {
  Model *m = p;
  return m->words[a / 4];
}
static void write_word(void *p, uintptr_t a, uint32_t v) {
  Model *m = p;
  m->words[a / 4] = v;
}
int main(void) {
  GPIO_TypeDef vendor = {0};
  Model minimal = {0};
  vendor.MODER = minimal.words[0] = 0xa8000000u;
  vendor.OSPEEDR = minimal.words[2] = 0x0c000000u;
  vendor.PUPDR = minimal.words[3] = 0x64000000u;
  GPIO_InitTypeDef config = {GPIO_PIN_2, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                             GPIO_SPEED_FREQ_LOW, 0};
  HAL_GPIO_Init(&vendor, &config);
  ArBoard b = {&minimal, read_word, write_word, 0, 2, 0};
  CHECK(ar_board_init(&b) == AR_OK);
  CHECK(vendor.MODER == minimal.words[0] && vendor.OTYPER == minimal.words[1] &&
        vendor.OSPEEDR == minimal.words[2] && vendor.PUPDR == minimal.words[3]);
  for (unsigned on = 0; on < 2; ++on) {
    HAL_GPIO_WritePin(&vendor, GPIO_PIN_2, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
    CHECK(ar_board_set(&b, on) == AR_OK && vendor.BSRR == minimal.words[6]);
  }
  config.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(&vendor, &config);
  b.pin = 3;
  b.active_low = 1;
  CHECK(ar_board_init(&b) == AR_OK && vendor.MODER == minimal.words[0]);
  HAL_GPIO_WritePin(&vendor, GPIO_PIN_3, GPIO_PIN_RESET);
  CHECK(ar_board_set(&b, 1) == AR_OK && vendor.BSRR == minimal.words[6]);
  printf("PASS actual HAL GPIO selected output model sizeofGPIO=%zu compared "
         "init,set,reset,two serialized clients\n",
         sizeof vendor);
}
