/* Separate ACSL reference; mathematical function is a finite expression, no assumed axiom. */
/*@ logic integer sum4{L}(unsigned *a,integer n) =
      (n>0?a[0]:0)+(n>1?a[1]:0)+(n>2?a[2]:0)+(n>3?a[3]:0);
*/
/*@ requires n <= 4;
    requires \valid_read(a+(0..n-1));
    requires \forall integer k; 0 <= k < n ==> a[k] <= 4294967295;
    assigns \nothing;
    ensures \result == sum4(a,n);
    ensures \result <= 10*n;
*/
unsigned proved_sum(const unsigned *a,unsigned n){
 unsigned i=0,acc=0;
 /*@ loop invariant 0 <= i <= n;
     loop invariant acc == sum4(a,i);
     loop invariant 0 <= acc <= 10*i;
     loop assigns i,acc;
     loop variant n-i;
 */
 while(i<n){acc+=a[i];++i;}
 return acc;
}
/* Public entry has live four-element storage. Short logical extent is checked;
   arbitrary invalid C pointer/lifetime is never admitted by this contract. */
/*@ requires \valid_read(a+(0..3));
    requires \valid(out);
    requires \separated(a+(0..3),out);
    assigns *out;
    ensures \result == 0 ==> *out == sum4(a,n);
    ensures \result != 0 ==> *out == \old(*out);
*/
int checked_sum(const unsigned *a,unsigned extent,unsigned n,unsigned *out){
 if(n>4||n>extent)return 1;
 /*@ loop invariant 0 <= j <= n;
     loop invariant \forall integer k; 0 <= k < j ==> a[k] <= 4294967295;
     loop assigns j;
     loop variant n-j;
 */
 for(unsigned j=0;j<n;++j)if(a[j]>10)return 2;
 *out=proved_sum(a,n);return 0;
}
/*@ requires x <= 10; assigns \nothing; ensures \result <= 9; */
unsigned false_bound(unsigned x){return x;}
