#include "propagation.h"
#include "propagation_macro.h"
static er_result scan(const unsigned char *s,size_t n,size_t *at){
    size_t i=*at,start=i;uint32_t v=0;
    while(i<n&&s[i]>='0'&&s[i]<='9'){
        uint32_t d=(uint32_t)(s[i]-'0');
        if(v>(UINT32_C(65535)-d)/10)return er_error(ER_RANGE,i);
        v=v*10+d;++i;
    }
    if(i==start)return er_error(ER_INVALID,i);
    *at=i;return er_ok(v);
}
er_result epr_pair_explicit(const unsigned char *s,size_t n,uint32_t *sum){
    size_t at=0;er_result a,b,r;
    if((!s&&n)||!sum)return er_error(ER_INVALID,0);
    a=scan(s,n,&at);if(a.tag!=ER_OK){r=a;goto failed;}
    if(at==n||s[at]!=':'){r=er_error(ER_INVALID,at);goto failed;}
    ++at;b=scan(s,n,&at);if(b.tag!=ER_OK){r=b;goto failed;}
    if(EPR_MUTATION!=7&&at!=n){r=er_error(ER_INVALID,at);goto failed;}
    if(EPR_MUTATION!=6&&a.as.value>UINT32_C(65535)-b.as.value){r=er_error(ER_RANGE,at);goto failed;}
    *sum=a.as.value+b.as.value;return er_ok(*sum);
failed:return r;
}
er_result epr_pair_macro(const unsigned char *s,size_t n,uint32_t *sum){
    size_t at=0;er_result a,b,r;
    if((!s&&n)||!sum)return er_error(ER_INVALID,0);
    EPR_TRY(first_result,scan(s,n,&at),&r,failed);a=r;
    if(at==n||s[at]!=':'){r=er_error(ER_INVALID,at);goto failed;}
    ++at;EPR_TRY(second_result,scan(s,n,&at),&r,failed);b=r;
    if(at!=n){r=er_error(ER_INVALID,at);goto failed;}
    if(a.as.value>UINT32_C(65535)-b.as.value){r=er_error(ER_RANGE,at);goto failed;}
    *sum=a.as.value+b.as.value;return er_ok(*sum);
failed:return r;
}
er_code epr_init(struct epr_parser *p,unsigned signed_first){
    if(!p||signed_first>1)return ER_INVALID;
    struct epr_parser empty={0};empty.signed_first=signed_first;empty.terminal=ER_AGAIN;*p=empty;return ER_OK;
}
static struct epr_step answer(struct epr_parser *p,size_t consumed){struct epr_step r={p->terminal,consumed,p->position,p->error};return r;}
static struct epr_step fail(struct epr_parser *p,size_t consumed,er_code code){
    p->phase=3;p->terminal=code;p->error=EPR_MUTATION==2?consumed:p->position;return answer(p,consumed);
}
struct epr_step epr_feed(struct epr_parser *p,const unsigned char *s,size_t n,unsigned final,uint32_t *sum){
    struct epr_step invalid={ER_INVALID,0,p?p->position:0,0};size_t used=0;
    if(!p||(!s&&n)||!sum||final>1)return invalid;
    if(n>SIZE_MAX-p->position){invalid.code=ER_RANGE;invalid.error=p->position;return invalid;}
    if(p->phase>=2){if(p->phase==2)*sum=p->sum;return answer(p,0);}
    while(used<n){
        unsigned char c=s[used];uint32_t *v=p->phase?&p->second:&p->first;
        if(p->phase==0&&p->position==0&&p->signed_first&&c=='-'){p->negative=1;}
        else if(c>='0'&&c<='9'){
            uint32_t d=(uint32_t)(c-'0');
            if(*v>(UINT32_C(65535)-d)/10)return fail(p,used,ER_RANGE);
            *v=*v*10+d;p->digits=1;
        }else if(p->phase==0&&c==':'&&p->digits){p->phase=1;p->digits=0;}
        else return fail(p,used,ER_INVALID);
        ++used;++p->position;
        if(EPR_MUTATION==3)*sum=p->first;
    }
    if(!final)return answer(p,used);
    if(p->phase!=1||!p->digits)return fail(p,used,ER_INVALID);
    if(p->negative){
        if(EPR_MUTATION!=4&&p->second<p->first)return fail(p,used,ER_RANGE);
        p->sum=p->second-p->first;
    }else{
        if(p->first>UINT32_C(65535)-p->second)return fail(p,used,ER_RANGE);
        p->sum=p->first+p->second;
    }
    p->phase=2;p->terminal=ER_OK;*sum=p->sum;return answer(p,used);
}
