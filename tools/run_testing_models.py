"""Execute exact T32 model/reducer/metamorphic and controlled native schedules."""
import argparse,gzip,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_testing_architecture import PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/testing_depth/models';CORE=ROOT/'assessor/testing_depth/models.c';PROVIDER=ROOT/'assessor/testing_depth/architecture.c';INCLUDE=ROOT/'labs/testing_depth/architecture';BASE=ROOT/'reviews/v5/expansion/legacy_testing_models'
VARIANTS=(0,1,2,3,4,5,6,'stub')
def load(name):
 s=importlib.util.spec_from_file_location('tm_'+name,PUBLIC/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
builder=load('build');oracle=load('oracle')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return sorted([Path(__file__).resolve(),ROOT/'tools/run_testing_architecture.py',CORE,ROOT/'assessor/testing_depth/models.md',PROVIDER,INCLUDE/'architecture.h',INCLUDE/'CONTRACTS.md',*[p for p in PUBLIC.rglob('*') if p.is_file() and '__pycache__' not in p.parts]])
def companion(cc,flags,test,exe,repair=1):return [cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-pthread','-DTM_SCHEDULE_REPAIR='+str(repair),str(PUBLIC/(test+'.c')),'-o',str(exe)]
def semantic(out,code,positive):
 if code!=(0 if positive else 1) or ('TM_PASS:' if positive else 'TM_CHECK:') not in out or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError('Wrong intended model semantic result')
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use models evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False);pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};v={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Original finite models, exact rational arithmetic and controlled actual pthread schedules; no physical/all-execution/learner proof'};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(cmd,label,expected=0,compressed=False):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,capture_output=True,text=True,env=env,timeout=60);out=r.stdout+r.stderr;p=logs/(str(len(v['commands'])).zfill(4)+'-'+label+('.log.gz' if compressed else '.log'))
  if compressed:p.write_bytes(gzip.compress(out.encode(),mtime=0))
  else:p.write_text(out,encoding='utf-8',newline='\n')
  row={'command':cmd,'exit_code':r.returncode,'log':p.relative_to(ROOT).as_posix()};v['commands'].append(row)
  if expected is not None and r.returncode!=expected or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError(label+': '+out[:5000])
  return out,row
 try:
  v['toolchains']={cc:run([cc,'--version'],cc)[0].splitlines()[0] for cc in ('gcc','clang')}
  with tempfile.TemporaryDirectory(prefix='testing-models-') as d:
   work=Path(d)
   for profile,cc,flags in PROFILES:
    for variant in VARIANTS:
     name=profile+'-'+str(variant);exe=work/name;source=PUBLIC/'models.c' if variant=='stub' else CORE;_,b=run(builder.command(cc,flags,source,PROVIDER,INCLUDE,exe,mutant=0 if variant=='stub' else variant),name+'-build');out,e=run([exe],name,0 if variant==0 else 1);semantic(out,e['exit_code'],variant==0);v['observations'].append({'target':'core','profile':profile,'variant':variant,'build':b,'execution':e,'passed':True})
    exe=work/(profile+'-replay');_,b=run(builder.command(cc,flags,CORE,PROVIDER,INCLUDE,exe,'replay.c'),profile+'-replay-build');out,e=run([exe],profile+'-replay',compressed=True);v['observations'].append({'target':'replay','profile':profile,'build':b,'execution':e,'expected':oracle.check(out),'passed':True})
    trace=work/(profile+'-trace');bad=work/(profile+'-bad');good=work/(profile+'-good');correlated=work/(profile+'-correlated');builds=[]
    for cmd in (builder.command(cc,flags,CORE,PROVIDER,INCLUDE,trace,'trace.c',1),companion(cc,flags,'schedule',bad,0),companion(cc,flags,'schedule',good),companion(cc,flags,'correlated',correlated)):
     builds.append(run(cmd,profile+'-investigation-build')[1])
    begin=len(v['commands'])
    def observed(cmd):
     out,row=run(cmd,profile+'-investigation',expected=None);return out,row['exit_code']
    result=oracle.investigate(trace,bad,good,correlated,observed);saved={}
    for p in (trace,bad,good,correlated):
     target=logs/p.name;target.write_bytes(p.read_bytes());saved[str(p)]=target.relative_to(ROOT).as_posix()
    v['observations'].append({'target':'investigation','profile':profile,'builds':builds,'executions':v['commands'][begin:],'artifacts':saved,'expected':result,'passed':True})
 except (OSError,ValueError,subprocess.SubprocessError) as e:v['error']=str(e)
 v['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());v['passed']='error' not in v and v['source_stable'] and len(v['observations'])==60;v['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for p in (destination,logs/'campaign.json'):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':v['passed'],'commands':len(v['commands']),'observations':len(v['observations']),'error':v.get('error')}));return 0 if v['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
