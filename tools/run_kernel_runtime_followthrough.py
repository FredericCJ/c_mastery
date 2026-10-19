"""Freeze the actual T48 runtime guest provider before whole-family publication."""
import argparse,hashlib,json,subprocess,sys
from pathlib import Path
from audit_kernel_runtime_stack import audit
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
 paths=[ROOT/p for p in ('assessor/kernel_followthrough/boot/boot.S','assessor/kernel/model.c','labs/kernel/include/cmk_contract.h','labs/kernel/include/cmk_abi.h','labs/kernel/platform/link.ld','labs/kernel/platform/user.ld','labs/kernel/tools/guest.py','assessor/runtime/cvm.c','labs/runtime/include/cvm.h','assessor/memory_followthrough/byte_support.c','tools/run_kernel_guest_followthrough.py','tools/audit_kernel_runtime_stack.py','tools/run_kernel_runtime_followthrough.py')]
 for d in ('labs/kernel_followthrough/guest','assessor/kernel_followthrough/guest','labs/kernel_followthrough/runtime','assessor/kernel_followthrough/runtime'):
  paths.extend(p for p in (ROOT/d).glob('*') if p.is_file() and p.suffix in ('.c','.h','.S','.py'))
 for d,name in (('contracts','contracts'),('device','device')):paths.extend([ROOT/f'labs/kernel_followthrough/{d}/{name}.h',ROOT/f'assessor/kernel_followthrough/{d}/{name}.c'])
 return sorted(set(paths))
def validate(v):
 assert v['passed'] and v['error'] is None
 assert v['inspected_paths']=={p.relative_to(ROOT).as_posix():sha(p) for p in sources()}
 assert len(v['guests'])==12
 for row in v['guests']:
  p=ROOT/row['receipt'];assert sha(p)==row['sha256'];r=json.loads(p.read_text());assert r['passed'] and r['error'] is None
  assert all(sha(ROOT/p)==h for p,h in r['artifact_sha256'].items())
  assert all(c['exit_code']==c['expected_exit_code'] for c in r['commands'])
  if row['case'] in (26,27,29):assert audit(p.parent)['fits_one_page']==(row['case']==27)
 return True
def main(work):
 work=work.resolve();assert work.is_relative_to(ROOT/'reviews/v5/expansion/legacy_kernel_followthrough');work.mkdir(parents=True,exist_ok=False);before={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};guests=[]
 try:
  plans=[(cc,opt,case) for cc in ('gcc','clang') for opt in ('O0','O2') for case in (26,27)]+[(cc,'O2',case) for cc in ('gcc','clang') for case in (28,29)]
  for cc,opt,case in plans:
   folder=work/f'{cc}-{opt}-case{case}';cmd=[sys.executable,str(ROOT/'tools/run_kernel_guest_followthrough.py'),'--work',str(folder),'--cc',cc,'--optimization',opt,'--case',str(case)];r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=90);assert r.returncode==0,(cmd,r.stdout,r.stderr);receipt=folder/'result.json';guests.append({'case':case,'compiler':cc,'optimization':opt,'receipt':receipt.relative_to(ROOT).as_posix(),'sha256':sha(receipt)})
   if case in (26,27,29):record=audit(folder);assert record['fits_one_page']==(case==27);(folder/'stack-audit.json').write_text(json.dumps(record,indent=2)+'\n')
   print(json.dumps({'completed':folder.name}),flush=True)
  assert before=={p.relative_to(ROOT).as_posix():sha(p) for p in sources()}
 except Exception as e:passed=False;error=repr(e)
 else:passed=True;error=None
 report={'passed':passed,'error':error,'inspected_paths':before,'guests':guests,'scope':'Frozen native VM probe and actual ring3 adapters; author execution only; unchanged baseline core versus explicitly registered scratch variant are separate profiles.'};(work/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'passed':passed,'error':error,'guests':len(guests)}));return int(not passed)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path);p.add_argument('--check',type=Path);a=p.parse_args()
 if a.check:validate(json.loads(a.check.read_text()));print('Frozen runtime guest source and 12 actual profiles PASS')
 else:raise SystemExit(main(a.work))
