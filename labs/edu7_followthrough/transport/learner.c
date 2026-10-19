#include "transport.h"
int et_tx_open(EtTx * a0,unsigned a1,uint16_t a2,unsigned a3){(void)a0;(void)a1;(void)a2;(void)a3;return ET_STUB;}
int et_rx_open(EtRx * a0,unsigned a1,uint16_t a2,unsigned a3){(void)a0;(void)a1;(void)a2;(void)a3;return ET_STUB;}
int et_begin(EtTx * a0,uint16_t a1,const uint8_t * a2,size_t a3,uint32_t a4){(void)a0;(void)a1;(void)a2;(void)a3;(void)a4;return ET_STUB;}
int et_offer(EtTx * a0,uint32_t a1,uint8_t * a2,size_t a3,size_t * a4,uint32_t * a5){(void)a0;(void)a1;(void)a2;(void)a3;(void)a4;(void)a5;return ET_STUB;}
int et_accept(EtTx * a0,uint16_t a1,uint32_t a2,unsigned a3,uint32_t a4){(void)a0;(void)a1;(void)a2;(void)a3;(void)a4;return ET_STUB;}
int et_ack(EtTx * a0,const uint8_t * a1,size_t a2,uint32_t a3){(void)a0;(void)a1;(void)a2;(void)a3;return ET_STUB;}
int et_receive(EtRx * a0,const uint8_t * a1,size_t a2,uint32_t a3,uint8_t * a4,size_t a5,size_t * a6){(void)a0;(void)a1;(void)a2;(void)a3;(void)a4;(void)a5;(void)a6;return ET_STUB;}
int et_expire(EtRx * a0,uint32_t a1){(void)a0;(void)a1;return ET_STUB;}
int et_peek(const EtRx * a0,uint8_t * a1,size_t a2,size_t * a3){(void)a0;(void)a1;(void)a2;(void)a3;return ET_STUB;}
int et_release(EtRx * a0,uint16_t a1,uint16_t a2){(void)a0;(void)a1;(void)a2;return ET_STUB;}
