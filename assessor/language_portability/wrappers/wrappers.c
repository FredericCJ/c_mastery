#include "wrappers.h"
#include <stdarg.h>
#include <stdio.h>
#ifndef LW_MUTANT
#define LW_MUTANT 0
#endif
static int left(LwObservation *s){++s->left;return 3;}
static int right(LwObservation *s){++s->right;return 7;}
static int emit(LwObservation *s,const char *format,...)
{int n;va_list args;++s->calls;va_start(args,format);n=vsnprintf(s->text,sizeof s->text,format,args);va_end(args);return n<0 || (unsigned)n>=sizeof s->text?-1:0;}
#if LW_MUTANT == 1
#define LW_TRACE(s,...) (emit((s),__VA_ARGS__),emit((s),__VA_ARGS__))
#else
#define LW_TRACE(s,...) emit((s),__VA_ARGS__)
#endif
#define LW_NAMED(s,format,...) emit((s),(format),__VA_ARGS__)
#define LW_ZERO(s,message) emit((s),"%s",(message))
#define LW_DISABLED(...) ((void)0)
int lw_trace(unsigned n,LwObservation *out)
{
 LwObservation s={0};int status=0;if(!out || n>5)return -1;
 switch(n){case 0:status=LW_TRACE(&s,"");break;case 1:status=LW_TRACE(&s,"%d",left(&s));break;case 2:status=LW_TRACE(&s,"%d:%d",left(&s),right(&s));break;case 3:status=LW_ZERO(&s,"literal");break;case 4:status=LW_NAMED(&s,"%d",left(&s));break;case 5:LW_DISABLED(&s,"%d:%d",left(&s),right(&s));break;}
 if(status)return -1;
 *out=s;return 0;
}
#define MIN_BAD(x,y) x<y?x:y
#define MIN_REPEAT(x,y) ((x)<(y)?(x):(y))
static inline int typed_min(int x,int y){return LW_MUTANT==2?y:(x<y?x:y);}
typedef struct {int value;} Box;
static Box box_left(LwObservation *s){Box b={left(s)};return b;}
static Box box_right(LwObservation *s){Box b={right(s)};return b;}
static inline Box box_min(Box x,Box y){return x.value<y.value?x:y;}
#define LW_MIN(x,y) _Generic((x),int:typed_min,Box:box_min)((x),(y))
int lw_min(unsigned n,LwObservation *out)
{
 LwObservation s={0};if(!out || n>3)return -1;
 switch(n){case 0:s.value=2*MIN_BAD(3,4);break;case 1:s.value=MIN_REPEAT(left(&s),right(&s));break;case 2:s.value=typed_min(left(&s),right(&s));break;case 3:s.value=LW_MIN(box_left(&s),box_right(&s)).value;break;}
 *out=s;return 0;
}
#define LW_TWO(s) do{(s)->value=3;if(LW_MUTANT!=3)++(s)->calls;}while(0)
#define LW_CONTINUE() do{continue;}while(0)
#define LW_BREAK() do{break;}while(0)
#define LW_COLLIDE(x) do{int lw_temp=1;(x)=lw_temp;}while(0)
static inline void set_value(LwObservation *s,int v){s->value=v;++s->calls;}
int lw_statement(unsigned n,LwObservation *out)
{
 LwObservation s={0};if(!out || n>5)return -1;
 switch(n){case 0:if(1)LW_TWO(&s);else s.value=7;break;case 1:if(0)LW_TWO(&s);else s.value=7;break;case 2:for(unsigned i=0;i<3;++i){LW_CONTINUE();++s.calls;}break;case 3:for(unsigned i=0;i<3;++i){LW_BREAK();++s.calls;}break;case 4:{int lw_temp=9;LW_COLLIDE(lw_temp);s.value=lw_temp;break;}case 5:set_value(&s,1);set_value(&s,2);break;}
 *out=s;return 0;
}
