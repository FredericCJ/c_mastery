#include "shunting.h"
#include <stdio.h>
#include <string.h>
int main(void) {
    char line[300];
    while(fgets(line,sizeof line,stdin)) {
        struct pd_tokens t;struct pd_tree a,b;struct sy_cost c;struct pd_diagnostic d;struct sy_state s;
        int lex,parse=-1,pratt=-1,eval=-1,stream=-1,same=0;int32_t value=123;
        unsigned mode=line[0]=='1';size_t n=strcspn(line+2,"\r\n");
        lex=pd_lex((const unsigned char *)line+2,n,&t,&d);
        if(lex==PD_OK) {
            parse=sy_parse(&t,mode,64,64,&a,&c,&d);pratt=pd_pratt(&t,mode,&b,&d);
            if(parse==PD_OK) {
                same=pratt==PD_OK && a.count==b.count && a.root==b.root;
                for(unsigned i=0;same && i<a.count;++i) {
                    struct pd_expr x=a.nodes[i],y=b.nodes[i];
                    same=x.kind==y.kind && x.left==y.left && x.right==y.right && x.value==y.value && x.offset==y.offset;
                }
                eval=pd_evaluate(&a,&value);
                stream=sy_begin(mode,64,64,&s);
                for(size_t i=0;stream==PD_OK || stream==PD_PENDING;++i) {
                    if(i==t.count)break;
                    stream=sy_push(&s,&t.tokens[i],&d);
                }
                if(stream==PD_OK) {
                    struct pd_tree v;struct sy_cost vc;stream=sy_result(&s,&v,&vc);
                    if(stream==PD_OK && (v.count!=a.count || v.root!=a.root || vc.reductions!=c.reductions || vc.pushes!=c.pushes))same=0;
                    for(unsigned i=0;same && i<a.count;++i) {
                        struct pd_expr x=a.nodes[i],y=v.nodes[i];
                        same=x.kind==y.kind && x.left==y.left && x.right==y.right && x.value==y.value && x.offset==y.offset;
                    }
                }
            }
        }
        printf("%d %d %d %d %d %d %ld\n",lex,parse,pratt,eval,stream,same,(long)value);
    }
    return ferror(stdin)?1:0;
}
