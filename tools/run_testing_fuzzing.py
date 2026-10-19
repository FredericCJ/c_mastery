"""Original local packet properties, finite libFuzzer and isolated diagnostic controls."""
import argparse,gzip,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_testing_architecture import PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/testing_depth/fuzzing';CORE=ROOT/'assessor/testing_depth/fuzzing.c';MODELS=ROOT/'assessor/testing_depth/models.c';QUEUE=ROOT/'assessor/testing_depth/architecture.c';MI=ROOT/'labs/testing_depth/models';QI=ROOT/'labs/testing_depth/architecture';BASE=ROOT/'reviews/v5/expansion/legacy_testing_fuzzing'
VARIANTS=(0,1,2,3,4,'stub')
def load(name):
 s=importlib.util.spec_from_file_location('tf_'+name,PUBLIC/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
builder=load('build');oracle=load('oracle')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return sorted([Path(__file__).resolve(),ROOT/'tools/run_testing_architecture.py',CORE,ROOT/'assessor/testing_depth/fuzzing.md',MODELS,QUEUE,MI/'models.h',MI/'CONTRACTS.md',QI/'architecture.h',QI/'CONTRACTS.md',*[p for p in PUBLIC.rglob('*') if p.is_file() and '__pycache__' not in p.parts]])
def compile_command(cc,flags,source,exe,test='tests.c',mutant=0):return builder.command(cc,flags,source,MODELS,QUEUE,MI,QI,exe,test,mutant)
def diagnostic_command(cc,exe,repaired):return [cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic','-O1','-g','-fsanitize=address','-fno-omit-frame-pointer','-no-pie','-DTF_REPAIRED='+str(repaired),str(PUBLIC/'defect.c'),str(PUBLIC/'triage.c'),'-o',str(exe)]
def semantic(out,code,positive):
 if code!=(0 if positive else 1) or ('TF_PASS:' if positive else 'TF_CHECK:') not in out or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError('Wrong intended ordinary semantic result')
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use original fuzzing evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False);pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};v={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Original local synthetic parser/property/diagnostic targets only; no external targets or exploitability claim'}
 runtime={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=0:exitcode=86:disable_coredump=1:allow_addr2line=1:external_symbolizer_path=/usr/bin/addr2line','UBSAN_OPTIONS':'halt_on_error=1'}
 env=dict(os.environ,**runtime);v['runtime_environment']=runtime;v['symbolizer_sha256']=sha(Path('/usr/bin/addr2line'))
 def run(cmd,label,expected=0,compressed=False,input_text=None,timeout=60):
  cmd=list(map(str,cmd));timedout=False
  try:r=subprocess.run(cmd,input=input_text,capture_output=True,text=True,env=env,timeout=timeout);out=r.stdout+r.stderr;code=r.returncode
  except subprocess.TimeoutExpired as e:
   timedout=True;code=None;out=''.join(x.decode('utf-8',errors='replace') if isinstance(x,bytes) else x or '' for x in (e.stdout,e.stderr))
  p=logs/(str(len(v['commands'])).zfill(4)+'-'+label+('.log.gz' if compressed else '.log'))
  if compressed:p.write_bytes(gzip.compress(out.encode(),mtime=0))
  else:p.write_text(out,encoding='utf-8',newline='\n')
  row={'command':cmd,'exit_code':code,'timed_out':timedout,'timeout_seconds':timeout,'log':p.relative_to(ROOT).as_posix()}
  if input_text is not None:row['stdin_sha256']=hashlib.sha256(input_text.encode()).hexdigest()
  v['commands'].append(row)
  if expected is not None and (code!=expected or timedout):raise ValueError(label+': '+out[:5000])
  return out,row
 try:
  v['toolchains']={cc:run([cc,'--version'],cc.replace('/','-'))[0].splitlines()[0] for cc in ('gcc','clang','/usr/bin/addr2line')}
  with tempfile.TemporaryDirectory(prefix='testing-fuzzing-') as d:
   work=Path(d)
   for profile,cc,flags in PROFILES:
    for variant in VARIANTS:
     name=profile+'-'+str(variant);exe=work/name;source=PUBLIC/'fuzzing.c' if variant=='stub' else CORE;_,b=run(compile_command(cc,flags,source,exe,mutant=0 if variant=='stub' else variant),name+'-build');out,e=run([exe],name,0 if variant==0 else 1);semantic(out,e['exit_code'],variant==0);v['observations'].append({'target':'core','profile':profile,'variant':variant,'build':b,'execution':e,'passed':True})
    name=profile+'-replay';exe=work/name;_,b=run(compile_command(cc,flags,CORE,exe,'replay.c'),name+'-build');out,e=run([exe],name,compressed=True,input_text=oracle.stream());v['observations'].append({'target':'replay','profile':profile,'build':b,'execution':e,'expected':oracle.check(out),'passed':True})
   config=json.loads((PUBLIC/'seeds.json').read_text(encoding='utf-8'));corpus=work/'corpus';corpus.mkdir();initial={}
   for seed in config['seeds']:
    p=corpus/seed['id'];p.write_bytes(bytes.fromhex(seed['hex']));initial[p.name]=sha(p)
   flags=['-O1','-g','-fsanitize=fuzzer,address,undefined','-fno-omit-frame-pointer','-no-pie'];exe=work/'fuzzer';_,b=run(compile_command('clang',flags,CORE,exe,'fuzzer.c'),'fuzzer-build');artifacts=work/'findings';artifacts.mkdir();cmd=[exe,corpus,'-runs=20000','-seed=17','-max_len=16','-timeout=2','-rss_limit_mb=512','-artifact_prefix='+str(artifacts)+'/'];out,e=run(cmd,'fuzzer-finite')
   if 'Done 20000 runs' not in out or 'cov:' not in out or 'ERROR:' in out or any(artifacts.iterdir()):raise ValueError('Coverage-guided run did not complete clean declared budget')
   saved=logs/'corpus';saved.mkdir()
   for p in corpus.iterdir():(saved/p.name).write_bytes(p.read_bytes())
   executable=logs/'fuzzer.elf';executable.write_bytes(exe.read_bytes());v['observations'].append({'target':'coverage-guided','build':b,'execution':e,'initial_corpus_sha256':initial,'final_corpus_sha256':{p.name:sha(p) for p in saved.iterdir()},'corpus':saved.relative_to(ROOT).as_posix(),'binary':executable.relative_to(ROOT).as_posix(),'passed':True})
   bad=work/'fuzzer-wrong-value';_,b=run(compile_command('clang',flags,CORE,bad,'fuzzer.c',3),'semantic-fuzzer-build');trigger=logs/'wrong-value-input';trigger.write_bytes(bytes.fromhex('010200010002'));out,e=run([bad,'-runs=1',trigger],'semantic-fuzzer',expected=None)
   if e['exit_code']!=77 or e['timed_out'] or 'TF_FUZZ_SEMANTIC:grammar-or-publication' not in out or 'ERROR: AddressSanitizer' in out:raise ValueError('Wrong noncrashing semantic-oracle diagnostic')
   v['observations'].append({'target':'semantic-oracle','build':b,'execution':e,'input':trigger.relative_to(ROOT).as_posix(),'passed':True})
   for cc in ('gcc','clang'):
    bad=work/(cc+'-bounds');good=work/(cc+'-repaired');builds=[run(diagnostic_command(cc,p,r),cc+'-diagnostic-build')[1] for p,r in ((bad,0),(good,1))];begin=len(v['commands']);inputs={}
    def diagnosis(data):
     name=hashlib.sha256(data).hexdigest();p=logs/('input-'+name);p.write_bytes(data);inputs[name]=p.relative_to(ROOT).as_posix();out,row=run([bad,p],cc+'-diagnosis',expected=None)
     if row['timed_out']:raise ValueError('Unrelated diagnostic timeout')
     if row['exit_code'] not in (0,86):raise ValueError('Unrelated diagnostic process failure')
     return oracle.bounds_diagnostic(out,row['exit_code'])
    reduction=oracle.reduce_bytes(bytes.fromhex(config['intentional_bounds_original']),diagnosis)
    if reduction['minimized_hex']!='0101':raise ValueError('Unexpected diagnosed shortest input')
    repaired=[]
    for data in (bytes.fromhex(reduction['original_hex']),bytes.fromhex(reduction['minimized_hex'])):
     p=logs/('input-'+hashlib.sha256(data).hexdigest());out,row=run([good,p],cc+'-repaired')
     if out.strip()!='TF_TRIAGE_RESULT:1':raise ValueError('Repaired parser did not reject truncated input')
     repaired.append(row)
    saved={}
    for p in (bad,good):
     target=logs/(p.name+'.elf');target.write_bytes(p.read_bytes());saved[str(p)]=target.relative_to(ROOT).as_posix()
    v['observations'].append({'target':'bounds-triage','compiler':cc,'builds':builds,'executions':v['commands'][begin:],'inputs':inputs,'reduction':reduction,'repaired':repaired,'artifacts':saved,'passed':True,'claim':'Observed original heap-buffer-overflow READ of size1 only; no arbitrary execution or privilege claim'})
   exe=work/'timeout-control';cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-O1',PUBLIC/'timeout.c','-o',exe];_,b=run(cmd,'timeout-build');rows=[]
   for data,wanted in ((b'\xff\x00',True),(b'\xff',True),(b'',False),(b'\x00',False)):
    p=logs/('timeout-input-'+hashlib.sha256(data).hexdigest());p.write_bytes(data);out,row=run([exe,p],'timeout-control',expected=None,timeout=0.5)
    if row['timed_out']!=wanted or not wanted and (row['exit_code']!=0 or out.strip()!='TF_TIMEOUT_CONTROL:terminated'):raise ValueError('Wrong original timeout control result')
    rows.append({'input':p.relative_to(ROOT).as_posix(),'execution':row,'expected_timeout':wanted})
   v['observations'].append({'target':'timeout','build':b,'cases':rows,'passed':True,'claim':'Owned original process killed by subprocess timeout; not memory diagnosis or external service attack'})
 except (OSError,ValueError,subprocess.SubprocessError) as e:v['error']=str(e)
 v['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());v['passed']='error' not in v and v['source_stable'] and len(v['observations'])==47;v['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for p in (destination,logs/'campaign.json'):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':v['passed'],'commands':len(v['commands']),'observations':len(v['observations']),'error':v.get('error')}));return 0 if v['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
