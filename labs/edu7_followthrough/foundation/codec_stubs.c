/* Explicit incomplete prerequisites for an isolated export. Replace these with
 * the completed learner com_edu7.c producer; this is not a protocol solution. */
#include "com_edu7.h"
uint16_t edu_crc(const void *a,size_t b){(void)a;(void)b;return 0;}
int edu_frame_encode(const EduFrame *a,void *b,size_t c,size_t *d){(void)a;(void)b;(void)c;(void)d;return -1;}
int edu_frame_byte(EduFramer *a,uint8_t b,int c,EduFrame *d){(void)a;(void)b;(void)c;(void)d;return -1;}
int edu_network_encode(const EduDatagram *a,void *b,size_t *c){(void)a;(void)b;(void)c;return -1;}
int edu_network_decode(const void *a,size_t b,EduDatagram *c){(void)a;(void)b;(void)c;return -1;}
int edu_route(const EduDatagram *a,unsigned b,EduDatagram *c,unsigned *d,uint8_t *e){(void)a;(void)b;(void)c;(void)d;(void)e;return -1;}
int edu_record_encode(const EduRecord *a,void *b,size_t *c){(void)a;(void)b;(void)c;return -1;}
int edu_record_decode(const void *a,size_t b,EduRecord *c){(void)a;(void)b;(void)c;return -1;}
int edu_application(const EduRecord *a,int16_t *b,EduRecord *c){(void)a;(void)b;(void)c;return -1;}
