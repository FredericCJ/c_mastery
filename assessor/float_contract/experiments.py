"""Distinct representation, compiler, runtime-environment and oracle experiments."""
import json,re
from pathlib import Path

def run(c,root):
 ref=root/'assessor/float_contract';lab=root/'labs/float_contract'
 base=['-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-I'+str(lab)]
 strict=['-frounding-math','-ffp-contract=off','-fno-fast-math']
 mpfr=['-I/home/frede/cmastery-library-tools/sysroot/usr/include','-I/home/frede/cmastery-library-tools/sysroot/usr/include/x86_64-linux-gnu','/usr/lib/x86_64-linux-gnu/libmpfr.so.6','/usr/lib/x86_64-linux-gnu/libgmp.so.10']
 c.group='cpu-capability';probe=c.write('cpu.c','#include <stdio.h>\nint main(void){int ok=__builtin_cpu_supports("fma")&&__builtin_cpu_supports("avx2");puts(ok?"FMA_AVX2_AVAILABLE":"FMA_AVX2_UNAVAILABLE");return !ok;}\n');exe=c.root/'cpu';c.run(['gcc','-std=c17',probe,'-o',exe]);c.run([exe],contains='FMA_AVX2_AVAILABLE')
 measurements=[];manifest=[]
 for cc in ['gcc','clang']:
  for label,src,flags in [('observations','observations.c',[]),('equal-long-double','observations.c',['-mlong-double-64']),('ftz','ftz.c',[]),('interval','interval.c',mpfr)]:
   c.group=cc+':'+label;exe=c.root/(cc+'-'+label)
   c.run([cc,*base,*strict,ref/src,*flags,'-lm','-o',exe]);text=c.run([exe],contains={'ftz':'FF_FTZ_PASS SSE_process_local restored','interval':'FF_INTERVAL_PASS directed_bounds refuse_ambiguous increase_precision exact_sum'}.get(label,'FF_OBSERVATIONS_PASS format special conversion fma_four_modes'))
   if label=='equal-long-double':c.require('same_precision=1' in text and 'addition_distinguishes=0' in text,'actual double equals long double precision without long-double library ABI calls')
   if label=='ftz':c.run(['objdump','-drwC',exe],contains='ldmxcsr')
   if label=='interval':c.require('precision=53 midpoint=1 lo=0x1p+0 hi=0x1.0000000000001p+0 settled=0' in text and 'precision=128 midpoint=1 lo=0x1p+0 hi=0x1p+0 settled=1' in text,'interval ambiguity is refused then resolved')
  profiles=[('separate',['-ffp-contract=off','-frounding-math']),('contract',['-ffp-contract=fast','-mfma','-mavx2']),('fast',['-ffast-math'])]
  if cc=='gcc':profiles.append(('x87',['-mfpmath=387','-fexcess-precision=fast','-ffp-contract=off']))
  for mode,flags in profiles:
   c.group=cc+':operations:'+mode;obj=c.root/(cc+'-'+mode+'.o');exe=c.root/(cc+'-'+mode)
   if cc=='clang' and mode=='fast':
    c.group='clang:fast-diagnostic';c.run([cc,*[x for x in base if x!='-no-pie'],'-O3',*flags,'-c',ref/'operations.c','-o',c.root/'clang-fast-rejected.o'],expected=1,contains='-Wnan-infinity-disabled')
    c.group=cc+':operations:'+mode;flags=flags+['-Wno-error=nan-infinity-disabled']
   c.run([cc,*[x for x in base if x!='-no-pie'],'-O3',*flags,'-fstack-usage','-c',ref/'operations.c','-o',obj])
   asm=c.run(['objdump','-drwC',obj]);c.run(['nm','-u',obj])
   if mode=='contract':c.require('vfmadd' in asm,'actual contracted instruction')
   if mode=='separate':c.require('mulsd' in asm and 'addsd' in asm and 'vfmadd' not in asm,'actual separated scalar operations')
   if mode=='x87':c.require('fldl' in asm and 'fmull' in asm and 'faddl' in asm,'actual wider x87 operations')
   c.run([cc,*base,*strict,ref/'operations_driver.c',obj,'-lm','-o',exe])
   text=c.run([exe],contains='FF_OPERATIONS_PASS strict_caller finite_admission nan_filter_repair raw_timings')
   observed=dict(re.findall(r'(\w+)=([^ ]+)',next(x for x in text.splitlines() if x.startswith('OPERATIONS '))))
   c.require(observed['separate']=='0x0p+0' and observed['repaired']=='0x1.8p+1','actual semantic-type store and strict missing-data adapter')
   if mode in ['contract','x87']:c.require(observed['expression']=='-0x1p-54','fused or wider result follows changed rounding points')
   if mode=='separate':c.require(observed['expression']=='0x0p+0' and observed['missing_nan']=='0' and observed['zero_sign']=='0','strict exceptional behavior')
   if mode=='fast':c.require(observed['zero_sign']=='1' and (cc!='gcc' or observed['missing_nan']=='1'),'actual fast signed-zero violation and GCC NaN counterexample; Clang NaN observation has no promised value')
   times=[dict(re.findall(r'(\w+)=([^ ]+)',line)) for line in text.splitlines() if line.startswith('TIMING ')]
   c.require(len(times)==5 and all(int(x['nanoseconds'])>=0 and x['calls']=='32768' and x['count']=='64' and x['exact']=='-0x1p-5' for x in times),'timed equivalent exact finite workload after admission')
   measurements.append(dict(compiler=cc,mode=mode,trials=times,admission='64 exact dyadic finite values, exact sum-1/32; no exceptional-data or compensated-sum equivalence claim'))
   manifest.append(dict(compiler=cc,mode=mode,flags=flags,observed=observed))
 c.write('compiler-modes.json',json.dumps(manifest,indent=2)+'\n');c.write('timings.json',json.dumps(dict(observations=measurements,limits='Five raw trials per build, local host/OS/compiler. No universal speed claim or observed-maximum worst-case guarantee.'),indent=2)+'\n')
