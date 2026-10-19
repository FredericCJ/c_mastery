#include "checkpoint_numeric.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CP_CHECK: %s:%d %s\n",__FILE__,__LINE__,#x);exit(1);}}while(0)
void cp_test_numeric(void) {
    const int32_t x[]={0,4,8,4,0};const int64_t expected[]={0,4,16,24,16};
    CPNExact e;cpn_exact_init(&e);int64_t numerator[5];
    CHECK(cpn_exact(&e,x,5,numerator,5)==CPN_OK);
    for(size_t i=0;i<5;++i)CHECK(numerator[i]==expected[i]);
    const int32_t extrema[]={INT32_MAX,INT32_MAX,INT32_MIN,INT32_MIN};
    cpn_exact_init(&e);CHECK(cpn_exact(&e,extrema,4,numerator,5)==CPN_OK);
    for(size_t i=0;i<4;++i){int64_t reference=extrema[i];if(i)reference+=2*(int64_t)extrema[i-1];if(i>1)reference+=extrema[i-2];CHECK(numerator[i]==reference);}
    /* Independent convolution over the whole input, compared with recurrence and every split. */
    for(int a=-2;a<=2;++a)for(int b=-2;b<=2;++b)for(int c=-2;c<=2;++c) {
        double input[]={a,b,c,0,0},out[5];int64_t ref[5];int32_t ints[]={a,b,c,0,0};
        for(size_t i=0;i<5;++i){ref[i]=ints[i];if(i)ref[i]+=2*(int64_t)ints[i-1];if(i>1)ref[i]+=ints[i-2];}
        for(size_t split=0;split<=5;++split) {
            CPNDouble s;cpn_double_init(&s);
            CHECK(cpn_double(&s,input,split,out,split)==CPN_OK);
            CHECK(cpn_double(&s,input+split,5-split,out+split,5-split)==CPN_OK);
            for(size_t i=0;i<5;++i)CHECK(out[i]==(double)ref[i]/4.0);
            CHECK(s.previous==0 && s.older==0);
        }
    }
    volatile double big=9007199254740992.0,one=1.0;
    volatile double first=big+one;volatile double second=big-big;
    CHECK(first-big==0.0 && second+one==1.0);
    /* This cancellation-sensitive order difference is observed, not a tolerance waiver. */
    double input[]={-4.0*9007199254740992.0,2.0*9007199254740992.0,4.0};
    CPNDouble s;double out[3];cpn_double_init(&s);CHECK(cpn_double(&s,input,3,out,3)==CPN_OK);
    volatile double reordered=(0.5*input[1]+0.25*input[0])+0.25*input[2];
    CHECK(out[2]==0.0 && reordered==1.0);
}
void cp_test_numeric_policy(void) {
    CPNDouble s;cpn_double_init(&s);double input[]={4,8,4,0},out[]={99,99,99,99};
    CHECK(cpn_double(&s,input,1,out,1)==CPN_OK && s.previous==4);
    CHECK(cpn_double(&s,input+1,3,out,2)==CPN_CAPACITY && s.previous==4 && s.older==0 && out[0]==1);
    double bad[]={8,NAN,4};
    CHECK(cpn_double(&s,bad,3,out,4)==CPN_INVALID && s.previous==4 && out[0]==1);
    bad[1]=INFINITY;CHECK(cpn_double(&s,bad,3,out,4)==CPN_INVALID && s.previous==4);
    bad[1]=1e101;CHECK(cpn_double(&s,bad,3,out,4)==CPN_INVALID && s.previous==4);
    double overlap[]={4,8,4,0,99};cpn_double_init(&s);
    CHECK(cpn_double(&s,overlap,4,overlap+1,4)==CPN_OVERLAP && s.previous==0 && overlap[1]==8);
    CHECK(cpn_double(&s,overlap+1,4,overlap,4)==CPN_OVERLAP && s.previous==0 && overlap[1]==8);
    CHECK(cpn_double(&s,overlap,4,overlap,4)==CPN_OK);
    const double expected[]={1,4,6,4};for(size_t i=0;i<4;++i)CHECK(overlap[i]==expected[i]);
    CHECK(s.previous==0 && s.older==4);
    CHECK(cpn_double(&s,NULL,0,NULL,0)==CPN_OK && s.older==4);
    CHECK(cpn_double(&s,input,CPN_BLOCK+1,out,CPN_BLOCK+1)==CPN_INVALID);
    CPNExact e;cpn_exact_init(&e);int32_t sample=4;int64_t fraction=99;
    CHECK(cpn_exact(&e,&sample,1,&fraction,0)==CPN_CAPACITY && e.previous==0 && fraction==99);
}
void cp_test_numeric_transaction(void){
    double input[]={0,4,8,4,0},expected[]={0,1,4,6,4};
    for(int atomic=0;atomic<=1;++atomic)for(size_t fail=1;fail<=6;++fail){
        CPNDouble state={0,0};double out[]={99,99,99,99,99};size_t committed=77;
        CHECK(cpn_transaction(&state,input,5,out,5,fail,atomic,&committed)==(fail<=5?CPN_FAILED:CPN_OK));
        size_t count=fail<=5?(atomic?0:fail-1):5;CHECK(committed==count);
        CHECK(state.previous==(count?input[count-1]:0) && state.older==(count>1?input[count-2]:0));
        for(size_t i=0;i<5;++i)CHECK(out[i]==(i<count?expected[i]:99));
        CHECK(cpn_transaction(&state,input+count,5-count,out+count,5-count,0,atomic,&committed)==CPN_OK && committed==5-count);
        for(size_t i=0;i<5;++i)CHECK(out[i]==expected[i]);
    }
    for(int numerator=-19;numerator<=19;++numerator){
        int32_t actual=99;CHECK(cpn_round_even(numerator,&actual)==CPN_OK);
        int best=0,best_distance=100;
        for(int candidate=-6;candidate<=6;++candidate){int distance=abs(4*candidate-numerator);if(distance<best_distance || (distance==best_distance && candidate%2==0)){best=candidate;best_distance=distance;}}
        CHECK(actual==best);
    }
    int32_t rounded=123;CHECK(cpn_round_even(INT64_MAX,&rounded)==CPN_CAPACITY && rounded==123);
    CHECK(cpn_round_even(INT64_MIN,&rounded)==CPN_CAPACITY && rounded==123);
    CHECK(cpn_round_even((int64_t)INT32_MAX*4+1,&rounded)==CPN_OK && rounded==INT32_MAX);
    CHECK(cpn_round_even((int64_t)INT32_MAX*4+2,&rounded)==CPN_CAPACITY && rounded==INT32_MAX);
    double impulse[]={2,0,0,0},out[4],state=0;
    CHECK(cpn_iir_half(&state,impulse,4,out,4)==CPN_OK);
    const double want[]={1,.5,.25,.125};for(size_t i=0;i<4;++i)CHECK(out[i]==want[i]);
    for(size_t split=0;split<=4;++split){state=0;CHECK(cpn_iir_half(&state,impulse,split,out,split)==CPN_OK);CHECK(cpn_iir_half(&state,impulse+split,4-split,out+split,4-split)==CPN_OK);for(size_t i=0;i<4;++i)CHECK(out[i]==want[i]);}
    double bad[]={1,NAN};state=.25;out[0]=99;CHECK(cpn_iir_half(&state,bad,2,out,2)==CPN_INVALID && state==.25 && out[0]==99);
}
