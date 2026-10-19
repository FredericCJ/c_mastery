#include "commit.h"
int at_same(const SCMachine*a,const SCMachine*b){(void)a;(void)b;return 0;}
enum ATStatus at_init(ATMachine*a,const SCMachine*b,unsigned c){(void)a;(void)b;(void)c;return AT_INVALID;}
enum ATStatus at_input(ATMachine*a,unsigned b,unsigned c){(void)a;(void)b;(void)c;return AT_INVALID;}
enum ATStatus at_begin(ATMachine*a,const SCInstruction*b,size_t c,ATTrace*d){(void)a;(void)b;(void)c;(void)d;return AT_INVALID;}
enum ATStatus at_poll(ATMachine*a,unsigned b,unsigned c,ATTrace*d){(void)a;(void)b;(void)c;(void)d;return AT_INVALID;}
enum ATStatus at_predict(const ATMachine*a,const SCInstruction*b,size_t c,unsigned d,ATSpec*e){(void)a;(void)b;(void)c;(void)d;(void)e;return AT_INVALID;}
enum ATStatus at_resolve(ATMachine*a,ATSpec*b,ATTrace*c){(void)a;(void)b;(void)c;return AT_INVALID;}
enum ATStatus at_retire(ATMachine*a,ATSpec*b,ATTrace*c){(void)a;(void)b;(void)c;return AT_INVALID;}
enum ATStatus at_discard(ATSpec*a){(void)a;return AT_INVALID;}
