"""Original type/scope/linkage specimens; undefined cases are never executed."""
from pathlib import Path

def run(c,root):
 lab=root/'labs/type_interfaces';ref=root/'assessor/type_interfaces/types.c'
 def case(name,text,objective,expected=0,reason=None,run=False,extra=()):
  p=c.write('cases/'+name+'.c',text+'\n');c.group=objective
  for cc in ['gcc','clang']:
   output=c.root/(cc+'-'+name+('' if run else '.o'))
   c.run([cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(lab),*extra,*([] if run else ['-c']),p,*([ref] if run and 'types.h' in text else []),'-o',output],expected=expected,contains=reason)
   if run and expected==0:c.run([output])
  return p
 case('row-shape','#include "types.h"\nvoid f(void){int *a[4]={0};int (*b)[4]=a;(void)b;}','T02.01.a',1,'incompatible')
 case('alias-compatible','typedef int Unary(int);typedef Unary *Pointer;Pointer factory(void);int (*factory(void))(int);int twice(int n){return n*2;}Pointer factory(void){return twice;}int main(void){return factory()(7)!=14;}','T02.01.c',run=True)
 case('wrapper-not-function','#include "types.h"\nint f(void){struct td_callable c=td_wrapped_factory();return c(7);}','T02.01.c',1,'call')
 case('opaque-size','#include "types.h"\nunsigned f(void){return sizeof(struct td_opaque);}','T02.02.a',1,'incomplete')
 case('function-size','unsigned f(void){return sizeof(int(int));}','T02.02.a',1,'function')
 case('void-size','unsigned f(void){return sizeof(void);}','T02.02.a',1,'void')
 case('category-positive','struct Opaque;struct Pair{int x,y;};typedef int Row[4];int f(int);int main(void){struct Opaque *p=0;struct Pair v={1,2};Row a={0};return !(sizeof a==4*sizeof(int)&&sizeof v>=2*sizeof(int)&&p==0);}','T02.02.a',run=True)
 case('top-const','int f(const int);int f(int);int f(int x){return x;}int main(void){return f(7)!=7;}','T02.02.b',run=True)
 case('pointee-const','int g(int*);int g(const int*);','T02.02.b',1,'conflicting')
 case('callback-sign','int step(unsigned x){return x==1;}int (*callback)(int)=step;','T02.02.b',1,'incompatible')
 case('signed-wrapper','static unsigned calls;static int step(unsigned x){++calls;return (int)x+1;}static int bridge(int x){if(x<0||x>1000)return -1;return step((unsigned)x);}int main(void){int (*cb)(int)=bridge;return cb(-1)!=-1||calls!=0||cb(7)!=8||calls!=1;}','T02.02.b',run=True)
 case('units-alias','typedef int Meters;typedef int Seconds;int speed(Meters d,Seconds t){return d/t;}int main(void){Meters d=12;Seconds t=3;return speed(t,d)!=0;}','T02.02.c',run=True)
 case('units-swapped','#include "types.h"\nTdStatus f(void){struct td_speed out;return td_speed((struct td_seconds){3},(struct td_meters){12},&out);}','T02.02.c',1,'incompatible')
 case('prototype-name','int f(int prototype_only);int g(void){return prototype_only;}','T02.03.a',1,'prototype_only')
 case('scope-map','static int value=4;int f(int prototype_only);int main(void){int before=value;{int value=5;before+=value;}goto done;done:return before!=9||value!=4;}','T02.03.a',run=True)
 case('prototype-tag','int f(struct Private *p);struct Private{int n;};int f(struct Private *p){return p->n;}','T02.03.a',1,'conflicting')
 case('file-tag','struct Public;int f(struct Public *p);struct Public{int n;};int f(struct Public *p){return p->n;}int main(void){struct Public p={7};return f(&p)!=7;}','T02.03.a',run=True)
 case('namespaces','struct name{int name;};struct Other{int name;};int main(void){int name=1;struct name a={2};struct Other b={3};goto name;name:return name+a.name+b.name!=6;}','T02.03.b',run=True)
 case('ordinary-collision','typedef int name;int name;','T02.03.b',1,'different kind')
 # Legal but wrong policy: shadowing is not executed undefined behavior.
 case('shadow-defect','int main(void){int status=0;{int status=7;(void)status;}return status!=0;}','T02.03.c',run=True)
 case('shadow-repair','int main(void){int status=0;{int observed=7;status=observed;}return status!=7;}','T02.03.c',run=True)
 case('shadow-diagnostic','int f(void){int status=0;{int status=7;(void)status;}return status;}','T02.03.c',1,'shadow',extra=['-Wshadow'])
 case('callback-status','typedef struct{int *status;} Context;static void report(void *p,int error){Context *c=p;*c->status=error;}int main(void){int status=0;Context c={&status};report(&c,7);return status!=7;}','T02.03.c',run=True)
 case('visible-extern','static int x=7;int f(void){extern int x;return x;}int main(void){return f()!=7;}','T02.04.b',run=True)
 case('register-address','int *f(void){register int x=1;return &x;}','T02.04.b',1,'address')
 # In one TU the same identifier gains internal and external linkage. This
 # violates6.2.2p7; translation outcome is observed, no execution is permitted.
 hidden=c.write('cases/hidden-extern-analysis.c','static int x;int f(void){int x=1;{extern int x;return x;}return x;}\n')
 c.group='T02.04.b'
 for cc in ['gcc','clang']:
  # GCC/Clang diagnose differently, including accepting this UB translation.
  # Capture outcome without imposing a required diagnostic on undefined behavior.
  import subprocess,os
  argv=[cc,'-std=c17','-c',str(hidden),'-o',str(c.root/(cc+'-hidden.o'))]
  r=subprocess.run(argv,cwd=c.root,capture_output=True,text=True,env=dict(os.environ,LC_ALL='C'))
  c.commands.append(dict(group=c.group,argv=argv,cwd=str(c.root),exit=r.returncode,expected=r.returncode,diagnostic=None,stdout=r.stdout,stderr=r.stderr,environment={'LC_ALL':'C'},passed=r.returncode in [0,1],category='analysis-only-observation'))
  c.require(r.returncode in [0,1],'hidden extern translation observed '+cc)
 case('cast-analysis','int step(unsigned x){return x==1;}int analysis_only(int x){int (*bad)(int)=(int(*)(int))step;return bad(x);}','T02.02.b')
 case('adapter-cast-analysis','int step(int,void*);int analysis_only(void *p){int (*bad)(void*,int)=(int(*)(void*,int))step;return bad(p,1);}','T02.05.c',extra=['-Wno-cast-function-type'])
 # The cast specimen stays private/compile-only. Its suppressed optional cast
 # warning is an observation, not the repair; the actual wrapper is types.c.
 case('static-row-precondition','int f(int a[static 4]){return a[3];}int main(void){int a[4]={1,2,3,4};return f(a)!=4;}','T02.05.b',run=True)
 case('parameter-adjustment','#include <stddef.h>\n#include <stdio.h>\nstatic size_t f(int a[4]){return sizeof a;}static size_t g(int (*a)[4]){return sizeof *a;}int main(void){int a[4]={0};printf("TD_SIZE pointer=%zu row=%zu\\n",f(a),g(&a));return f(a)!=sizeof(int*)||g(&a)!=4*sizeof(int);}','T02.05.b',run=True,extra=['-Wno-sizeof-array-argument'])
 case('pointer-row-mismatch','void g(int (*)[4]);void f(void){int a[4]={0};g(a);}','T02.05.b',1,'incompatible')
 c.group='T02.05.a';old=c.write('cases/empty-c17.c','int f();int main(void){return f(1)!=2;}\n');definition=c.write('cases/empty-definition.c','int f(int x){return x+1;}\n');new=c.write('cases/empty-c23.c','int f();int f(){return 2;}int main(void){return f()!=2;}\n')
 for cc in ['gcc','clang']:
  exe=c.root/(cc+'-empty-c17');c.run([cc,'-std=c17',old,definition,'-o',exe]);c.run([exe])
  c.run([cc,'-std=c23','-pedantic-errors','-c',old,'-o',c.root/(cc+'-empty-reject.o')],expected=1,contains='argument')
  exe=c.root/(cc+'-empty-c23');c.run([cc,'-std=c23','-pedantic-errors',new,'-o',exe]);c.run([exe])
  c.run([cc,'-std=c23','-dM','-E','-x','c','/dev/null'],contains='__STDC_VERSION__ 202311L')

def units(c,root):
 c.group='T02.01.b';d=c.root/'units';d.mkdir()
 a=c.write('units/a.c','extern int x;int x;typedef int Count;int f(int);int f(int n){return n+x;}\n')
 b=c.write('units/b.c','extern int x;int f(int);int main(void){x=7;return f(2)!=9;}\n')
 for cc in ['gcc','clang']:
  for name,p in [('a',a),('b',b)]:c.run([cc,'-std=c17','-fno-common','-c',p,'-o',d/(cc+'-'+name+'.o')])
  c.run(['nm','-S',d/(cc+'-a.o')],contains=' B x');c.run(['nm','-S',d/(cc+'-b.o')],contains=' U x')
  exe=d/(cc+'-one');c.run([cc,d/(cc+'-a.o'),d/(cc+'-b.o'),'-o',exe]);c.run([exe])
  initialized=c.write('units/extern-init.c','extern int x=3;\n')
  c.run([cc,'-std=c17','-c',initialized,'-o',d/(cc+'-init.o')])
  c.run(['nm','-S',d/(cc+'-init.o')],contains=' D x')
  c.run([cc,d/(cc+'-a.o'),d/(cc+'-b.o'),d/(cc+'-init.o'),'-o',d/(cc+'-duplicate')],expected='nonzero',contains='multiple definition')
 c.group='T02.04.a'
 for name in ['left','right']:c.write('units/'+name+'.c','static int state=3;int *'+name+'(void){return &state;}\n')
 owner=c.write('units/owners.c','int *left(void);int *right(void);int external=7;int f(void){int automatic=2;static int persistent=3;return automatic+persistent++;}int main(void){int *a=left(),*b=right();*a=9;return a==b||*b!=3||f()!=5||f()!=6||external!=7;}\n')
 for cc in ['gcc','clang']:
  objects=[]
  for name in ['left','right','owners']:
   obj=d/(cc+'-'+name+'.o');objects.append(obj);c.run([cc,'-std=c17','-O0','-c',d/(name+'.c'),'-o',obj]);c.run(['nm','-aS',obj],contains='state' if name!='owners' else 'persistent')
  exe=d/(cc+'-owners');c.run([cc,*objects,'-o',exe]);c.run([exe])
 c.group='T02.04.b'
 tls=c.write('units/tls.c','#include <pthread.h>\nstatic _Thread_local int value=3;struct Result{int before,after;};static void *worker(void *p){struct Result *r=p;r->before=value;value=9;r->after=value;return 0;}int main(void){pthread_t t;static struct Result r;value=7;if(pthread_create(&t,0,worker,&r))return 2;if(pthread_join(t,0))return 3;return value!=7||r.before!=3||r.after!=9;}\n')
 for cc in ['gcc','clang']:
  exe=d/(cc+'-tls');c.run([cc,'-std=c17','-pthread',tls,'-o',exe]);c.run([exe]);c.run(['readelf','-sW',exe],contains='TLS')
 c.group='T02.04.c'
 bad=c.write('units/bad-provider.c','double value(double x){return x+1;}\n');caller=c.write('units/bad-caller.c','int value(int);int main(void){return value(1)!=2;}\n')
 for cc in ['gcc','clang']:
  # Independent TUs may link. Incompatible function invocation is never run.
  for name,p in [('provider',bad),('caller',caller)]:c.run([cc,'-std=c17','-c',p,'-o',d/(cc+'-bad-'+name+'.o')])
  c.run([cc,d/(cc+'-bad-provider.o'),d/(cc+'-bad-caller.o'),'-o',d/(cc+'-bad-link')])
 header=c.write('units/public.h','int value(int);\n');fixed=c.write('units/fixed.c','#include "public.h"\nint value(int x){return x+1;}\n');wrong=c.write('units/header-reject.c','#include "public.h"\ndouble value(double x){return x+1;}\n')
 for cc in ['gcc','clang']:
  c.run([cc,'-std=c17','-c',wrong,'-o',d/(cc+'-header-reject.o')],expected=1,contains='conflicting')
  exe=d/(cc+'-fixed');c.run([cc,'-std=c17',fixed,caller,'-o',exe]);c.run([exe])
 c.run(['gcc','-std=c17','-flto','-Werror=lto-type-mismatch',bad,caller,'-o',d/'lto-bad'],expected='nonzero',contains='type of')
 c.run(['gcc','-std=c17','-flto','-Werror=lto-type-mismatch',fixed,caller,'-o',d/'lto-good']);c.run([d/'lto-good'])
 c.group='T02.02.a'
 lab=root/'labs/type_interfaces';ref=root/'assessor/type_interfaces/types.c'
 left=c.write('units/opaque-left.c','#include "types.h"\nTdStatus left(void){return td_add(td_owner(0),7);}\n')
 right=c.write('units/opaque-right.c','#include "types.h"\nTdStatus right(int *out){return td_get(td_owner(0),out);}\n')
 main=c.write('units/opaque-main.c','#include "types.h"\nTdStatus left(void);enum td_status right(int*);int main(void){int n=91;return left()!=TD_OK||right(&n)!=TD_OK||n!=7;}\n')
 for cc in ['gcc','clang']:
  objects=[]
  for i,source in enumerate([left,right,main,ref]):
   obj=d/(cc+'-opaque-'+str(i)+'.o');objects.append(obj);c.run([cc,'-std=c17','-I'+str(lab),'-c',source,'-o',obj])
  exe=d/(cc+'-opaque');c.run([cc,*objects,'-o',exe]);c.run([exe])
 c.group='T02.02.b'
 top_a=c.write('units/top-provider.c','int top(int x){return x;}\n')
 top_b=c.write('units/top-caller.c','int top(const int);int main(void){return top(7)!=7;}\n')
 point_a=c.write('units/point-provider.c','int pointee(int *x){return *x;}\n')
 point_b=c.write('units/point-caller.c','int pointee(const int*);int main(void){int x=7;return pointee(&x)!=7;}\n')
 for cc in ['gcc','clang']:
  for name,source in [('top-a',top_a),('top-b',top_b),('point-a',point_a),('point-b',point_b)]:c.run([cc,'-std=c17','-c',source,'-o',d/(cc+'-'+name+'.o')])
  exe=d/(cc+'-top');c.run([cc,d/(cc+'-top-a.o'),d/(cc+'-top-b.o'),'-o',exe]);c.run([exe])
  # ABI-compatible representation does not make pointed-to const function
  # declarations compatible in C. This linked specimen is deliberately not run.
  c.run([cc,d/(cc+'-point-a.o'),d/(cc+'-point-b.o'),'-o',d/(cc+'-point-analysis')])
