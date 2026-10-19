#include <stddef.h>
#include <stdio.h>
static int probe(size_t n) {
    size_t i;int sum=0;
    if(n==0 || n>16)return -1;
    {int a[n];for(i=0;i<n;i++){a[i]=(int)i;sum+=a[i];}}
    return sum;
}
int main(void){if(probe(0)!=-1 || probe(17)!=-1 || probe(1)!=0 || probe(16)!=120)return 1;puts("LP_VLA positive_bounds");return 0;}
