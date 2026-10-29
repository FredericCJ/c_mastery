#include "escape.h"
#include "text.h"
#include "events.h"
#include <stdio.h>
#include <string.h>
#define REQUIRE(x) do{if(!(x)){fprintf(stderr,"ES_ASSERT:composition:%d:%s\n",__LINE__,#x);return 41;}}while(0)
struct es_capture {unsigned char bytes[64];size_t n,starts,ends,documents;};
static enum ev_action take(void *context,const struct ev_event *event) {
 struct es_capture *c=context;
 if(event->kind==EV_START)c->starts++;
 else if(event->kind==EV_END)c->ends++;
 else if(event->kind==EV_DOCUMENT)c->documents++;
 else {
  if(event->length>64-c->n)return EV_REJECT;
  for(size_t i=0;i<event->length;i++)c->bytes[c->n++]=event->data[i];
 }
 return EV_ACCEPT;
}
static enum ev_status parse(const unsigned char *s,size_t n,size_t split,struct es_capture *c) {
 struct ev_parser p;enum ev_status status=ev_init(&p,8);size_t used;
 if(status!=EV_OK)return status;
 status=ev_feed(&p,s,split,take,c,&used);
 if(status!=EV_OK||used!=split)return status==EV_OK?EV_INVALID:status;
 status=ev_feed(&p,s+split,n-split,take,c,&used);
 if(status!=EV_OK||used!=n-split)return status==EV_OK?EV_INVALID:status;
 return ev_finish(&p,take,c);
}
int main(void) {
 struct tc_program program;REQUIRE(tc_template_compile((const unsigned char *)"{a}",3,&program)==TC_OK);
 const char *cases[]={"","'","&","<a/>","&#39;","&apos;","&lt;x&gt;","&<>\"'\n\t","</a><b/>"};
 size_t observations=0;
 for(size_t k=0;k<sizeof cases/sizeof *cases;k++) {
  struct tc_model model={0};struct tc_text html;struct es_text canonical,direct,plain;
  size_t n=strlen(cases[k]);model.fields[0].count=1;model.fields[0].values[0].length=n;
  memcpy(model.fields[0].values[0].bytes,cases[k],n);
  REQUIRE(tc_template_render(&program,1,0,&model,1,4096,&html)==TC_OK);
  REQUIRE(es_recode_html(html.bytes,html.length,384,&canonical)==ES_OK);
  REQUIRE(es_encode((const unsigned char *)cases[k],n,384,&direct)==ES_OK);
  REQUIRE(canonical.length==direct.length&&!memcmp(canonical.bytes,direct.bytes,direct.length));
  REQUIRE(es_decode(canonical.bytes,canonical.length,64,&plain)==ES_OK&&plain.length==n&&!memcmp(plain.bytes,cases[k],n));
  unsigned char doc[391];memcpy(doc,"<a>",3);memcpy(doc+3,canonical.bytes,canonical.length);memcpy(doc+3+canonical.length,"</a>",4);
  for(size_t split=0;split<=canonical.length+7;split++) {
   struct es_capture c={0};REQUIRE(parse(doc,canonical.length+7,split,&c)==EV_DONE);
   REQUIRE(c.starts==1&&c.ends==1&&c.documents==1&&c.n==n&&!memcmp(c.bytes,cases[k],n));observations++;
  }
  if(k==1) {
   REQUIRE(html.length==5&&!memcmp(html.bytes,"&#39;",5));
   REQUIRE(es_decode(html.bytes,html.length,64,&plain)==ES_SYNTAX);
   memcpy(doc+3,html.bytes,html.length);memcpy(doc+3+html.length,"</a>",4);
   struct es_capture c={0};REQUIRE(parse(doc,html.length+7,0,&c)==EV_SYNTAX);
  }
 }
 printf("ES_COMPOSE:%zu:actual template numeric spelling, checked adapter, event text exactly once; EV_TEXT=%d\n",observations,EV_TEXT);return 0;
}
