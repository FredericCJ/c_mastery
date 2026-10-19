"""Original controlled two-TU, loader, ABI and local Git experiments for T01."""
from pathlib import Path
import json,re,shutil
import hashlib,itertools
def stages(c,root):
 c.group='T01.04-stages';d=c.root/'stages';d.mkdir()
 pub=root/'labs/foundation_list';ref=root/'assessor/foundation_list/list.c'
 for name,source in [('list.c',ref),('main.c',pub/'main.c'),('list.h',pub/'list.h')]:shutil.copyfile(source,d/name)
 for cc in ['gcc','clang']:
  c.run([cc,'-std=c17','-E','list.c','-o',cc+'.i'],d)
  c.run([cc,'-std=c17','-S',cc+'.i','-o',cc+'.s'],d)
  c.run([cc,'-c',cc+'.s','-o',cc+'.o'],d)
  c.run([cc,'-std=c17','-c','main.c','-o',cc+'-main.o'],d)
  c.run([cc,cc+'.o',cc+'-main.o','-o',cc+'-list'],d)
  c.run([d/(cc+'-list'),'2','-5','9'],contains='ok count=3 sum=6')
  c.require('ft_tokens' in (d/(cc+'.i')).read_text() and '#include "list.h"' not in (d/(cc+'.i')).read_text(),'preprocessed content '+cc)
  c.require('ft_tokens' in (d/(cc+'.s')).read_text(),'assembly symbol '+cc)
  c.run(['readelf','-hW',d/(cc+'.o')],contains='REL (Relocatable file)')
  c.run(['readelf','-lW',d/(cc+'-list')],contains='INTERP')
 c.observe('T01.04.a','Actual preprocessing, assembly text, relocatable objects, linked ELF and separate process execution; two application TUs',['stages'],False)
 shutil.copyfile(d/'list.c',d/'no_suffix')
 c.run(['gcc','-std=c17','-c','no_suffix','-o','inferred.o'],d)
 c.require(not (d/'inferred.o').exists(),'unknown suffix does not compile C')
 c.run(['gcc','-std=c17','-x','c','-c','no_suffix','-o','explicit.o'],d)
 c.run(['readelf','-hW',d/'explicit.o'],contains='REL (Relocatable file)')
 c.observe('T01.04.a','No suffix is treated as link input; explicit -x c produces a relocatable C object',['stages/no_suffix','stages/explicit.o'],True)
 c.write('stages/syntax.c','int broken( { return 0; }\n')
 c.run(['gcc','-std=c17','-c','syntax.c','-o','syntax.o'],d,expected='nonzero',contains='error:')
 c.run(['gcc','gcc-main.o','-o','missing-definition'],d,expected='nonzero',contains='ft_tokens')
 absent=d/'original-teaching-loader-does-not-exist'
 c.run(['gcc','gcc.o','gcc-main.o','-Wl,--dynamic-linker='+str(absent),'-o','bad-interpreter'],d)
 c.require((d/'bad-interpreter').is_file() and not absent.exists(),'existing ELF with absent named interpreter')
 c.run(['readelf','-lW',d/'bad-interpreter'],contains=str(absent))
 c.run([d/'bad-interpreter'],expected='launch-unavailable',contains='No such file')
 c.run([d/'gcc-list','12x'],expected=2,contains='malformed')
 c.observe('T01.04.c','Syntax, missing-definition, absent ELF interpreter and valid program input rejection are separate observed stages',['stages'],False)
 lib=d/'libs';lib.mkdir();c.run(['gcc','-std=c17','-fPIC','-shared','list.c','-o',lib/'libft.so'],d)
 c.run(['gcc','-std=c17','main.c','-L'+str(lib),'-lft','-Wl,-rpath,$ORIGIN/libs','-o','shared-list'],d)
 c.run(['readelf','-dW',d/'shared-list'],contains='libft.so')
 c.run([d/'shared-list','2','-5','9'],contains='ok count=3 sum=6')
 # Move only the newly created owned teaching library; no host loader setting.
 saved=d/'held-libft.so';assert lib.resolve().is_relative_to(c.root.resolve())
 (lib/'libft.so').rename(saved)
 c.run([d/'shared-list','2'],expected=127,contains='libft.so')
 saved.rename(lib/'libft.so');c.run([d/'shared-list','2'],contains='ok count=1 sum=2')
 c.observe('T01.04.c','Shared link succeeds; moving its original local dependency prevents loader completion, restoring it succeeds',['stages/shared-list','stages/libs/libft.so'],True)
 c.group='T01.04-frame'
 probe=c.write('frames.c','int ft_mix(int a,int b) { int local=a+b; return local*2; }\nint main(void) { return ft_mix(3,4)!=14; }\n')
 assembly={}
 for cc in ['gcc','clang']:
  for opt in ['0','3']:
   obj=c.root/(cc+'-frame-'+opt+'.o');exe=c.root/(cc+'-frame-'+opt)
   c.run([cc,'-std=c17','-O'+opt,'-g','-fno-omit-frame-pointer','-c',probe,'-o',obj])
   text=c.run(['objdump','-drwC',obj]);assembly[cc+'-O'+opt]=text
   c.run([cc,obj,'-o',exe]);c.run([exe])
   body=text.split('<ft_mix>:',1)[1].split('<main>:',1)[0] if '<ft_mix>:' in text else ''
   c.require(body and any(reg in body for reg in ['%edi','%rdi']) and any(reg in body for reg in ['%esi','%rsi']),'two actual parameter register families '+cc+opt)
   slots=re.findall(r'-0x[0-9a-f]+\(%rbp\)',body)
   c.require(bool(slots)==(opt=='0'),'actual local stack operands versus optimized dataflow '+cc+opt,slots)
 c.write('frame-observations.json',json.dumps(assembly,indent=2)+'\n')
 c.observe('T01.04.b','Actual x86-64 disassembly identifies two integer ABI parameters and return; source local is distinct from a required stack slot',['frames.c','frame-observations.json'],False)
 c.observe('T01.04.b','O0 andO3 object bodies are retained for annotation; optimized local may exist only as register/dataflow, not a separate addressable slot',['frame-observations.json'],True)

def revisions(c,root):
 c.group='T01.05-revisions';d=c.root/'original list repository';d.mkdir()
 pub=root/'labs/foundation_list';ref=root/'assessor/foundation_list/list.c'
 for name,source in [('main.c',pub/'main.c'),('list.h',pub/'list.h'),('tests.c',pub/'tests.c')]:shutil.copyfile(source,d/name)
 fixed=ref.read_text();bad=fixed.replace('#define FT_MUTANT 0','#define FT_MUTANT 1');c.require(bad!=fixed,'controlled defect activated')
 (d/'list.c').write_text(bad);hooks=c.root/'empty-hooks';hooks.mkdir()
 git=['git','-c','user.name=Curriculum Example','-c','user.email=curriculum@example.invalid','-c','commit.gpgsign=false','-c','core.hooksPath='+str(hooks)]
 c.run([*git,'init','--initial-branch=defect'],d);c.run([*git,'add','list.c','main.c','list.h','tests.c'],d);c.run([*git,'commit','-m','Original defined final-only prefix defect'],d)
 defect=c.run([*git,'rev-parse','HEAD'],d).strip()
 c.run([*git,'switch','-c','repair'],d);(d/'list.c').write_text(fixed)
 c.run([*git,'add','list.c'],d);c.run([*git,'commit','-m','Check each declared-range prefix before publication'],d)
 repair=c.run([*git,'rev-parse','HEAD'],d).strip()
 records=[]
 for name,commit,expected,marker in [('defect',defect,0,'ok count=3 sum=32767'),('repair',repair,5,'prefix')]:
  tree=c.root/('clean '+name);c.run([*git,'worktree','add','--detach',tree,commit],d)
  c.require(not c.run([*git,'status','--porcelain'],tree).strip(),'pristine source worktree '+name)
  out=c.root/('replay '+name);out.mkdir()
  for cc,opt in [('gcc','0'),('clang','3')]:
   exe=out/(cc+'-list');commands_start=len(c.commands)
   c.run([cc,'-std=c17','-O'+opt,'-I'+str(tree),'-c',tree/'list.c','-o',out/(cc+'-list.o')])
   c.run([cc,'-std=c17','-O'+opt,'-I'+str(tree),'-c',tree/'main.c','-o',out/(cc+'-main.o')])
   c.run([cc,out/(cc+'-list.o'),out/(cc+'-main.o'),'-o',exe])
   c.run([exe,'32767','1','-1'],expected=expected,contains=marker)
   c.run([exe,'2','-5','9'],contains='ok count=3 sum=6')
   records.append(dict(revision=name,commit=commit,compiler=shutil.which(cc),optimization=opt,command_indices=list(range(commands_start,len(c.commands))),source_directory=str(tree),binary=str(exe)))
 c.write('revisions.json',json.dumps(records,indent=2)+'\n')
 c.observe('T01.05.b','Original defect and repair commits replay from separate clean worktrees against identical minimized inputs',['revisions.json'],False)
 c.observe('T01.05.c','Both revisions bind compiler identity, exact argv/cwd/environment, source/generated hashes, inputs, statuses and output',['revisions.json','campaign command and generated-input records'],False)
 c.observe('T01.05.c','GCC O0 andClang O3 replay both revisions; mathematical prefix validity survives compiler change while disassembly/runtime observations are renewed',['revisions.json'],True)
 c.run([*git,'switch','-c','changed-interface',defect],d)
 for name in ['list.h','list.c','main.c','tests.c']:
  path=d/name;path.write_text(path.read_text().replace('ft_tokens','ft_parse_tokens'))
 c.run([*git,'add','list.h','list.c','main.c','tests.c'],d);c.run([*git,'commit','-m','Rename token entry API while preserving wire protocol'],d)
 changed=c.run([*git,'rev-parse','HEAD'],d).strip()
 c.run([*git,'rebase','--onto',changed,defect,'repair'],d)
 rebased=c.run([*git,'rev-parse','HEAD'],d).strip();c.require(rebased!=repair,'rebased source identity differs')
 exe=c.root/'rebased-list';c.run(['gcc','-std=c17','-Wall','-Werror','-I'+str(d),d/'list.c',d/'main.c','-o',exe])
 c.run([exe,'32767','1','-1'],expected=5,contains='prefix')
 # Old C API test is intentionally incompatible; the saved CLI regression is
 # still valid because the public wire and arithmetic properties did not change.
 old_test=c.root/'old-api-test.c';shutil.copyfile(pub/'tests.c',old_test)
 # Compile from a directory without the old header: quoted includes search the
 # source directory before -I. Keeping the original path would accidentally
 # validate the old header instead of the intended changed API.
 c.run(['gcc','-std=c17','-Werror','-I'+str(d),'-c',old_test,'-o',c.root/'old-api.o'],expected='nonzero',contains='ft_tokens')
 c.run(['gcc','-std=c17','-Wall','-Werror','-I'+str(d),d/'list.c',d/'tests.c','-o',c.root/'new-api-test'])
 c.run([c.root/'new-api-test'],contains='FT_PASS')
 c.observe('T01.05.b','Repair rebased onto changed API; same CLI regression still tests prefix atomicity, old C caller needs an explicit interface update',['original list repository','rebased-list'],True)
 c.group='T01.05-shell'
 spaced=c.root/'source directory with spaces';spaced.mkdir()
 for name,source in [('list.c',ref),('main.c',pub/'main.c'),('list.h',pub/'list.h')]:shutil.copyfile(source,spaced/name)
 c.run(['env','-i','PATH=/usr/bin:/bin','LC_ALL=C','/bin/sh',pub/'replay.sh',shutil.which('gcc'),ref,pub,c.root/'fresh-shell-output'],contains='ok count=3 sum=6')
 c.run(['env','-i','PATH=/usr/bin:/bin','LC_ALL=C','/bin/sh',pub/'replay.sh',shutil.which('gcc'),spaced/'list.c',spaced,c.root/'output directory with spaces'],contains='ok count=3 sum=6')
 c.observe('T01.05.a','Actual fresh environment shell invokes each compile and link explicitly',['replay.sh command records'],False)
 c.observe('T01.05.a','Spaced source and output directories preserve argv boundaries under the same script',['source directory with spaces','output directory with spaces'],True)

def diagnosis(c,root):
 c.group='T01.06-diagnosis';d=c.root/'stale-build';d.mkdir()
 pub=root/'labs/foundation_list';fixed=(root/'assessor/foundation_list/list.c').read_text()
 source=d/'list.c';source.write_text(fixed.replace('#define FT_MUTANT 0','#define FT_MUTANT 1'))
 c.run(['gcc','-std=c17','-O0','-I'+str(pub),'-c',source,'-o',d/'list.o'])
 c.run(['gcc','-std=c17','-O0','-I'+str(pub),'-c',pub/'main.c','-o',d/'main.o'])
 c.run(['gcc',d/'list.o',d/'main.o','-o',d/'list'])
 c.run([d/'list','32767','1','-1'],contains='ok count=3 sum=32767')
 sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
 before={p.name:sha(p) for p in [source,d/'list.o',d/'list']}
 source.write_text(fixed)
 # This real incorrect recipe only relinks the retained object after repair.
 c.run(['gcc',d/'list.o',d/'main.o','-o',d/'list'])
 c.run([d/'list','32767','1','-1'],contains='ok count=3 sum=32767')
 stale={p.name:sha(p) for p in [source,d/'list.o',d/'list']}
 c.require(before['list.c']!=stale['list.c'] and before['list.o']==stale['list.o'],'source changed but stale object consumed')
 c.run(['gcc','-std=c17','-O0','-I'+str(pub),'-c',source,'-o',d/'list.o'])
 c.run(['gcc',d/'list.o',d/'main.o','-o',d/'list'])
 c.run([d/'list','32767','1','-1'],expected=5,contains='prefix')
 after={p.name:sha(p) for p in [source,d/'list.o',d/'list']}
 c.require(before['list.o']!=after['list.o'] and before['list']!=after['list'],'recompilation actually changes linked input and result')
 c.write('stale-build-identities.json',json.dumps(dict(before=before,stale=stale,after=after),indent=2)+'\n')
 c.observe('T01.06.c','Repair in source is ineffective when the link consumes an old object; actual recompile fixes the saved regression',['stale-build-identities.json'],True)
 # Minimize two different properties. Do not call final-only mismatch a
 # two-token example: at least three are necessary for failure then recovery.
 bad=lambda xs:any(not -32768<=sum(xs[:i])<=32767 for i in range(1,len(xs)+1))
 mismatch=lambda xs:bad(xs) and -32768<=sum(xs)<=32767
 def shrink(xs,predicate):
  changed=True
  while changed:
   changed=False
   for i in range(len(xs)):
    candidate=xs[:i]+xs[i+1:]
    if predicate(candidate):xs=candidate;changed=True;break
  return xs
 a=shrink([0,32767,1,0],bad);b=shrink([0,32767,1,-1],mismatch)
 c.require(a==[32767,1] and b==[32767,1,-1],'different minimized properties',dict(prefix=a,cancellation=b))
 c.require(all(not bad(a[:i]+a[i+1:]) for i in range(len(a))) and all(not mismatch(b[:i]+b[i+1:]) for i in range(len(b))),'deletion minimality')
 c.write('minimization.json',json.dumps(dict(prefix=a,cancellation=b,method='Deterministic deletion, mathematical integer oracle; deletion-minimal, not unique global minimizer',first_invalid_prefix=32768),indent=2)+'\n')
 for cc in ['gcc','clang']:
  for opt in ['0','3']:c.run([cc,'-std=c17','-O'+opt,'-c',root/'assessor/foundation_list/analysis_only.c','-o',d/(cc+'-analysis-'+opt+'.o')])
 c.observe('T01.06.c','Two-element boundary prefix violates declared range; actual INT_MAX+1 check-after-add is separately analyzed and compile-only. Three elements distinguish final-only cancellation',['minimization.json','assessor/foundation_list/analysis_only.c'],False)
