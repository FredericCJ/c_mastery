#include "clinic.h"
#include <stdio.h>
#include <stdlib.h>
static void *allocate(void *c,size_t n){(void)c;return malloc(n);}
static void release(void *c,void *p){(void)c;free(p);}
int main(void)
{
    McAllocator a={NULL,allocate,release};
    for(unsigned history=0;history<81;++history){
        McOwner owner={0};unsigned code=history;
        for(unsigned step=0;step<4;++step){
            unsigned event=code%3;int status,out=99;code/=3;
            status=event==0?mc_acquire(&owner,a,7):event==1?mc_release(&owner):mc_read(&owner,&out);
            printf("%u,%u,%u,%d,%u,%u,%d\n",history,step,event,status,owner.value!=NULL,owner.releases,out);
        }
        if(owner.value && mc_release(&owner))return 2;
    }
    return 0;
}
