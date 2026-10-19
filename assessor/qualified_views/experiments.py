"""Qualification diagnostics, lifetime specimens and actual volatile code inspection."""
import re
NEGATIVE={
 'double-pointer':('void f(void){int *p=0;const int **q=&p;(void)q;}','incompatible'),
 'const-pointee':('void f(void){int a=1;const int *p=&a;*p=2;}','read-only'),
 'const-pointer':('void f(void){int a=1,b=2;int *const p=&a;p=&b;}','read-only'),
 'both-const':('void f(void){int a=1;const int *const p=&a;*p=2;}','read-only'),
 'qualifier-drop':('void mutate(int *);void f(const int *p){mutate(p);}','qualifier'),
 'opaque-size':('#include "views.h"\nint f(void){return sizeof(FvOwner);}','incomplete')}
POSITIVE='''#include "views.h"
int main(void){int a=1,b=2;const int *p=&a;int *const q=&a;const int *const r=&a;
 p=&b;*q=3;return *p==2 && *r==3?0:1;}
'''
def run(c,root):
 lab=root/'labs/qualified_views';ref=root/'assessor/qualified_views';runtime=root/'assessor/execution/support.c'
 for cc in ['gcc','clang']:
  c.group=cc+':qualification'
  good=c.write(cc+'-qualified.c',POSITIVE);exe=c.root/(cc+'-qualified')
  flags=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(lab)]
  c.run([cc,*flags,good,'-o',exe]);c.run([exe])
  for name,(text,reason) in NEGATIVE.items():
   src=c.write(cc+'-'+name+'.c',text+'\n')
   if cc=='clang' and name=='const-pointer':reason='const-qualified'
   # Diagnostic categories are compiler-specific; match the intended constraint.
   c.run([cc,*flags,'-c',src,'-o',c.root/(cc+'-'+name+'.o')],expected=1,contains=reason)
  c.group=cc+':analysis-only';c.run([cc,'-std=c17','-O0','-Wall','-Wextra','-I'+str(lab),'-c',ref/'analysis_only.c','-o',c.root/(cc+'-analysis.o')])
  for opt in ['0','3']:
   c.group=cc+':object-O'+opt;obj=c.root/(cc+'-component-O'+opt+'.o')
   c.run([cc,'-std=c17','-O'+opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-I'+str(lab),'-c',ref/'views.c','-o',obj])
   names=c.run(['nm','-u',obj]);symbols=[line.split()[-1] for line in names.splitlines() if line.strip()]
   c.require(set(symbols)<={'memcpy','memset','memmove'},'actual component closure',symbols)
   c.run(['objdump','-drwC',obj]);c.run(['readelf','-sW',obj],contains='fv_copy')
  for mutant in [0,4]:
   c.group=cc+':volatile-'+str(mutant);obj=c.root/(cc+'-volatile-'+str(mutant)+'.o')
   c.run([cc,'-std=c17','-O3','-ffreestanding','-fno-builtin','-DFV_MUTANT='+str(mutant),'-I'+str(lab),'-c',ref/'views.c','-o',obj])
   code=c.run(['objdump','-d','--disassemble=fv_device_twice',obj],contains='<fv_device_twice>')
   accesses=len(re.findall(r'\(%rdi\)',code))
   c.require(accesses==(2 if mutant==0 else 1),'volatile reads observed; cached-read mutant distinguished',dict(mutant=mutant,loads=accesses,scope='recorded native x86-64 optimized implementation only'))
 c.group='runtime-provider';obj=c.root/'runtime.o';c.run(['gcc','-std=c17','-O3','-ffreestanding','-fno-builtin','-c',runtime,'-o',obj]);c.require(not c.run(['nm','-u',obj]).strip(),'original helper provider closes itself')
