#include "money.h"
#ifndef CM_MUTANT
#define CM_MUTANT 0
#endif
static int valid(CmMoney x)
{
    return x.scale<=9 && x.currency[0]>='A' && x.currency[0]<='Z' &&
        x.currency[1]>='A' && x.currency[1]<='Z' && x.currency[2]>='A' && x.currency[2]<='Z';
}
static int compatible(CmMoney a,CmMoney b)
{
    return valid(a)&&valid(b) && (CM_MUTANT==1 || (a.scale==b.scale &&
        a.currency[0]==b.currency[0] && a.currency[1]==b.currency[1] && a.currency[2]==b.currency[2]));
}
static uint64_t magnitude(int64_t x) { return x<0 ? UINT64_C(0)-(uint64_t)x : (uint64_t)x; }
/* Original fixed-iteration unsigned division; no library or compiler divider. */
static uint64_t div32(uint64_t value,uint32_t divisor,uint32_t *remainder)
{
    uint64_t q=0,r=0;
    for(unsigned i=64;i!=0;i--){r=(r<<1)|((value>>(i-1))&1u);if(r>=divisor){r-=divisor;q|=UINT64_C(1)<<(i-1);}}
    *remainder=(uint32_t)r;return q;
}
static CmStatus signed_value(uint64_t value,int negative,int64_t *out)
{
    uint64_t limit=negative?UINT64_C(1)<<63:(uint64_t)INT64_MAX;
    if(value>limit)return CM_OVERFLOW;
    *out=negative ? (value==limit?INT64_MIN:-(int64_t)value) : (int64_t)value;
    return CM_OK;
}
/* floor(|amount| * n / d), with an exact fractional numerator. */
static CmStatus product(uint64_t amount,uint32_t n,uint32_t d,uint64_t *out,uint32_t *rem)
{
    uint32_t r,junk;uint64_t q=div32(amount,d,&r);
    if(n && q>div32(UINT64_MAX,n,&junk))return CM_OVERFLOW;
    uint64_t high=q*n,low=div32((uint64_t)r*n,d,rem);
    if(high>UINT64_MAX-low)return CM_OVERFLOW;
    *out=high+low;return CM_OK;
}
CmStatus cm_make(int64_t amount,const char currency[3],unsigned scale,CmMoney *out)
{
    if(!currency||!out||scale>9)return CM_INVALID;
    CmMoney v={amount,{currency[0],currency[1],currency[2]},(uint8_t)scale};
    if(!valid(v))return CM_INVALID;
    *out=v;return CM_OK;
}
CmStatus cm_add(CmMoney a,CmMoney b,CmMoney *out)
{
    if(!out||!compatible(a,b))return CM_INVALID;
    if((b.minor>0 && a.minor>INT64_MAX-b.minor)||(b.minor<0 && a.minor<INT64_MIN-b.minor))return CM_OVERFLOW;
    if(CM_MUTANT==2 && b.minor==0)return CM_OVERFLOW;
    a.minor+=b.minor;*out=a;return CM_OK;
}
CmStatus cm_ratio(CmMoney a,uint32_t n,uint32_t d,CmRound mode,CmMoney *out)
{
    if(!out||!valid(a)||!d||(mode!=CM_ZERO&&mode!=CM_FLOOR&&mode!=CM_EVEN))return CM_INVALID;
    uint64_t q;uint32_t r;CmStatus s=product(magnitude(a.minor),n,d,&q,&r);if(s!=CM_OK)return s;
    int up=mode==CM_FLOOR && a.minor<0 && r;
    if(mode==CM_EVEN)up=(uint64_t)r*2>d || ((uint64_t)r*2==d && ((q&1u)||CM_MUTANT==3));
    if(CM_MUTANT==4 && mode==CM_FLOOR)up=0;
    if(up){if(q==UINT64_MAX)return CM_OVERFLOW;q++;}
    s=signed_value(q,a.minor<0,&a.minor);if(s!=CM_OK)return s;
    *out=a;return CM_OK;
}
CmStatus cm_allocate(CmMoney a,const uint32_t *weights,size_t n,CmMoney *out)
{
    if(!out||!weights||!valid(a)||n==0||n>CM_MAX)return CM_INVALID;
    uint32_t sum=0;for(size_t i=0;i<n;i++){if(weights[i]>UINT32_MAX-sum)return CM_OVERFLOW;sum+=weights[i];}
    if(!sum)return CM_INVALID;
    uint64_t parts[CM_MAX],used=0;uint32_t remainders[CM_MAX];unsigned chosen[CM_MAX]={0};
    for(size_t i=0;i<n;i++){CmStatus s=product(magnitude(a.minor),weights[i],sum,&parts[i],&remainders[i]);if(s!=CM_OK)return s;used+=parts[i];}
    uint64_t left=magnitude(a.minor)-used;
    if(CM_MUTANT==5)left=0;
    for(uint64_t k=0;k<left;k++){
        size_t best=n;
        for(size_t i=0;i<n;i++)if(!chosen[i]&&weights[i] && (best==n||remainders[i]>remainders[best]||(CM_MUTANT==6&&remainders[i]==remainders[best])))best=i;
        if(best==n)return CM_INVALID;
        chosen[best]=1;parts[best]++;
    }
    CmMoney staged[CM_MAX];
    for(size_t i=0;i<n;i++){staged[i]=a;CmStatus s=signed_value(parts[i],a.minor<0,&staged[i].minor);if(s!=CM_OK)return s;}
    for(size_t i=0;i<n;i++)out[i]=staged[i];
    return CM_OK;
}
CmStatus cm_settle(CmMoney *accounts,size_t n,size_t from,const size_t *dest,const uint32_t *weights,size_t k,CmMoney amount,size_t abort_after)
{
    if(!accounts||!dest||!weights||n==0||n>CM_MAX||from>=n||k==0||k>CM_MAX||amount.minor<0||!compatible(accounts[from],amount))return CM_INVALID;
    for(size_t i=0;i<n;i++)if(!valid(accounts[i]))return CM_INVALID;
    for(size_t i=0;i<k;i++){
        if(dest[i]>=n||dest[i]==from||!compatible(accounts[dest[i]],amount))return CM_INVALID;
        for(size_t j=0;j<i;j++)if(dest[j]==dest[i] && CM_MUTANT!=7)return CM_INVALID;
    }
    if(accounts[from].minor<amount.minor)return CM_FUNDS;
    CmMoney parts[CM_MAX],staged[CM_MAX];CmStatus s=cm_allocate(amount,weights,k,parts);if(s!=CM_OK)return s;
    for(size_t i=0;i<n;i++)staged[i]=accounts[i];
    staged[from].minor-=amount.minor;
    if(CM_MUTANT==8)accounts[from]=staged[from];
    if(abort_after==0)return CM_ABORTED;
    for(size_t i=0;i<k;i++){
        s=cm_add(staged[dest[i]],parts[i],&staged[dest[i]]);if(s!=CM_OK)return s;
        if(abort_after==i+1)return CM_ABORTED;
    }
    for(size_t i=0;i<n;i++)accounts[i]=staged[i];
    return CM_OK;
}
