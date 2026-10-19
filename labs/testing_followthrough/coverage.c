#include "coverage.h"
int cv_allow(Cv *c,size_t n,int a){(void)c;(void)n;(void)a;return 0;}
int cv_push(Cv *c,int x){(void)c;(void)x;return 0;}
void cv_cleanup(Cv *c){(void)c;}
int cv_decision(int a,int b){(void)a;(void)b;return 0;}
int cv_changed(int a,int b,int c){(void)a;(void)b;(void)c;return 0;}
