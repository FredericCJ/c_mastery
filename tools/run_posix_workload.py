"""Equivalent record contracts, changed access patterns and measured warm samples."""
import argparse,hashlib,json,platform,statistics,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from check_posix_followthrough import fresh
ROOT=Path(__file__).resolve().parents[1]
def run(report):
 base=ROOT/'reviews/v5/expansion/legacy_posix_workload';dest=(ROOT/report).resolve()
 if not dest.is_relative_to(base.resolve()):raise ValueError('Use workload evidence directory')
 public=ROOT/'labs/posix_lifecycle_depth/workload';private=ROOT/'assessor/posix_lifecycle_depth/workload';mapping=ROOT/'assessor/posix_lifecycle_depth/mapping/map.c';include=ROOT/'labs/posix_lifecycle_depth/mapping'
 fault_path=ROOT/'reviews/v5/expansion/legacy_posix_mapping/mapping_validation.json';fault=json.loads(fault_path.read_text(encoding='utf-8'));fresh(fault,114,6)
 fault_cases=[{'profile':p['name'],'outcome':o} for p in fault['profiles'] for o in p['outcomes'] if o['case']=='faults' and o['variant']=='reference']
 if len(fault_cases)!=6:raise ValueError('Missing reused isolated process-fault evidence')
 paths=[Path(__file__).resolve(),ROOT/'tools/check_posix_followthrough.py',mapping,include/'map.h',fault_path]+[p for d in (public,private) for p in d.iterdir() if p.is_file()]
 pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths};stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ');logs=base/'runs'/stamp;logs.mkdir(parents=True)
 result={'schema_version':1,'created_utc':stamp,'inspected_paths':pins,'profiles':[],'environment':platform.uname()._asdict(),'reused_fault_evidence':{'path':fault_path.relative_to(ROOT).as_posix(),'sha256':hashlib.sha256(fault_path.read_bytes()).hexdigest(),'reference_fault_outcomes':len(fault_cases),'scope':'Previously executed isolated SIGSEGV/SIGBUS controls; source/log freshness verified, not rerun here.'},'scope':'Same logical record bounds and unchanged-output errors, sequential versus sparse random records. Warm prevalidated data, setup excluded and reported. No physical disk, cold-cache or universal speed claim.'}
 try:
  with tempfile.TemporaryDirectory(prefix='posix-workload-',dir='/var/tmp') as directory:
   for cc in ('gcc','clang'):
    for mode,flags in [('debug',['-O0','-g']),('release',['-O3']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
     name=cc+'-'+mode;outcomes=[];samples=[]
     for case,variants in [('record',('reference','mutant1','mutant2','mutant3','learner')),('bench',('reference','weak_mapping'))]:
      for variant in variants:
       label=name+'-'+case+'-'+variant;exe=Path(directory)/label;impl=public/'record_stub.c' if variant=='learner' else private/'record.c';command=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-I'+str(public),str(public/('tests.c' if case=='record' else 'bench.c')),str(impl),'-Wl,--wrap=pread,--wrap=__pread_chk','-o',str(exe)]
       if case=='bench':command+=['-I'+str(include),str(mapping)]
       if variant.startswith('mutant'):command.append('-DPW_MUTANT='+variant.removeprefix('mutant'))
       if variant=='weak_mapping':command.append('-DPM_MUTANT=1')
       build=subprocess.run(command,capture_output=True,text=True,timeout=60);(logs/(label+'-build.log')).write_text(build.stdout+build.stderr,encoding='utf-8',newline='\n')
       if build.returncode:raise RuntimeError(label+' compilation: '+build.stderr)
       observed=subprocess.run([str(exe)],capture_output=True,text=True,timeout=30);(logs/(label+'.log')).write_text(observed.stdout+observed.stderr,encoding='utf-8',newline='\n')
       passed=(observed.returncode==0 and 'PW_PASS' in observed.stdout) if variant=='reference' else (observed.returncode==1 and 'PL_CHECK:' in observed.stderr and 'Sanitizer' not in observed.stderr)
       observations=observed.stdout.splitlines()
       if case=='bench' and variant=='reference':
        for line in observations:
         if line.startswith('PW_SAMPLE '):samples.append({k:(v if k in ('method','workload') else int(v)) for k,v in (item.split('=',1) for item in line.split()[1:])})
        if len(samples)!=32:passed=False
        for workload in ('sequential','sparse_random'):
         for method in ('pread','mapped'):
          selected=[s for s in samples if s['workload']==workload and s['method']==method]
          if len(selected)!=8 or {s['trial'] for s in selected}!=set(range(8)) or any(s['pread_calls']!=(8192 if method=='pread' else 0) or s['records']!=8192 or s['bytes']!=262144 for s in selected):passed=False
         if len({s['digest'] for s in samples if s['workload']==workload})!=1:passed=False
       outcomes.append({'case':case,'variant':variant,'command':command,'build_exit':build.returncode,'execution_exit':observed.returncode,'passed':passed,'observations':observations})
       if not passed:raise RuntimeError(label+' semantic result: '+observed.stdout+observed.stderr)
     summary=[{'workload':w,'method':m,'wall_ns_min':min(v),'wall_ns_median':statistics.median(v),'wall_ns_max':max(v),'interpretation':'Observed distribution only; no enforced speed ordering.'} for w in ('sequential','sparse_random') for m in ('pread','mapped') for v in [[s['wall_ns'] for s in samples if s['workload']==w and s['method']==m]]]
     result['profiles'].append({'name':name,'compiler':subprocess.check_output([cc,'--version'],text=True).splitlines()[0],'outcomes':outcomes,'samples':samples,'sample_summary':summary,'passed':True})
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:result['error']=str(error)
 result['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items());result['passed']='error' not in result and result['source_stable'] and len(result['profiles'])==6;result['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()};payload=json.dumps(result,indent=2)+'\n';(logs/'report.json').write_text(payload,encoding='utf-8',newline='\n');dest.write_text(payload,encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'profiles':len(result['profiles']),'error':result.get('error')}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();raise SystemExit(run(a.report))
