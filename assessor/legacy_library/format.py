"""Exact signed-result, byte-width and two-phase publication observations."""
def run(c,root,args):
 c.group='library-format';pub=root/'labs/foundations/library_depth';source=root/'assessor/legacy_library/format.c'
 for cc in ('gcc','clang'):
  for name,flags in [('debug',['-O0']),('release',['-O2']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
   exe=c.root/(cc+'-'+name);c.run([cc,'-std=c17','-Wall','-Wextra','-Werror','-Wformat=2',*flags,'-I'+str(pub),pub/'format_tests.c',source,'-o',exe]);c.run([exe],contains='FORMAT-PASS:prefix:transaction:typed:width:byte-precision')
 for name,flag,selected in [('signed','LF_MUTANT_SIGN',source),('terminator','LF_MUTANT_FULL',source),('premature','LF_MUTANT_COMMIT',source),('learner','UNSOLVED',pub/'format.c')]:
  exe=c.root/name;c.run(['gcc','-std=c17','-O2','-I'+str(pub),'-D'+flag,pub/'format_tests.c',selected,'-o',exe]);c.run([exe],expected=1,contains='FORMAT-CHECK:')
 pairs=c.write('format/pairs.c',r'''#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
int main(void){printf("%d,%lu,%zu,%td,%" PRId32 ",%.1f,%.1Lf\n",-3,4UL,(size_t)5,(ptrdiff_t)-6,(int32_t)7,1.5,2.5L);return 0;}
''')
 bads=[('size','"%u",(size_t)1'),('ptrdiff','"%d",(ptrdiff_t)1'),('long-double','"%f",(long double)1'),('width','"%*s",(size_t)3,"a"')]
 for cc in ('gcc','clang'):
  exe=c.root/(cc+'-pairs');c.run([cc,'-std=c17','-Wall','-Wextra','-Werror','-Wformat=2',pairs,'-o',exe]);out=c.run([exe]);c.require(out=='-3,4,5,-6,7,1.5,2.5\n','independent literal type-pair expected output',cc)
  for name,arguments in bads:
   bad=c.write('format/'+name+'.c','#include <stdio.h>\n#include <stddef.h>\nvoid specimen(void){printf('+arguments+');}\n');c.run([cc,'-std=c17','-Wall','-Wextra','-Werror','-Wformat=2','-c',bad,'-o',c.root/(cc+'-'+name+'.o')],expected='nonzero',contains='format')
 for oid,ordinary,changed in [('09.a','Seven correct format/type pairs execute; four mismatches are deliberately rejected at translation and never run','Typed configuration policy rejects arbitrary formats and tag mismatch; literal %n payload remains ordinary text'),('09.b','Independent required length8 and exact right/left padding verify positive/negative dynamic width and negative precision','A two-byte precision returns E2 82, an incomplete UTF-8 prefix, without claiming a Unicode scalar boundary'),('09.c','Capacities0,1,4,7 and exact-length6 distinguish required/written/NUL/completeness; negative result remains signed','Query failure, render failure, unstable length and absent terminator all preserve public output and result before transactional commit')]:
  c.observe('T12.'+oid,ordinary,['library-format']);c.observe('T12.'+oid,changed,['library-format'],True)
