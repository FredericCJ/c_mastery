#include "ownership.h"
#include <stdio.h>
int main(void) {
 size_t extent,a,n,b,m;int aw,bw;
 while(scanf("%zu %zu %zu %d %zu %zu %d",&extent,&a,&n,&aw,&b,&m,&bw)==7) {
  OwBorrows borrows={.extent=extent};OwToken x,y;
  OwStatus first=ow_borrow(&borrows,a,n,aw,&x);
  OwStatus second=ow_borrow(&borrows,b,m,bw,&y);
  printf("%d %d\n",(int)first,(int)second);
 }
 return ferror(stdin)?2:0;
}
