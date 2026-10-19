#include <stdio.h>
#include <stdlib.h>
void en_note(const char *s){puts(s);}
int en_initialization(void);int en_transmit(void);int en_receive(void);int en_recovery(void);int en_drop_status(void);int en_boundaries(void);int en_failures(void);
int en_mapped(void);int en_chain(void);int en_publication(void);int en_visibility(void);int en_consumer(void);int en_order(void);
int main(int argc,char **argv){int (*tests[])(void)={en_initialization,en_transmit,en_receive,en_recovery,en_drop_status,en_boundaries,en_failures,en_mapped,en_chain,en_publication,en_visibility,en_consumer,en_order};if(argc!=2)return 2;int n=atoi(argv[1]);if(n<0 || n>=13)return 2;return tests[n]();}
