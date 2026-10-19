"""Inspect raw receipts, exact resource ledgers and unchanged retained providers."""
import argparse,hashlib,json,re,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
_digests={}
def sha(p):
 p=p.resolve();s=p.stat();key=(str(p),s.st_size,s.st_mtime_ns,s.st_ctime_ns)
 if key not in _digests:_digests[key]=hashlib.sha256(p.read_bytes()).hexdigest()
 return _digests[key]
def load(p):return json.loads((ROOT/p).read_text())
def pins(values,root=ROOT):
 for p,h in values.items():assert (root/p).is_file() and sha(root/p)==h,p
def inspect(m,e):
 checks=[]
 def check(name,value):checks.append(dict(name=name,passed=bool(value)))
 pins(m['source_sha256']);pins(m['artifacts']);pins(e['source_sha256']);pins(e['artifacts'])
 positives=[x for x in m['components'] if not x.get('target') and not x['mutant'] and not x['learner']]
 check('27 cases x GCC/Clang x O0/O2 x2/4 CPUs',len(positives)==216 and {(x['case'],x['compiler'],x['optimization'],x['cpus']) for x in positives}=={(n,c,o,p) for n in range(1,28) for c in ('gcc','clang') for o in ('O0','O2') for p in (2,4)})
 negatives=[x for x in m['components'] if not x.get('target') and x['mutant']]
 check('five mechanism-specific guest mutants',len(negatives)==5 and {(x['case'],x['mutant']) for x in negatives}=={(9,'SF_MUTANT_CONSUME_COPY'),(12,'SF_MUTANT_THREAD16'),(16,'SDK_MUTANT_LOAD_LEAK'),(17,'SDK_MUTANT_OFFSET'),(21,'SDK_MUTANT_PAYLOAD_BINDING')})
 learners=[x for x in m['components'] if not x.get('target') and x['learner']]
 check('both compiler public scaffold semantic rejections',len(learners)==2 and {x['compiler'] for x in learners}=={'gcc','clang'})
 check('four target compiler/optimization campaigns',len([x for x in m['components'] if x.get('target')])==4)
 check('six native profiles per VM/store/workload',set(x['name'] for x in m['rows'] if x['name'].startswith('native-'))=={f'native-{family}-{cc}-{opt}-{kind}' for family in ('vm','store','workload') for cc in ('gcc','clang') for opt in ('O0','O2','san') for kind in ('compile','run')})
 check('four native semantic negative controls',set(x['name'] for x in m['rows'] if x['name'].startswith('negative-'))=={f'negative-{family}-{kind}' for family in ('vm','vm-learner','store-learner','workload-learner') for kind in ('compile','run')})
 check('271 source campaign commands and44 export commands',len(m['rows'])==271 and len(e['rows'])==44)
 workload=[]
 for row in m['components']:
  d=load(row['path']);check(row['path']+' source-bound passed',d['passed'] and sha(ROOT/row['path'])==row['sha256']);pins(d['source_sha256']);pins(d['artifacts'])
  for command in d['rows']:
   text=(ROOT/command['log']).read_text();check(command['log']+' exact raw outcome',sha(ROOT/command['log'])==command['sha256'] and command['passed'] and command['returncode']==command.get('expected',0) and (command.get('marker') is None or command['marker'] in text))
  if row.get('target'):
   check(row['path']+' eight distinct role images',len(d['images'])==8 and {(i['core'],i['swap'],i['burst']) for i in d['images']}=={(c,s,b) for c in ('m7','m4') for s in (0,1) for b in (1,4)})
   for image in d['images']:check(image['elf']+' audited-only',image['physical']=='not-run' and not image['unresolved'] and not image['external_runtime'])
  else:
   q=[x for x in d['rows'] if x['name'].startswith('guest')];check(row['path']+' actual original guest commands',len(q)==(3 if row['case']==23 else 1) and all('qemu-system-x86_64' in x['command'][0] and 'pc-i440fx-10.2' in x['command'] and 'none' in x['command'] for x in q))
   if row['case'] in (24,25):
    text=(ROOT/q[0]['log']).read_text();values=[list(map(int,line.split()[1:9])) for line in text.splitlines() if line.startswith('SFW ')];check(row['path']+' identical workload',len(values)==2 and values[0][:7]==[1,128,128,0,128,0,8384] and values[1][:7]==[4,128,47,81,45,2,1980]);workload.extend(dict(environment='QEMU-TCG',unit='emulated-TSC-counts',case=row['case'],compiler=row['compiler'],optimization=row['optimization'],cpus=row['cpus'],burst=v[0],elapsed=v[7],ledger=v[:7]) for v in values)
   if row['case']==23:
    line=load(str(Path(row['path']).parent/'disk-lineage.json'));obs=line['observations'];check(row['path']+' retained disk lineage',len(obs)==5 and obs[0]['disk_sha256']==obs[3]['disk_sha256'] and obs[0]['disk_sha256']!=obs[1]['disk_sha256'] and line['kernel_sha256']==sha(ROOT/Path(row['path']).parent/'kernel.elf'))
 for row in m['rows']+e['rows']:
  text=(ROOT/row['log']).read_text();check(row['name']+' runner outcome',row['passed'] and row['returncode']==row['expected'] and (row['marker'] is None or row['marker'] in text));check(row['name']+' no unexpected sanitizer error','runtime error:' not in text and 'ERROR: AddressSanitizer' not in text)
  if row['name'].startswith('native-workload') and row['name'].endswith('-run'):
   values=[list(map(int,line.split()[1:9])) for line in text.splitlines() if line.startswith('SFW ')];check(row['name']+' host same ledger',len(values)==2 and values[0][:7]==[1,128,128,0,128,0,8384] and values[1][:7]==[4,128,47,81,45,2,1980]);workload.extend(dict(environment='native-host',unit='CLOCK_MONOTONIC-ns',profile=row['name'],burst=v[0],elapsed=v[7],ledger=v[:7],timing_eligible='-san-' not in row['name']) for v in values)
 check('physical timing remains absent',not any(x['environment']=='physical-H745' for x in workload));check('11 relocated exact projects',len(e['packages'])==11)
 for package in e['packages']:
  root=ROOT/package['public_root'];pins(package['public_sha256'],root);check(package['project']+' public has no private source',not package['private_reference_in_public'] and not any('assessor' in p.relative_to(root).parts for p in root.rglob('*')))
  check(package['project']+' two compiler reference and scaffold gates',len(package['reports'])==4 and sorted(x['kind'] for x in package['reports'])==['public-scaffold','public-scaffold','reference','reference'])
  for row in package['reports']:
   d=load(row['path']);check(row['path']+' isolated actual validation',sha(ROOT/row['path'])==row['sha256'] and d['passed']);pins(d['source_sha256'],ROOT/row['root']);pins(d['artifacts'],ROOT/row['root'])
 return checks,workload
def main():
 p=argparse.ArgumentParser();p.add_argument('--mechanisms',required=True);p.add_argument('--exports',required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();assert not a.output.exists();m=load(a.mechanisms);e=load(a.exports);assert m['passed'] and e['passed'];checks,observations=inspect(m,e)
 from check_service_adapters_v5 import validate,regressions
 old=load('reviews/v5/expansion/followthrough/service_adapter_campaign.json');errors=validate(old);assert not errors,errors;controls=regressions(old)
 from check_amp_followthrough import validate_campaign
 amp=load('reviews/v5/expansion/legacy_amp_followthrough/campaign.json');validate_campaign(amp)
 providers=[dict(name='service-adapters',path='reviews/v5/expansion/followthrough/service_adapter_campaign.json',sha256=sha(ROOT/'reviews/v5/expansion/followthrough/service_adapter_campaign.json'),passed=True,controls=controls,scope='Actual152 receipt guest executions plus unchanged host cut/sector model and service mechanisms.'),dict(name='amp-followthrough',path='reviews/v5/expansion/legacy_amp_followthrough/campaign.json',sha256=sha(ROOT/'reviews/v5/expansion/legacy_amp_followthrough/campaign.json'),passed=True,scope='Frozen scheduler/cache/DMA/epoch/role image and emulator mechanisms, physical gate retained.')]
 result=dict(passed=all(c['passed'] for c in checks),error=None,checks=checks,observations=observations,providers=providers,components={n:dict(path=v,sha256=sha(ROOT/v)) for n,v in [('mechanisms',a.mechanisms),('exports',a.exports)]},source_sha256={p:sha(ROOT/p) for p in ['tools/audit_services_followthrough.py','tools/check_service_adapters_v5.py','tools/check_amp_followthrough.py']},artifacts={},scope='Raw source/image/log checks and retained-provider revalidation; no physical or independent learner promotion.')
 a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=result['passed'],checks=len(checks),observations=len(observations),failed=[x for x in checks if not x['passed']])));return int(not result['passed'])
if __name__=='__main__':raise SystemExit(main())
