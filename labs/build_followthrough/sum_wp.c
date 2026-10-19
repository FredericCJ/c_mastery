/* ACSL source is a separate trusted inner kernel, not public rejection behavior. */
/*@ requires 0 <= n <= 4;
    requires \valid_read(a+(0..n-1));
    requires \forall integer k; 0 <= k < n ==> 0 <= a[k] <= 10;
    assigns \nothing;
    ensures 0 <= \result <= 10*n;
*/
unsigned inner_sum(const unsigned *a,unsigned n){
 unsigned i=0,acc=0;
 /*@ loop invariant 0 <= i <= n;
     loop invariant 0 <= acc <= 10*i;
     loop assigns i,acc;
     loop variant n-i;
 */
 while(i<n){acc+=a[i];++i;}
 return acc;
}
/* Deliberately false goal preserves an investigation target. */
/*@ requires 0 <= x <= 10; assigns \nothing; ensures \result <= 9; */
unsigned false_bound(unsigned x){return x;}
