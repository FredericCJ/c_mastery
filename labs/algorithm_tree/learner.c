#include "tree.h"
void at_init(ATPool*p){(void)p;}
int at_insert(ATPool*p,int32_t k,unsigned f,ATWork*w){(void)p;(void)k;(void)f;(void)w;return ALGO_UNIMPLEMENTED;}
int at_delete(ATPool*p,int32_t k,unsigned f,ATWork*w){(void)p;(void)k;(void)f;(void)w;return ALGO_UNIMPLEMENTED;}
int at_contains(ATPool*p,int32_t k,unsigned f,ATWork*w){(void)p;(void)k;(void)f;(void)w;return 0;}
int at_certificate(const ATPool*p,ATCertificate*c){(void)p;(void)c;return ALGO_UNIMPLEMENTED;}
int at_cd_certificate(const CDTree*p,ATCertificate*c){(void)p;(void)c;return ALGO_UNIMPLEMENTED;}
int at_check(const ATCertificate*c,unsigned p,int*h){(void)c;(void)p;(void)h;return ALGO_UNIMPLEMENTED;}
/* Required prior artifacts are separate unsolved import placeholders. */
int algo_avl_insert(algo_avl*t,int32_t k){(void)t;(void)k;return ALGO_UNIMPLEMENTED;}
int algo_avl_contains(const algo_avl*t,int32_t k){(void)t;(void)k;return 0;}
void cd_tree_init(CDTree*t,unsigned b,unsigned m){(void)t;(void)b;(void)m;}
enum CDStatus cd_tree_insert(CDTree*t,int k,unsigned f,int*s){(void)t;(void)k;(void)f;(void)s;return CD_FAILED;}
enum CDStatus cd_tree_delete(CDTree*t,int k){(void)t;(void)k;return CD_FAILED;}
