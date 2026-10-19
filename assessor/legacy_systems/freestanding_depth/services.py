"""Bounded refusing services, arena, reset and per-task instance model."""
def run(c,root,args):
 c.group='freestanding-services';pub=root/'labs/machine_image/freestanding_depth';private=root/'assessor/legacy_systems/freestanding_depth';src=private/'services.c'
 for cc in ('gcc','clang'):
  for name,flags in [('debug',['-O0']),('release',['-O2']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
   exe=c.root/(cc+'-services-'+name);c.run([cc,'-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-ffreestanding','-fno-builtin',*flags,'-I'+str(pub),pub/'service_tests.c',src,'-o',exe]);c.run([exe],contains='SERVICE-PASS:')
 for name,define,source in [('refusal','FR_MUTANT_REFUSAL',src),('resend','FR_MUTANT_RESEND',src),('retained','FR_MUTANT_RETAIN',src),('task','FR_MUTANT_TLS',src),('service-learner','UNSOLVED',pub/'services.c')]:
  exe=c.root/name;c.run(['gcc','-std=c17','-O2','-ffreestanding','-fno-builtin','-D'+define,'-I'+str(pub),pub/'service_tests.c',source,'-o',exe]);c.run([exe],expected=1,contains='SERVICE-CHECK:')
 for cc in ('gcc','clang','arm-none-eabi-gcc'):
  obj=c.root/(cc+'-services.o');flags=['-mcpu=cortex-m4','-mthumb'] if cc.startswith('arm') else []
  c.run([cc,'-std=c17','-O2','-ffreestanding','-fno-builtin',*flags,'-I'+str(pub),'-c',src,'-o',obj]);out=c.run(['arm-none-eabi-nm' if flags else 'nm','-u',obj]);c.require(not out.strip(),'service model has no implicit imported helper',dict(cc=cc,undefined=out))
 for oid,ordinary,changed in [('07.b','24-byte caller-owned output advances only accepted prefixes and rejects unavailable heap without writes','Aligned64-byte arena exhaustive used/alignment/size matrix rejects overflow/exhaustion without commit'),('08.a','Plan rejects overlap/range/stack alignment before bounded data/BSS writes','All33 reset cuts are restartable while retained and unrelated RAM bytes survive'),('08.b','Ready service precedes one initialization hook per task instance','Four independent task instances reject stale generation handles, duplicate hooks and generation exhaustion'),('08.c','Unsupported heap remains a visible no-change refusal before and after readiness','Client retries NOT_READY only after explicit transition and never treats UNSUPPORTED as retryable')]:
  c.observe('T11.'+oid,ordinary,['freestanding-services']);c.observe('T11.'+oid,changed,['freestanding-services'],True)
