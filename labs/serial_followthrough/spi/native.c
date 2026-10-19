#include <stdio.h>
void ss_note(const char *s){puts(s);}
void ss_values(const char *s,unsigned a,unsigned b,unsigned c,unsigned d){printf("OBS %s %u %u %u %u\n",s,a,b,c,d);}
int ss_edges(void);int ss_queue(void);int ss_quiescence(void);int ss_irq_native(void);int ss_device(void);
int main(int argc,char **argv){if(argc!=2 || argv[1][1])return 42;switch(argv[1][0]){case '0':return ss_edges();case '1':return ss_queue();case '2':return ss_quiescence();case '3':return ss_irq_native();case '4':return ss_device();default:return 42;}}
