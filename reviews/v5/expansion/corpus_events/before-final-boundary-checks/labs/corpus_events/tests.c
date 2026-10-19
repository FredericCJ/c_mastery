#include "events.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"EV_CHECK:%d:%s\n",__LINE__,#x);return 41;} } while(0)
typedef struct {EvEvent events[512];size_t count,calls;unsigned pause,waiting,reenter;EvParser*p;int bad;} Capture;
static EvAction capture(void *v,const EvEvent *e){
    Capture*c=v;c->calls++;
    if(c->reenter){size_t used=77;if(ev_feed(c->p,0,0,capture,c,&used)!=EV_BUSY||used!=77||ev_finish(c->p,capture,c)!=EV_BUSY||ev_cancel(c->p)!=EV_BUSY)c->bad=1;}
    if(c->pause&&!c->waiting){c->waiting=1;return EV_WAIT;}
    c->waiting=0;if(c->count==512)return EV_REJECT;c->events[c->count++]=*e;return EV_ACCEPT;
}
static EvStatus parse(const unsigned char*b,size_t n,size_t chunk,unsigned pause,Capture*c){
    EvParser p;if(ev_init(&p,EV_DEPTH)!=EV_OK)return EV_INVALID;c->p=&p;c->pause=pause;
    size_t at=0;EvStatus s=EV_OK;
    while(at<n){size_t count=n-at;if(count>chunk)count=chunk;size_t u=999;s=ev_feed(&p,b+at,count,capture,c,&u);if(u>count)return EV_INVALID;at+=u;if(s!=EV_OK&&s!=EV_PAUSED)return s;}
    do{s=ev_finish(&p,capture,c);}while(s==EV_PAUSED);
    return s;
}
static EvAction reject(void*v,const EvEvent*e){(void)v;(void)e;return EV_REJECT;}
static int parser(void){
    const char*text=" \n<a>x&amp;y<b/>z</a>\t";
    for(size_t chunk=1;chunk<=strlen(text);chunk++)for(unsigned pause=0;pause<2;pause++){
        Capture c={0};c.reenter=1;CHECK(parse((const unsigned char*)text,strlen(text),chunk,pause,&c)==EV_DONE);CHECK(!c.bad);CHECK(c.count>=7);
        CHECK(c.events[0].kind==EV_START&&c.events[0].data[0]=='a');CHECK(c.events[c.count-1].kind==EV_DOCUMENT);
        char letters[32]={0};size_t n=0,starts=0,ends=0;
        for(size_t i=0;i<c.count;i++){CHECK(c.events[i].sequence==i+1);if(c.events[i].kind==EV_CHARACTERS){memcpy(letters+n,c.events[i].data,c.events[i].length);n+=c.events[i].length;}if(c.events[i].kind==EV_START)starts++;if(c.events[i].kind==EV_END)ends++;}
        CHECK(n==4&&memcmp(letters,"x&yz",4)==0&&starts==2&&ends==2);
    }
    const char*bad[]={"","<a>","<a></b>","<a/><b/>","<a>&missing;</a>","<a>&amp</a>","<a x='y'/>","<!DOCTYPE a>","<a>\r</a>","<a/", "<1/>","x<a/>","<a>\x80</a>","<a><b></a></b>"};
    for(size_t i=0;i<sizeof bad/sizeof bad[0];i++){Capture c={0};CHECK(parse((const unsigned char*)bad[i],strlen(bad[i]),1,1,&c)==EV_SYNTAX);}
    const char*deep="<a><a><a><a><a><a><a><a><a/>";Capture c={0};CHECK(parse((const unsigned char*)deep,strlen(deep),1,0,&c)==EV_LIMIT);
    const char*longname="<abcdefghijklm/>";Capture d={0};CHECK(parse((const unsigned char*)longname,strlen(longname),1,0,&d)==EV_LIMIT);
    EvParser p;CHECK(ev_init(&p,1)==EV_OK);size_t used=99;Capture q={0};q.pause=1;q.p=&p;
    unsigned char input[]="<a/>";CHECK(ev_feed(&p,input,4,capture,&q,&used)==EV_PAUSED&&used==4);memset(input,'?',4);
    CHECK(ev_feed(&p,0,0,capture,&q,&used)==EV_PAUSED&&used==0);CHECK(q.count==1&&q.events[0].data[0]=='a');
    CHECK(ev_cancel(&p)==EV_CANCELLED);size_t calls=q.calls;CHECK(ev_finish(&p,capture,&q)==EV_CANCELLED&&q.calls==calls);
    CHECK(ev_init(&p,EV_DEPTH)==EV_OK);CHECK(ev_feed(&p,(const unsigned char*)"<a/>",4,reject,0,&used)==EV_CALLBACK);CHECK(used==4);CHECK(ev_finish(&p,capture,&q)==EV_CALLBACK);
    CHECK(ev_init(&p,EV_DEPTH)==EV_OK);p.sequence=UINT32_MAX;CHECK(ev_feed(&p,(const unsigned char*)"<a/>",4,capture,&q,&used)==EV_LIMIT);CHECK(ev_cancel(&p)==EV_LIMIT);
    CHECK(ev_init(&p,EV_DEPTH)==EV_OK);used=77;CHECK(ev_feed(&p,0,1,capture,&q,&used)==EV_INVALID&&used==77);
    /* Large text is processed with bounded state and no retained input. */
    CHECK(ev_init(&p,EV_DEPTH)==EV_OK);EvQueue queue;evq_init(&queue);EvEvent e;size_t total=0;
    CHECK(ev_feed(&p,(const unsigned char*)"<a>",3,evq_offer,&queue,&used)==EV_OK);CHECK(evq_pop(&queue,&e)==EV_OK);
    for(size_t k=0;k<10000;k++){unsigned char ch='x';CHECK(ev_feed(&p,&ch,1,evq_offer,&queue,&used)==EV_OK&&used==1);while(evq_pop(&queue,&e)==EV_OK)total+=e.length;}
    EvStatus s=ev_feed(&p,(const unsigned char*)"</a>",4,evq_offer,&queue,&used);CHECK(s==EV_OK||s==EV_PAUSED);while(evq_pop(&queue,&e)==EV_OK)if(e.kind==EV_CHARACTERS)total+=e.length;CHECK(total==10000);
    CHECK(ev_finish(&p,evq_offer,&queue)==EV_DONE);puts("EV_OK:parser");return 0;
}
static int consumer(void){
    const unsigned char doc[]="<sum><g><n>12345</n><n>4294967295</n></g><n>7</n></sum>";
    for(size_t chunk=1;chunk<sizeof doc;chunk++){
        EvParser p;EvQueue q;EvSum s;EvEvent e;CHECK(ev_init(&p,EV_DEPTH)==EV_OK);evq_init(&q);evs_init(&s);size_t at=0;
        while(at<sizeof doc-1){size_t n=sizeof doc-1-at;if(n>chunk)n=chunk;size_t used;EvStatus r=ev_feed(&p,doc+at,n,evq_offer,&q,&used);CHECK(r==EV_OK||r==EV_PAUSED);at+=used;while(evq_pop(&q,&e)==EV_OK)CHECK(evs_event(&s,&e)==EV_OK);}
        EvStatus r;do{r=ev_finish(&p,evq_offer,&q);while(evq_pop(&q,&e)==EV_OK)CHECK(evs_event(&s,&e)==EV_OK);}while(r==EV_PAUSED);
        CHECK(r==EV_DONE&&s.done&&s.sum==UINT64_C(4294979647));
    }
    EvSum s;evs_init(&s);EvEvent e={EV_START,1,1,3,{'s','u','m'}};CHECK(evs_event(&s,&e)==EV_OK);EvSum before=s;CHECK(evs_event(&s,&e)==EV_SYNTAX&&memcmp(&s,&before,sizeof s)==0);
    e=(EvEvent){EV_START,2,2,1,{'n'}};CHECK(evs_event(&s,&e)==EV_OK);e=(EvEvent){EV_CHARACTERS,3,2,1,{'9'}};CHECK(evs_event(&s,&e)==EV_OK);
    before=s;e=(EvEvent){EV_CHARACTERS,4,2,2,{'1','x'}};CHECK(evs_event(&s,&e)==EV_SYNTAX&&memcmp(&s,&before,sizeof s)==0);
    s.value=UINT32_MAX;before=s;e=(EvEvent){EV_CHARACTERS,4,2,1,{'0'}};CHECK(evs_event(&s,&e)==EV_LIMIT&&memcmp(&s,&before,sizeof s)==0);
    s.value=9;s.sum=UINT64_MAX-8;before=s;e=(EvEvent){EV_END,4,2,1,{'n'}};CHECK(evs_event(&s,&e)==EV_LIMIT&&memcmp(&s,&before,sizeof s)==0);
    EvQueue q;evq_init(&q);e=(EvEvent){EV_START,1,1,1,{'a'}};CHECK(evq_offer(&q,&e)==EV_ACCEPT);e.data[0]='b';CHECK(evq_offer(&q,&e)==EV_ACCEPT);CHECK(evq_offer(&q,&e)==EV_WAIT);CHECK(evq_pop(&q,&e)==EV_OK&&e.data[0]=='a');CHECK(evq_pop(&q,&e)==EV_OK&&e.data[0]=='b');EvEvent old=e;CHECK(evq_pop(&q,&e)==EV_PAUSED&&memcmp(&e,&old,sizeof e)==0);
    puts("EV_OK:consumer");return 0;
}
static int oracle(void){
    char hex[8193];size_t chunk;unsigned pause;
    while(scanf("%8192s %zu %u",hex,&chunk,&pause)==3){unsigned char b[4096];size_t n=strlen(hex)/2;for(size_t i=0;i<n;i++){unsigned x;if(sscanf(hex+2*i,"%2x",&x)!=1)return 3;b[i]=(unsigned char)x;}Capture c={0};EvStatus s=parse(b,n,chunk,pause,&c);printf("%d",s);for(size_t i=0;i<c.count;i++){EvEvent*e=&c.events[i];printf(" %u,%zu,",(unsigned)e->kind,e->depth);for(size_t j=0;j<e->length;j++)printf("%02x",e->data[j]);}putchar('\n');}
    return 0;
}
int main(int argc,char**argv){if(argc!=2)return 2;if(!strcmp(argv[1],"parser"))return parser();if(!strcmp(argv[1],"consumer"))return consumer();if(!strcmp(argv[1],"oracle"))return oracle();return 2;}
