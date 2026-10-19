"""Fresh exact P6 companion and unchanged-provider correctness, with retained binaries."""
import argparse,hashlib,json,os,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PUBLIC=['labs/numeric_followthrough/tests.c','labs/numeric_followthrough/numeric.h','labs/numeric_followthrough/numeric.c']
PROVIDERS=['assessor/checkpoint_depth/checkpoint_numeric.c','assessor/checkpoint_depth/checkpoint_vector.c','labs/checkpoint_depth/include/checkpoint_numeric.h','labs/checkpoint_depth/include/checkpoint_vector.h','labs/checkpoint_depth/tests/numeric.c','labs/checkpoint_depth/tests/vector.c','labs/checkpoint_depth/tests/numeric_experiment.c','labs/checkpoint_depth/CONTRACTS.md']
SOURCES=['labs/numeric_followthrough/tests.c','assessor/numeric_followthrough/numeric.c',*PROVIDERS[:2],*PROVIDERS[4:6]]
FLAGS=['-std=c17','-ffp-contract=off','-fno-fast-math','-march=x86-64','-mno-avx','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-Ilabs/numeric_followthrough','-Ilabs/checkpoint_depth/include']
QEMU='build/checkpoint-tools/qemu-user-local/usr/bin/qemu-x86_64'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def oracle(text):
 exact=rounds=0
 for line in text.splitlines():
  if line.startswith('NF_EXACT '):
   v=list(map(int,line.split()[1:]));x=v[:5];expected=[x[i]+(2*x[i-1] if i else 0)+(x[i-2] if i>1 else 0) for i in range(5)];assert v[5:]==expected;exact+=1
  elif line.startswith('NF_ROUND '):
   n,got=map(int,line.split()[1:]);q,r=divmod(n,4);want=q+(r>2 or r==2 and q%2!=0);assert got==want;rounds+=1
 assert exact==4096 and rounds==20003
 return dict(convolutions=exact,rounded_numerators=rounds)
def main():
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);a=p.parse_args();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False);rows=[];profiles=[];inputs=set(PUBLIC+PROVIDERS+['assessor/numeric_followthrough/numeric.c','tools/run_numeric_followthrough.py']);before={s:sha(ROOT/s) for s in sorted(inputs)}
 env=os.environ.copy()
 for k in ('CFLAGS','CPPFLAGS','LDFLAGS','CPATH','LIBRARY_PATH','C_INCLUDE_PATH'):env.pop(k,None)
 def run(name,cmd,expected=0,marker=None):
  r=subprocess.run(list(map(str,cmd)),cwd=ROOT,env=env,capture_output=True,text=True,timeout=180);text=r.stdout+r.stderr;log=work/(name+'.txt');log.write_text(text);row=dict(name=name,command=list(map(str,cmd)),returncode=r.returncode,expected=expected,marker=marker,passed=r.returncode==expected and (marker is None or marker in text),log=log.relative_to(ROOT).as_posix(),sha256=sha(log));rows.append(row);assert row['passed'],name;return text
 try:
  run('qemu-version',[ROOT/QEMU,'--version'])
  for cc in ('gcc','clang'):
   run(cc+'-version',[cc,'--version'])
   for opt,flags in [('O0',['-O0']),('O2',['-O2']),('san',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
    name=cc+'-'+opt;binary=work/name;run(name+'-build',[cc,*FLAGS,*flags,*SOURCES,'-lm','-Wl,-Map,'+str(work/(name+'.map')),'-o',binary]);out=run(name+'-run',[binary,'corpus'],marker='NF_PASS');counts=oracle(out);profiles.append(dict(name=name,oracle=counts,binary=binary.relative_to(ROOT).as_posix(),sha256=sha(binary),guarded=320 if 'avx_usable=1' in out else 240));run(name+'-symbols',['nm','-u',binary]);run(name+'-elf',['readelf','-lWd',binary])
    if opt!='san':run(name+'-no-avx',[ROOT/QEMU,'-cpu','Nehalem',binary],marker='CP_CAPS: avx_usable=0')
  for mutation in range(9,19):
   name='provider-mutant-'+str(mutation);binary=work/name;run(name+'-build',['gcc',*FLAGS,'-O2','-DCP_MUTANT='+str(mutation),*SOURCES,'-lm','-o',binary]);run(name+'-run',[binary],1,'CP_CHECK:')
  for mutation in (1,2):
   name='policy-mutant-'+str(mutation);binary=work/name;run(name+'-build',['gcc',*FLAGS,'-O2','-DNF_MUTANT='+str(mutation),*SOURCES,'-lm','-o',binary]);run(name+'-run',[binary],1,'NF_CHECK:')
  binary=work/'learner';sources=[s.replace('assessor/numeric_followthrough/numeric.c','labs/numeric_followthrough/numeric.c') for s in SOURCES];run('learner-build',['gcc',*FLAGS,'-O2',*sources,'-lm','-o',binary]);run('learner-run',[binary],1,'NF_CHECK:');passed=True;error=None
 except Exception as exc:passed=False;error=repr(exc)
 after={s:sha(ROOT/s) for s in sorted(inputs)}
 if after!=before:passed=False;error='source changed during campaign'
 result=dict(passed=passed,error=error,rows=rows,profiles=profiles,source_sha256=after,tool_sha256={QEMU:sha(ROOT/QEMU)},artifacts={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},scope='Actual host and QEMU Nehalem executions; selected no-AVX emulator is not a physical machine, timing or independent learner claim.')
 (work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,commands=len(rows),profiles=len(profiles))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
