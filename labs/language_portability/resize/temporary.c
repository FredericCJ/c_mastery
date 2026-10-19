#include <stdio.h>
struct Record {unsigned char tag[3];long value;};
static struct Record make(void){struct Record r={{1,2,3},42};return r;}
int main(void){
    struct Record r={{4,5,6},7};
    /* Comparison only within the full expression; no escaped temporary borrow. */
    int same=((void)0,r).tag==r.tag;
    printf("temporary_address_equal=%d returned_value=%ld\n",same,make().value);
    return make().value!=42;
}
