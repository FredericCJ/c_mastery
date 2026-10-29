#include "protocols.h"
static int run(struct pt_thread *p,unsigned ready){
 PT_BEGIN(p);p->sum=17;
 PT_WAIT(p,1,ready);
 p->sum+=12;PT_END(p);
}
int main(void){struct pt_thread p={0};if(run(&p,0)!=PT_PENDING||p.sum!=17)return 1;if(run(&p,1)!=PT_DONE||p.sum!=29)return 1;return 0;}
