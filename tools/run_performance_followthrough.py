"""T43 immutable author execution producer. No host policy changes."""
import argparse,hashlib,json,os,re,subprocess,sys,time
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/performance_followthrough';REF=ROOT/'assessor/performance_followthrough';BASE=ROOT/'reviews/v5/expansion/legacy_performance_followthrough';sys.path.insert(0,str(LAB))
import analyze
RETAINED=['assessor/foundations/foundation.c','labs/foundations/include/foundation.h','labs/foundations/src/foundation.c','assessor/containers_depth/sequence.c','assessor/containers_depth/indexed.c','assessor/containers_depth/graph_consumer.c','labs/containers_depth/containers.h','labs/containers_depth/learner.c','assessor/algorithms/algo.c','assessor/algorithms/x86_64.S','labs/algorithms/include/algo.h','labs/algorithms/src/algo.c','labs/algorithms/src/x86_64.S']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return sorted({p for p in [*LAB.rglob('*'),*REF.rglob('*'),*[ROOT/p for p in RETAINED],*(ROOT/'fixtures/performance').glob('*.json'),Path(__file__)] if p.is_file() and '__pycache__' not in p.parts})
def main(report):
 dest=(ROOT/report).resolve();assert dest.is_relative_to(BASE.resolve()) and not dest.exists();out=dest.with_suffix('');out.mkdir(parents=True,exist_ok=False);before={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};commands=[];checks=[];artifacts=set();observations={}
 def run(label,command,code=0,timeout=60,cwd=ROOT):
  command=list(map(str,command));start=time.monotonic();r=subprocess.run(command,cwd=cwd,capture_output=True,text=True,timeout=timeout);log=out/(label+'.txt');log.write_text(r.stdout+r.stderr);artifacts.add(log);commands.append({'label':label,'command':command,'cwd':str(cwd),'exit_code':r.returncode,'expected_exit_code':code,'seconds':time.monotonic()-start,'output':log.relative_to(ROOT).as_posix()});assert r.returncode==code,(label,r.returncode,r.stdout,r.stderr);return r.stdout+r.stderr
 def check(name,value):checks.append({'name':name,'passed':bool(value)});assert value,name
 def artifact(path):artifacts.add(path);return path
 try:
  for tool in ('gcc','clang','gdb','gprof'):run(tool+'-version',[tool,'--version'])
  run('kernel',['uname','-a']);run('cpu',['lscpu']);run('filesystem',['findmnt','-T','/tmp','-o','TARGET,SOURCE,FSTYPE,OPTIONS']);run('cgroup-membership',['cat','/proc/self/cgroup'])
  certificates=json.loads(run('certificates',[sys.executable,LAB/'certificates.py']));check('distinct-certificates',certificates['passed'] and len(certificates['records'])==16);observations['certificates']=certificates
  includes=['-I'+str(ROOT/p) for p in ['labs/performance_followthrough','labs/foundations/include','labs/algorithms/include','labs/containers_depth']]
  core=[LAB/'tests.c',REF/'cost.c',REF/'byte_helpers.c',ROOT/'assessor/foundations/foundation.c',ROOT/'assessor/containers_depth/sequence.c'];common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*includes]
  for cc in ('gcc','clang'):
   for profile,flags in [('debug',['-O0','-g']),('release',['-O2','-DNDEBUG'])]:
    prefix=cc+'-'+profile;exe=artifact(out/(prefix+'-core'));run(prefix+'-core-build',[cc,*common,*flags,*core,'-o',exe]);text=run(prefix+'-core',[exe]);check(prefix+'-mechanisms','PF_PASS:all' in text and '9331' in text and '1093' in text)
   exe=artifact(out/(cc+'-graph'));run(cc+'-graph-build',[cc,*common,'-O2',LAB/'graph.c',ROOT/'assessor/algorithms/algo.c',ROOT/'assessor/containers_depth/indexed.c',ROOT/'assessor/containers_depth/graph_consumer.c','-o',exe]);check(cc+'-graph','stream-attempt5-accepted4-prefix' in run(cc+'-graph',[exe]))
   host=artifact(out/(cc+'-host'));run(cc+'-host-build',[cc,*common,'-O2','-g',LAB/'observations.c',ROOT/'assessor/algorithms/x86_64.S','-pthread','-o',host]);actual=[json.loads(s) for s in run(cc+'-host',[host]).splitlines()];check(cc+'-actual-host',len(actual)==10 and next(r for r in actual if r['case']=='child-clock-scope')['waited_child_cpu_us']>0);observations[cc+'-host']=actual
   pairtext=run(cc+'-pairs',[host,'pairs']);observations[cc+'-pairs']=analyze.pairs(pairtext);check(cc+'-31-pairs-three-sizes',len(observations[cc+'-pairs'])==7)
   pmu=[json.loads(s) for s in run(cc+'-pmu',[host,'pmu']).splitlines()];check(cc+'-pmu-status',len(pmu)==4 and all(r['status'] in ('measured','denied','unavailable') for r in pmu));observations[cc+'-pmu']=pmu
   cgroup=json.loads(run(cc+'-cgroup',[sys.executable,LAB/'cgroup_observe.py','--binary',host]));check(cc+'-readonly-cgroup',cgroup['passed'] and cgroup['policy_writes']==cgroup['membership_writes']==0 and not cgroup['same_descriptor_peak_reset']);observations[cc+'-cgroup']=cgroup
   run(cc+'-host-disassembly',['objdump','-d',host]);run(cc+'-host-symbols',['nm','-S',host])
   inline=artifact(out/(cc+'-inline'));run(cc+'-inline-build',[cc,*common,'-O3','-DPF_ALLOW_INLINE',LAB/'observations.c',ROOT/'assessor/algorithms/x86_64.S','-pthread','-o',inline]);observations[cc+'-inline-pairs']=analyze.pairs(run(cc+'-inline-pairs',[inline,'pairs']));run(cc+'-inline-disassembly',['objdump','-d',inline]);run(cc+'-inline-symbols',['nm','-S',inline])
   obj=artifact(out/(cc+'-cost.o'));run(cc+'-cost-object',[cc,*common,'-O2','-ffreestanding','-fno-builtin','-U_FORTIFY_SOURCE','-fstack-usage','-c',REF/'cost.c','-o',obj]);run(cc+'-cost-undefined',['nm','-u',obj])
   helper=artifact(out/(cc+'-helper.o'));run(cc+'-helper-object',[cc,*common,'-O2','-ffreestanding','-fno-builtin','-c',REF/'byte_helpers.c','-o',helper]);closed=artifact(out/(cc+'-closed.o'));run(cc+'-closed-object',[cc,'-r','-nostdlib',obj,helper,'-o',closed]);check(cc+'-closed-dependencies',not run(cc+'-closed-undefined',['nm','-u',closed]).strip())
   for lto in (False,True):
    prefix=cc+('-lto' if lto else '-stack');exe=artifact(out/prefix);flags=['-flto'] if lto else ['-fstack-usage'];flags+=(['-fuse-ld=/home/frede/cmastery-b06-tools/usr/bin/ld.lld-21'] if lto and cc=='clang' else []);flags+=(['-fno-ipa-cp'] if cc=='gcc' else [])
    run(prefix+'-build',[cc,'-std=c17','-O2','-g',*flags,LAB/'stack.c','-o',exe]);run(prefix,[exe]);dis=run(prefix+'-disassembly',['objdump','-d',exe]);observations[prefix+'-callgraph']=analyze.callgraph(dis);run(prefix+'-symbols',['nm','-S',exe])
   for mode in ('base','hook','drop','pg'):
    prefix=cc+'-'+mode;folder=out/prefix;folder.mkdir();exe=artifact(folder/'profile');flags=['-O2','-g','-fno-pie','-fno-omit-frame-pointer','-fstack-usage']+(['-finstrument-functions','-DPF_HOOKS'] if mode in ('hook','drop') else ['-pg'] if mode=='pg' else []);objects=[]
    for name,path,extra in [('main',LAB/'profile_main.c',flags),('kernel',ROOT/'assessor/algorithms/algo.c',flags),('helper',LAB/'profile_helper.c',['-O2','-g','-fno-pie','-fstack-usage'])]+([('recorder',REF/'recorder.c',['-O2','-g','-fno-pie','-fstack-usage']+(['-DPF_RECORD_CAP=4'] if mode=='drop' else []))] if mode in ('hook','drop') else []):
     obj=artifact(folder/(name+'.o'));objects.append(obj);run(prefix+'-'+name+'-compile',[cc,*common,*extra,'-c',path,'-o',obj])
    run(prefix+'-link',[cc,'-no-pie',*objects,*(['-pg'] if mode=='pg' else []),'-o',exe]);text=run(prefix,[exe],cwd=folder);syms=run(prefix+'-symbols',['nm','-S',exe]);run(prefix+'-disassembly',['objdump','-d',exe]);run(prefix+'-size',['size',exe]);check(prefix+'-normal-run','PROFILE_RUN:' in text and 'CALLERS:2:' in text)
    runtime=list(map(int,re.search(r'PROFILE_RUN:(\d+):(\d+)',text).groups()));callers=list(map(int,re.search(r'CALLERS:2:(\d+):8:(\d+):(\d+)',text).groups()));sym=analyze.symbols(syms);observations[prefix+'-perturbation']={'elapsed_ns':runtime[0],'checksum':runtime[1],'caller_expensive_2_ns':callers[0],'caller_cheap_8_ns':callers[1],'algo_lower_code_bytes':sym['algo_lower']['bytes'],'child_code_bytes':sym['pf_child']['bytes'],'clock_scope':'elapsed including instrumentation;notCPU','single_trial_not_statistical':True}
    if mode in ('hook','drop'):
     observations[prefix]=analyze.hooks(text,syms);check(prefix+'-bounded-trace',observations[prefix]['complete']==(mode=='hook') and observations[prefix]['uninstrumented_helper_boundary_absent'])
    if mode=='pg':
     gmon=artifact(folder/'gmon.out');check(prefix+'-gmon-normal-exit',gmon.is_file() and gmon.stat().st_size>0);gp=run(prefix+'-gprof',['gprof','-b',exe,gmon]);check(prefix+'-gprof-kernel-count',bool(re.search(r'\b1000\s+[^\n]*algo_lower',gp)));observations[prefix+'-gprof']={'no_time_samples_accumulated':'no time accumulated' in gp,'call_counts_available':True,'sampled_time_not_exact_wall':True}
  trace=artifact(out/'trace');run('trace-build',['gcc',*common,'-O0','-g',LAB/'trace_main.c',ROOT/'assessor/algorithms/algo.c',ROOT/'assessor/algorithms/x86_64.S','-o',trace]);observations['actual-static-CFG']=analyze.lower_cfg(run('trace-disassembly',['objdump','-d','--disassemble=algo_asm_lower',trace]));run('trace-symbols',['nm','-S',trace])
  for name,n,key in [('empty',0,2),('duplicate',8,2),('index4a',8,3),('index4b',7,3)]:
   text=run('trace-'+name,['gdb','-q','-batch','-x',LAB/'trace_gdb.py','--args',trace,str(n),str(key)],timeout=60);observations['trace-'+name]=analyze.trace(text)
  check('actual-changed-path',observations['trace-index4a']['result']==observations['trace-index4b']['result']==4 and observations['trace-index4a']['relative_pc_path']!=observations['trace-index4b']['relative_pc_path']);check('same-static-kernel-different-work',observations['trace-empty']['instructions']<observations['trace-duplicate']['instructions'])
  for name,old,new in [('stability','compare(a[j-1],x,w)>0','compare(a[j-1],x,w)>=0'),('flush','v.slots[i].dirty=0','v.slots[i].dirty=1'),('borrow','--sec;ns=1000000000u-n+m','ns=1000000000u-n+m'),('wrap','c.bound>=UINT64_C(4294967296)','c.bound>UINT64_C(4294967296)')]:
   source=(REF/'cost.c').read_text();assert source.count(old)==1;mutant=artifact(out/(name+'-mutant.c'));mutant.write_text(source.replace(old,new));exe=artifact(out/(name+'-mutant'));run(name+'-mutant-build',['gcc',*common,'-O2',LAB/'tests.c',mutant,ROOT/'assessor/foundations/foundation.c',ROOT/'assessor/containers_depth/sequence.c','-o',exe]);check(name+'-mutant-killed','PF_CHECK:' in run(name+'-mutant',[exe],code=1))
  stub=artifact(out/'stub');run('stub-build',['gcc',*common,'-O2',LAB/'tests.c',LAB/'cost.c',ROOT/'assessor/foundations/foundation.c',ROOT/'assessor/containers_depth/sequence.c','-o',stub]);check('stub-remains-incomplete','PF_CHECK:' in run('stub',[stub],code=1))
  for p in out.rglob('*.su'):artifacts.add(p)
  check('three-stack-qualifiers',any('dynamic,bounded' in p.read_text() for p in artifacts if p.suffix=='.su') and any('\tdynamic\n' in p.read_text() for p in artifacts if p.suffix=='.su'))
  check('source-stable',before=={p.relative_to(ROOT).as_posix():sha(p) for p in sources()})
 except Exception as e:passed=False;error=repr(e)
 else:passed=all(c['passed'] for c in checks);error=None
 result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':passed,'error':error,'commands':commands,'checks':checks,'observations':observations,'inspected_paths':before,'artifact_sha256':{p.relative_to(ROOT).as_posix():sha(p) for p in sorted(artifacts) if p.is_file()},'open_gates':['Independent review and minimum-seven-day unaided learner assessment unobserved.','Hardware PMU unavailable if recorded;no host policy changes.','No owned prepared delegated cgroup subtree;conditional experiment remains external.','No physical board/counter/energy/WCET observation.']};dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':passed,'commands':len(commands),'checks':len(checks),'error':error}));return 0 if passed else 1
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
