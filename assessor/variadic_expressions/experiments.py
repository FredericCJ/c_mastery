"""Actual expression observations; undefined specimens are only translated."""
from pathlib import Path
def run(c,root,profiles):
 lab=root/'labs/variadic_expressions';ref=root/'assessor/variadic_expressions'
 source=c.write('conversions.c','''#include <limits.h>
#include <stdint.h>
#include <stdio.h>
_Static_assert(INT_MAX==2147483647 && USHRT_MAX==65535,"recorded native model");
_Static_assert(_Generic((1?-1:1u),unsigned int:1,default:0),"conditional common type");
_Static_assert(_Generic(+(unsigned short)1,int:1,default:0),"native short promotes to int");
int main(void){volatile int negative=-1;volatile unsigned int one=1u;
 volatile unsigned short small=1;volatile int yes=1;
 if(negative<one)return 1;
 if(!(negative<small))return 2;
 if((yes?negative:one)!=UINT_MAX)return 3;
 unsigned char before=(unsigned char)(unsigned int)-1;
 if(before!=UCHAR_MAX)return 4;
 printf("TF_CONVERSION_PASS unsigned_false short_true conditional=%u narrowed=%u\\n",yes?negative:one,(unsigned)before);
 return 0;}
''')
 for cc,opt,tag in profiles:
  c.group=tag+':sequencing';exe=c.root/(tag+'-sequencing')
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-fno-pie','-no-pie']+(['-O1','-g'] if 'asan' in tag else [])
  env={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
  c.run([cc,*flags,'-Werror',ref/'sequencing.c','-o',exe]);text=c.run([exe],contains='TF_SEQUENCE_PASS',env=env)
  c.require(all(m in text for m in ['TF_TREE_PASS','TF_INDEPENDENT','TF_SHARED']),'all sequencing mechanisms executed')
  c.group=tag+':conversion';exe=c.root/(tag+'-conversion')
  # Intentional sign-comparison observations stay visible; no warning-suppression flags.
  c.run([cc,*flags,source,'-o',exe],contains='sign-compare');c.run([exe],contains='TF_CONVERSION_PASS unsigned_false short_true conditional=4294967295 narrowed=255',env=env)
 for cc in ['gcc','clang']:
  c.group=cc+':analysis-only'
  c.run([cc,'-std=c17','-O0','-Wall','-Wextra','-Wpedantic','-c',ref/'analysis_only.c','-o',c.root/(cc+'-analysis.o')],contains='promot')
  c.group=cc+':negative-translation'
  bad=c.write(cc+'-bad-va-assignment.c','#include <stdarg.h>\nvoid copy_by_assignment(int count,...){va_list a,b;va_start(a,count);b=a;va_end(b);va_end(a);}\n')
  # This target's array-shaped va_list rejects assignment. On scalar-shaped ABIs assignment
  # may compile and is STILL not a portable substitute for va_copy.
  c.run([cc,'-std=c17','-pedantic-errors','-c',bad,'-o',c.root/(cc+'-bad-va-assignment.o')],expected=1,contains='array')
  good=c.write(cc+'-good-va-copy.c','#include <stdarg.h>\nvoid independent_copy(int count,...){va_list a,b;va_start(a,count);va_copy(b,a);va_end(b);va_end(a);}\n')
  c.run([cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic-errors','-c',good,'-o',c.root/(cc+'-good-va-copy.o')])
