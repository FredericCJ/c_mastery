"""Execute PR-T32-01 original/changed contracts; no physical target inference."""
import argparse,gzip,hashlib,importlib.util,json,os,re,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/testing_depth/architecture';CORE=ROOT/'assessor/testing_depth/architecture.c';BASE=ROOT/'reviews/v5/expansion/legacy_testing_architecture'
PROFILES=(('gcc-debug','gcc',['-O0','-g']),('gcc-release','gcc',['-O3']),('gcc-asan','gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']),('clang-debug','clang',['-O0','-g']),('clang-release','clang',['-O3']),('clang-asan','clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']))
VARIANTS=(0,1,2,3,4,5,'stub')
AUDITS={'plain':[],'asan':['-fsanitize=address,undefined'],'coverage':['--coverage'],'trap':['-fsanitize=undefined','-fsanitize-trap=undefined']}
def load(name):
 s=importlib.util.spec_from_file_location('ta_'+name,PUBLIC/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
builder=load('build');oracle=load('oracle')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return sorted([Path(__file__).resolve(),CORE,ROOT/'assessor/testing_depth/architecture.md',*[p for p in PUBLIC.rglob('*') if p.is_file() and '__pycache__' not in p.parts]])
def audit_commands(kind,work):
 flags=AUDITS[kind];obj=work/(kind+'.o');exe=work/kind;linkmap=work/(kind+'.map')
 compile=['gcc','-std=c17','-O1','-g','-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-stack-protector',*flags,'-I'+str(PUBLIC),'-c',str(CORE),'-o',str(obj)]
 link=['gcc',*flags,'-no-pie','-I'+str(PUBLIC),str(PUBLIC/'tests.c'),str(obj),'-Wl,-Map='+str(linkmap),'-o',str(exe)]
 return [compile,['nm','-u',str(obj)],link,['nm','-u',str(exe)],[str(exe)],*([['gcc','-nostdlib','-nostartfiles','-nodefaultlibs','-no-pie','-Wl,-e,ta_init',str(obj),'-o',str(work/(kind+'-component.elf'))],['nm','-u',str(work/(kind+'-component.elf'))]] if kind in ('plain','trap') else [])]
def audit_result(kind,outputs,maptext):
 symbols=outputs[1]
 if kind in ('plain','trap') and (symbols.strip() or outputs[-1].strip()):raise ValueError('Unexpected component runtime dependency')
 if kind=='asan' and ('__asan_' not in symbols or '__ubsan_' not in symbols or 'libasan' not in maptext):raise ValueError('Missing actual sanitizer instrumentation')
 if kind=='coverage' and ('__gcov_' not in symbols or 'libgcov' not in maptext):raise ValueError('Missing actual coverage instrumentation')
 if 'malloc' not in outputs[3] or 'free' not in outputs[3]:raise ValueError('Missing harness allocator attribution')
 if 'TA_PASS:' not in outputs[4]:raise ValueError('Audit harness did not pass')
 return {'kind':kind,'component_undefined':symbols.splitlines(),'harness_allocator':True,'no_library_link':kind in ('plain','trap'),'scope':'Native x86-64 generated object/link evidence; host verification runtimes not target permissions'}
def semantic(out,code,positive,marker='TA'):
 if code!=(0 if positive else 1) or marker+('_PASS:' if positive else '_CHECK:') not in out or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError('Wrong intended semantic outcome')
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use testing architecture evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};v={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Finite list oracle, four representations, actual local Linux sockets and native object/runtime audit; no physical peripheral or learner inference'}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(cmd,label,expected=0,compressed=False):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,capture_output=True,text=True,env=env,timeout=60);out=r.stdout+r.stderr;p=logs/(str(len(v['commands'])).zfill(4)+'-'+label+('.log.gz' if compressed else '.log'))
  if compressed:p.write_bytes(gzip.compress(out.encode(),mtime=0))
  else:p.write_text(out,encoding='utf-8',newline='\n')
  row={'command':cmd,'exit_code':r.returncode,'log':p.relative_to(ROOT).as_posix()};v['commands'].append(row)
  if r.returncode!=expected:raise ValueError(label+': '+out[:6000])
  return out,row
 try:
  v['toolchains']={c:run([c,'--version'],c)[0].splitlines()[0] for c in ('gcc','clang','nm')}
  with tempfile.TemporaryDirectory(prefix='testing-architecture-') as d:
   work=Path(d)
   for profile,cc,flags in PROFILES:
    for variant in VARIANTS:
     name=profile+'-'+str(variant);exe=work/name;source=PUBLIC/'architecture.c' if variant=='stub' else CORE
     _,b=run(builder.command(cc,flags,source,exe,mutant=0 if variant=='stub' else variant),name+'-build');out,e=run([exe],name,0 if variant==0 else 1);semantic(out,e['exit_code'],variant==0);v['observations'].append({'target':'core','profile':profile,'variant':variant,'build':b,'execution':e,'passed':True})
    for test in ('replay','socket'):
     name=profile+'-'+test;exe=work/name;_,b=run(builder.command(cc,flags,CORE,exe,test+'.c'),name+'-build');out,e=run([exe],name,compressed=test=='replay')
     result=oracle.check(out) if test=='replay' else semantic(out,e['exit_code'],True,'TA_SOCKET')
     v['observations'].append({'target':test,'profile':profile,'build':b,'execution':e,'expected':result,'passed':True})
   for kind in AUDITS:
    rows=[];outputs=[]
    for command in audit_commands(kind,work):
     out,row=run(command,'audit-'+kind);outputs.append(out);rows.append(row)
    text=(work/(kind+'.map')).read_text(encoding='utf-8');p=logs/(kind+'.map');p.write_text(text,encoding='utf-8',newline='\n');result=audit_result(kind,outputs,text);v['observations'].append({'target':'audit','kind':kind,'commands':rows,'map':p.relative_to(ROOT).as_posix(),'expected':result,'passed':True})
 except (OSError,ValueError,subprocess.SubprocessError) as e:v['error']=str(e)
 v['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());v['passed']='error' not in v and v['source_stable'] and len(v['observations'])==58;v['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for p in (destination,logs/'campaign.json'):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':v['passed'],'commands':len(v['commands']),'observations':len(v['observations']),'error':v.get('error')}));return 0 if v['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
