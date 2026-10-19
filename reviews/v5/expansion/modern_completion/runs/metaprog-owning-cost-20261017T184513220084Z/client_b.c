#include "owning_ring.h"
int probe(void);int main(void){MpoWordsRing r;MpoWordsOps o={0};return !(probe()&&mpo_words_init(&r,o)==MPO_INVALID);}
