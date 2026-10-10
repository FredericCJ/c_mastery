#!/usr/bin/env python3
"""Verify the two new real Linux families, including learner and fault rejection.

Run inside Linux/WSL. Build outputs and reports go only to --work (default build/).
Hosted sanitizer results and uninstrumented freestanding ELF closure are separate
claims; no physical, privileged-policy or whole-project mastery is asserted.
"""
import argparse, json, os, pathlib, re, shutil, subprocess, sys, time
ROOT=pathlib.Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--work',type=pathlib.Path,default=ROOT/'build/v4-machine-hosted');parser.add_argument('--profiles',nargs='+',default=['gcc-debug','gcc-release','clang-debug','clang-release','clang-asan']);parser.add_argument('--skip-mutations',action='store_true');args=parser.parse_args();work=args.work.resolve();work.mkdir(parents=True,exist_ok=True)
if sys.platform!='linux':raise SystemExit('Run under Linux/WSL: real fork, mmap, signals and ELF tools are required.')
report={'schema_version':1,'families':['machine_image','hosted_systems'],'profiles':[],'learners':[],'mutations':[],'claims':'Finite executable mechanism gates; no learner mastery or physical evidence','failures':[]}

def run(command,log,expected=0,timeout=90):
 start=time.monotonic()
 try:
  r=subprocess.run([str(x) for x in command],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=timeout,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'},cwd=ROOT)
  text=r.stdout.decode(errors='replace');code=r.returncode
 except subprocess.TimeoutExpired as e:text=(e.stdout or b'').decode(errors='replace')+'\nTIMEOUT';code=124
 log.parent.mkdir(parents=True,exist_ok=True);log.write_text(text,encoding='utf-8')
 if expected is not None and code!=expected:raise RuntimeError(f'{command[0]} exit {code}; see {log}\n{text[-2400:]}')
 return code,time.monotonic()-start

def mutate(text,before,after):
 # Token matching survives formatting while requiring the deliberate fault site.
 tokens=re.findall(r'\w+|[^\w\s]',before)
 pattern=re.compile(r'\s*'.join(re.escape(t) for t in tokens))
 if not pattern.search(text):raise RuntimeError('mutation site missing: '+before)
 return pattern.sub(lambda match: after,text)

def configure(family,name,compiler='gcc',kind='Debug',assessor=None,asan=False):
 build=work/name;cmd=['cmake','-S',ROOT/'labs'/family,'-B',build,'-G','Ninja','-DCMAKE_C_COMPILER='+compiler,'-DCMAKE_ASM_COMPILER='+compiler,'-DCMAKE_BUILD_TYPE='+kind,'-DCMASTERY_ASSESSOR_ROOT='+str(assessor or '')]
 if asan:cmd+=['-DCMAKE_C_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined']
 run(cmd,build/'configure.log');run(['cmake','--build',build,'--parallel','4'],build/'build.log');return build

try:
 for family in report['families']:
  data=json.loads((ROOT/'labs'/family/'family.json').read_text())
  for profile in args.profiles:
   compiler,suffix=profile.split('-',1);kind='Release' if suffix=='release' else 'Debug';b=configure(family,f'{family}-{profile}',compiler,kind,ROOT/'assessor'/family,suffix=='asan');_,seconds=run(['ctest','--test-dir',b,'--output-on-failure'],b/'tests.log');report['profiles'].append({'family':family,'profile':profile,'status':'passed','seconds':round(seconds,3)});print(f'{family} {profile}: passed',flush=True)
  b=configure(family,f'{family}-learner')
  code,_=run(['ctest','--test-dir',b,'--output-on-failure'],b/'tests.log',expected=None);text=(b/'tests.log').read_text();rejected=[]
  for unit in data['units']:
   # At least one semantic gate per unit must reject; a supplied observation may pass.
   if not any(re.search(re.escape(test)+r'[^\n]*(?:Failed|Timeout|Subprocess aborted)',text) for test in unit['tests']):raise RuntimeError('learner unit did not reject: '+unit['id'])
   rejected.append(unit['id'])
  if not code:raise RuntimeError('unimplemented learner family passed')
  report['learners'].append({'family':family,'build':'passed','rejected_units':rejected});print(f'{family}: all 12 unfinished units rejected',flush=True)
 if not args.skip_mutations:
  mi_mutants=[('abi','abi.c','+8*h','+7*h'),('hostile','sum.S','mi_asm_sum:\n','mi_asm_sum:\n movq $99,%rbx\n'),('archive','archive.c','return 42','return 41'),('relocation','relocation.c','mi_pointer=mi_message','mi_pointer=mi_message+1'),('tls','tls.c','static _Thread_local','static'),('budget','layout.ld','<= 64','<= 256'),('entry','entry.c','sp[0]!=3','sp[0]!=2'),('syscall','syscall.c','return (long)done;','return (long)done-1;'),('bytes','bytes.c','(uintptr_t)a<(uintptr_t)b','1'),('wide','wide.c','return q;','return q+1;'),('closure','audit.c','return 1;','return 0;'),('image','image.c','0xfe','0xfd')]
  for case,file,before,after in mi_mutants:
   destination=work/'mutants'/('mi-'+case);shutil.copytree(ROOT/'assessor/machine_image',destination,dirs_exist_ok=True);p=destination/file;s=p.read_text();p.write_text(mutate(s,before,after),encoding='utf-8');b=configure('machine_image','mutant-mi-'+case,assessor=destination);code,_=run(['ctest','--test-dir',b,'-R',r'^machine_image\.'+case+'$','--output-on-failure'],b/'tests.log',expected=None)
   if code==0:raise RuntimeError('mutation survived: machine '+case)
   report['mutations'].append({'family':'machine_image','case':case,'status':'rejected','fault':before+' => '+after})
  hs_mutants=[('descriptor','return fcntl(fd,F_DUPFD_CLOEXEC,0);','return dup(fd);'),('copy','*moved+=done;','*moved+=done+1;'),('deadline','return -3;','return -2;'),('process','*observed=WEXITSTATUS(status);','*observed=status;'),('mapping','offset=off;','offset=0;'),('frame','head[0]<<8)|head[1]','head[1]<<8)|head[0]'),('signal','SIG_SETMASK,&old','SIG_BLOCK,&old'),('ipc','i<r/2','i<0'),('reactor','total+=x;','total+=x+1;'),('pool','*sum=total;','*sum=total+1;'),('queue',']=value;',']=value+1;'),('clocks','(rlim_t)limit,(rlim_t)limit','(rlim_t)limit+1,(rlim_t)limit+1')]
  for case,before,after in hs_mutants:
   destination=work/'mutants'/('hs-'+case);shutil.copytree(ROOT/'assessor/hosted_systems',destination,dirs_exist_ok=True);p=destination/(case+'.c');s=p.read_text();p.write_text(mutate(s,before,after),encoding='utf-8');b=configure('hosted_systems','mutant-hs-'+case,assessor=destination);code,_=run(['ctest','--test-dir',b,'-R',r'^hosted_systems\.'+case+'$','--output-on-failure'],b/'tests.log',expected=None)
   if code==0:raise RuntimeError('mutation survived: hosted '+case)
   report['mutations'].append({'family':'hosted_systems','case':case,'status':'rejected','fault':before+' => '+after})
  print('24 deliberately faulty implementations rejected after successful builds',flush=True)
except Exception as error:
 report['failures'].append(str(error));print(str(error),file=sys.stderr)
finally:
 report['status']='failed' if report['failures'] else 'passed';(work/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print('Report: '+str(work/'report.json'),flush=True)
sys.exit(bool(report['failures']))
