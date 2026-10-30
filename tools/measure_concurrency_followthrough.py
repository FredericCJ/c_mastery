"""Repeated native measurements with raw per-operation ledgers; sanitizer runs separate."""
import argparse,csv,hashlib,importlib.util,json,os,random,statistics,subprocess,sys,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from run_concurrency_followthrough import public,SOURCE,PROVIDERS

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def source_pins():
 paths={Path(__file__),ROOT/'tools/run_concurrency_followthrough.py',ROOT/'labs/concurrency_followthrough/run.py',SOURCE/'providers.json'}|set(public.recipe('measure',SOURCE,PROVIDERS))
 paths|={p for name in public.INCLUDES for p in (ROOT/name).glob('*.h')}
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(paths)}
def distribution(values):
 values=sorted(values);n=len(values);assert n
 return dict(count=n,minimum=values[0],median=statistics.median(values),p95=values[(95*n-1)//100],p99=values[(99*n-1)//100],maximum=values[-1])
def metadata(output):
 line=next(x for x in output.splitlines() if x.startswith('CF_MEASURE '));parts=line.split();result={'kind_name':parts[1]}
 for part in parts[2:]:k,v=part.split('=');result[k]=int(v)
 return result
def raw(path,meta):
 with path.open() as stream:rows=list(csv.DictReader(stream))
 kind=meta['kind_name']
 if kind=='queue':
  assert len(rows)==meta['total'] and [int(r['id']) for r in rows]==list(range(meta['total']))
  for r in rows:assert int(r['producer'])<meta['pairs'] and meta['pairs']<=int(r['consumer'])<2*meta['pairs']
  assert all(sum(int(r['consumer'])==who for r in rows)==meta['total']//meta['pairs'] for who in range(meta['pairs'],2*meta['pairs']))
  return dict(enqueue_ns=distribution([int(r['enqueue_ns']) for r in rows]),completion_ns=distribution([int(r['completion_ns']) for r in rows]))
 if kind=='rw':
  assert len(rows)==3*meta['writes'];return {role:distribution([int(r['latency_ns']) for r in rows if r['role']==role]) for role in ('writer','reader0','reader1')}
 assert kind=='pool' and len(rows)==4 and all(int(r['completed'])==meta['iterations'] for r in rows)
 return dict(scans=[int(r['scans']) for r in rows],retries=[int(r['retries']) for r in rows],thread_elapsed_ns=[int(r['elapsed_ns']) for r in rows])

def main():
 p=argparse.ArgumentParser();p.add_argument('--work',required=True,type=Path);a=p.parse_args();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False);before=source_pins();commands=[];observations=[];summary=[];comparisons=[];seed=55010
 env=os.environ.copy();env.update(ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1',TSAN_OPTIONS='halt_on_error=1:exitcode=66')
 for key in ('CPATH','C_INCLUDE_PATH','LIBRARY_PATH','CFLAGS','CPPFLAGS','LDFLAGS'):env.pop(key,None)
 def run(name,cmd):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,env=env,capture_output=True,text=True,timeout=120);text=r.stdout+r.stderr;path=work/(name+'.txt');path.write_text(text);okay=r.returncode==0;commands.append(dict(name=name,command=cmd,returncode=r.returncode,passed=okay,log=path.relative_to(ROOT).as_posix(),sha256=sha(path)));print(name,okay,flush=True)
  if not okay:raise RuntimeError(name+'\n'+text[-4000:])
  return text
 def observe(name,exe,args,cc,repeat,instrumented=False):
  path=work/(name+'.csv');out=run(name,[exe,*args,path]);m=metadata(out);d=raw(path,m);observations.append(dict(name=name,compiler=cc,repeat=repeat,instrumented=instrumented,parameters=args,metadata=m,raw=path.relative_to(ROOT).as_posix(),raw_sha256=sha(path),distribution=d));return m
 try:
  run('uname',['uname','-a']);run('cpu',['lscpu']);run('cache-line',['getconf','LEVEL1_DCACHE_LINESIZE'])
  configurations=[]
  for pairs in (1,4):
   for size in (8,256):
    for affinity in (1,2):
     configurations.append((pairs,size,affinity,0,[0,1,3] if pairs==1 else [1,3]))
  configurations.append((2,8,2,1,[1,3]));configurations.append((4,8,1,0,[2]))
  for cc in ('gcc','clang'):
   run(cc+'-version',[cc,'--version']);exe=work/(cc+'-native');compact=work/(cc+'-compact')
   run(cc+'-native-compile',public.compile_command(cc,['-O3'], 'measure',SOURCE,PROVIDERS,exe));run(cc+'-native-imports',['nm','-u',exe]);run(cc+'-native-disassembly',['objdump','-dr',exe])
   run(cc+'-compact-compile',public.compile_command(cc,['-O3','-DCF_PAD=0'],'measure',SOURCE,PROVIDERS,compact))
   rng=random.Random(seed)
   for repeat in range(5):
    configs=list(configurations);rng.shuffle(configs)
    for pairs,size,affinity,pause,kinds in configs:
     kinds=list(kinds);rng.shuffle(kinds)
     for kind in kinds:
      name=f'{cc}-queue-k{kind}-p{pairs}-b{size}-a{affinity}-s{pause}-r{repeat}';observe(name,exe,['queue',kind,pairs,size,affinity,pause,10000],cc,repeat)
    for size in (8,256):observe(f'{cc}-compact-b{size}-r{repeat}',compact,['queue',0,1,size,2,0,10000],cc,repeat)
    choices=[(kind,pause) for kind in (0,1) for pause in (0,1)];rng.shuffle(choices)
    for kind,pause in choices:observe(f'{cc}-rw-k{kind}-s{pause}-r{repeat}',exe,['rw',kind,pause,1000],cc,repeat)
    for policy in rng.sample([0,1,2],3):observe(f'{cc}-pool-k{policy}-r{repeat}',exe,['pool',policy,10000],cc,repeat)
   # Correctness-only instrumented invocations never enter timing summaries.
   for profile,flags in [('asan',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('tsan',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer'])]:
    instrumented=work/(cc+'-'+profile);run(cc+'-'+profile+'-compile',public.compile_command(cc,flags,'measure',SOURCE,PROVIDERS,instrumented))
    jobs=[['queue',1,4,8,1,1,4000],['queue',3,4,256,2,0,4000],['rw',0,1,500],['rw',1,1,500],['pool',2,1000]]
    for i,args in enumerate(jobs):observe(cc+'-'+profile+'-'+str(i),instrumented,args,cc,-1,True)
  groups={}
  for row in observations:
   if row['instrumented']:continue
   key=json.dumps([row['compiler'],row['parameters'],row['metadata'].get('padded')]);groups.setdefault(key,[]).append(row)
  for key,rows in groups.items():
   assert len(rows)==5 and {r['repeat'] for r in rows}==set(range(5));summary.append(dict(key=json.loads(key),elapsed_ns=distribution([r['metadata']['elapsed_ns'] for r in rows]),runs=[r['name'] for r in rows],uncertainty='five raw repeats; range and empirical quantiles, shared-host interference and clock/virtual-CPU effects not eliminated'))
  for cc in ('gcc','clang'):
   for size in (8,256):
    for affinity in (1,2):
     selected={kind:sorted([r for r in observations if not r['instrumented'] and r['compiler']==cc and r['parameters']==['queue',kind,4,size,affinity,0,10000]],key=lambda x:x['repeat']) for kind in (1,3)}
     ratios=[a['metadata']['elapsed_ns']/b['metadata']['elapsed_ns'] for a,b in zip(selected[1],selected[3])];assert len(ratios)==5
     rng=random.Random(seed);boot=sorted(statistics.median(rng.choices(ratios,k=5)) for _ in range(2000))
     comparisons.append(dict(compiler=cc,payload_bytes=size,allowed_cpus=affinity,threads=8,oversubscribed=True,AP_over_mutex_paired_ratios=ratios,median_ratio=statistics.median(ratios),bootstrap_median_interval=[boot[49],boot[1949]],locking_faster_in_median=statistics.median(ratios)>1,limits='Exploratory paired empirical resampling, not population confidence or guaranteed latency; synchronization, pool collection and try-failure details differ as declared.'))
   assert any(x['compiler']==cc and x['locking_faster_in_median'] for x in comparisons),'No observed locking-faster case; do not invent one.'
 except Exception as e:passed=False;error=str(e)
 else:passed=True;error=None
 after=source_pins()
 if after!=before:passed=False;error='measurement sources changed during campaign'
 report=dict(passed=passed,error=error,commands=commands,observations=observations,summaries=summary,comparisons=comparisons,source_sha256=after,artifacts={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},seed=seed,scope='Bounded local host author measurements. Original AP provider unchanged; all logical messages distinct/exact-once and all workers joined. Queue payload is an eight-byte index/check pair into equal caller-owned immutable bodies. Capacity31 is an upper bound, with AP conservative pool admission distinguished. No host settings, privileged counters, physical cache claim or sanitizer timing inference.')
 (work/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,commands=len(commands),observations=len(observations),groups=len(summary),comparisons=len(comparisons))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
