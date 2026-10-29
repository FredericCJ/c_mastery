#include "ownership.h"
#include <stdio.h>
int main(void) {
 size_t extent,a,n,b,m;int aw,bw;
 while(scanf("%zu %zu %zu %d %zu %zu %d",&extent,&a,&n,&aw,&b,&m,&bw)==7) {
  struct ow_borrows borrows={.extent=extent};struct ow_token x,y;
  enum ow_status first=ow_borrow(&borrows,a,n,aw,&x);
  enum ow_status second=ow_borrow(&borrows,b,m,bw,&y);
  printf("%d %d\n",(int)first,(int)second);
 }
 return ferror(stdin)?2:0;
}
