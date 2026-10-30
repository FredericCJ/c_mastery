"""Exact retry criterion harness with validated reuse of existing service evidence."""
import argparse,hashlib,json,platform,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from check_service_adapters_v5 import validate
ROOT=Path(__file__).resolve().parents[1]
def run(report):
 base=ROOT/'reviews/v5/expansion/legacy_posix_retry';dest=(ROOT/report).resolve()
 if not dest.is_relative_to(base.resolve()):raise ValueError('Use POSIX retry evidence directory')
 public=ROOT/'labs/posix_lifecycle_depth/retry';receipt=ROOT/'assessor/service_depth/kernel/receipt.c';store=ROOT/'assessor/storage/store.c'
 parent_path=ROOT/'reviews/v5/expansion/integrated/service_depth_campaign.json';parent=json.loads(parent_path.read_text(encoding='utf-8'));binding=parent['adapter_campaign'];adapter_path=ROOT/binding['path'];adapter=json.loads(adapter_path.read_text(encoding='utf-8'))
 if not parent.get('passed') or hashlib.sha256(adapter_path.read_bytes()).hexdigest()!=binding['sha256']:raise ValueError('Parent adapter binding stale')
 errors=validate(adapter)
 if errors:raise ValueError('Current nested service evidence invalid: '+str(errors))
 reused=[r for r in adapter['runs'] if r['id'] in ('receipt-host','receipt-guests')]
 paths=[Path(__file__).resolve(),ROOT/'tools/check_service_adapters_v5.py',receipt,store,ROOT/'labs/service_depth/adapters/receipt.h',ROOT/'labs/service_depth/adapters/receipt.c',ROOT/'labs/storage/include/store.h',ROOT/'labs/service_depth/adapters/CONTRACTS.md',ROOT/'assessor/posix_lifecycle_depth/retry/ANALYSIS.md',parent_path,adapter_path,*[ROOT/r['path'] for r in reused]]+[p for p in public.iterdir() if p.is_file()]
 pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
 stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ');logs=base/'runs'/stamp;logs.mkdir(parents=True)
 result={'schema_version':1,'created_utc':stamp,'inspected_paths':pins,'profiles':[],'environment':platform.uname()._asdict(),'reused_evidence':{'adapter':binding,'receipts':reused,'validation':'Current nested source/command/semantic controls checked by check_service_adapters_v5.validate; no guest or old host campaign rerun.'},'scope':'Original lost-reply/expiry/split-publication criterion; two finite public device models, explicit existing store/receipt imports. No physical validation or learner assessment.'}
 try:
  with tempfile.TemporaryDirectory(prefix='posix-retry-',dir='/var/tmp') as directory:
   for cc in ('gcc','clang'):
    for mode,flags in [('debug',['-O0','-g']),('release',['-O3']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
     name=cc+'-'+mode;outcomes=[]
     for variant in ('reference','SDK_MUTANT_REAPPLY','SDK_MUTANT_SPLIT_RECEIPT','SDK_MUTANT_PAYLOAD_BINDING','learner'):
      label=name+'-'+variant;exe=Path(directory)/label;impl=ROOT/'labs/service_depth/adapters/receipt.c' if variant=='learner' else receipt
      command=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-I'+str(ROOT/'labs/service_depth/adapters'),'-I'+str(ROOT/'labs/storage/include'),str(public/'tests.c'),str(impl),str(store),'-o',str(exe)]
      if variant.startswith('SDK_'):command.append('-D'+variant)
      build=subprocess.run(command,capture_output=True,text=True,timeout=90);(logs/(label+'-build.log')).write_text(build.stdout+build.stderr,encoding='utf-8',newline='\n')
      if build.returncode:raise RuntimeError(label+' compile failure: '+build.stderr)
      observed=subprocess.run([str(exe)],capture_output=True,text=True,timeout=30);(logs/(label+'.log')).write_text(observed.stdout+observed.stderr,encoding='utf-8',newline='\n')
      marker={'SDK_MUTANT_REAPPLY':'out==7','SDK_MUTANT_SPLIT_RECEIPT':'out==7','SDK_MUTANT_PAYLOAD_BINDING':'SDR_CONFLICT','learner':'sdr_initialize'}.get(variant)
      passed=(observed.returncode==0 and 'PR_PASS' in observed.stdout and 'cuts=74 events=37' in observed.stdout) if variant=='reference' else (observed.returncode==1 and 'PL_CHECK:' in observed.stderr and ''.join(marker.split()) in ''.join(observed.stderr.split()) and 'Sanitizer' not in observed.stderr)
      outcomes.append({'variant':variant,'command':command,'build_exit':build.returncode,'execution_exit':observed.returncode,'passed':passed,'observations':observed.stdout.splitlines(),'semantic_marker':marker})
      if not passed:raise RuntimeError(label+' semantic failure: '+observed.stdout+observed.stderr)
     result['profiles'].append({'name':name,'compiler':subprocess.check_output([cc,'--version'],text=True).splitlines()[0],'outcomes':outcomes,'passed':True})
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:result['error']=str(error)
 result['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items());result['passed']='error' not in result and result['source_stable'] and len(result['profiles'])==6
 result['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()};payload=json.dumps(result,indent=2)+'\n';(logs/'report.json').write_text(payload,encoding='utf-8',newline='\n');dest.write_text(payload,encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'profiles':len(result['profiles']),'error':result.get('error')}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();raise SystemExit(run(a.report))
