"""Compose completed T56 producers without changing any retained provider receipt."""
import argparse,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
from research.v5.expansion.legacy_services_followthrough.author_support import OPEN,PROJECTS,public_paths
from research.v3.verify_fixtures import compute
from audit_services_followthrough import sha

def main():
 p=argparse.ArgumentParser()
 for name in ('mechanisms','audit','exports'):p.add_argument('--'+name,type=Path,required=True)
 a=p.parse_args();components={};paths={ROOT/p for p in public_paths()}
 for name in ('mechanisms','audit','exports'):
  path=getattr(a,name);d=json.loads(path.read_text());assert d['passed'] and d['error'] is None
  for f,h in {**d['source_sha256'],**d['artifacts']}.items():assert sha(ROOT/f)==h,f
  paths|={ROOT/p for p in d['source_sha256']};components[name]=dict(path=path.as_posix(),sha256=sha(path))
  if name=='mechanisms':
   for row in d['components']:
    child=json.loads((ROOT/row['path']).read_text());paths|={ROOT/p for p in child['source_sha256']}
 fixtures=[]
 for project in PROJECTS:
  path=ROOT/'fixtures/os_v3'/(project+'.json');d=json.loads(path.read_text());actual=compute(d['oracle'],d['input']);assert actual==d['expected'];fixtures.append(dict(path=path.relative_to(ROOT).as_posix(),sha256=sha(path),oracle=d['oracle'],actual=actual))
 here=ROOT/'research/v5/expansion/legacy_services_followthrough';review=ROOT/'reviews/v5/expansion/legacy_services_followthrough';review.mkdir(parents=True,exist_ok=True)
 manual='research/v5/expansion/legacy_amp_followthrough/resources.json';registered=json.loads((ROOT/manual).read_text());provider=[p for p in paths if p.relative_to(ROOT).as_posix().startswith('assessor/') and not p.is_relative_to(ROOT/'assessor/services_followthrough')]
 retained=['curriculum/v5/expansion/legacy_assessments/PR-T56-07.json','research/v5/expansion/followthrough/request_objective_readiness.json','reviews/v5/expansion/followthrough/service_adapter_campaign.json','reviews/v5/expansion/legacy_amp_followthrough/campaign.json']
 resource=dict(schema_version=1,resources=[dict(path=manual,sha256=sha(ROOT/manual),registered_passages=registered['resources'],scope='Preserved bounded RM0399 revision4 and PM0253 revision5 passage inspection; no fresh full/current manual access or silicon qualification.')],frozen_provider_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(provider)},retained_receipts={p:sha(ROOT/p) for p in retained},original_fixture_oracle=dict(path='research/v3/verify_fixtures.py',sha256=sha(ROOT/'research/v3/verify_fixtures.py'),scope='Actual references/reap/pool/cow/span/read/fuel/durable/recovery/pipeline branches inspected and all11 exact original fixtures executed.'),imports=[dict(source='assessor/kernel_smp',use='Actual process/thread scheduler, page table, TLB acknowledgment, virtio request and static bounce ownership. Original bytes copied; staged patches and identities in every guest report.'),dict(source='assessor/service_depth/kernel',use='Checked loader rollback, files, explicit shared clone profile and durable receipt. New SF syscalls and wrapper contracts leave retained old ABI unchanged.'),dict(source='assessor/runtime/cvm.c',use='Actual bounded VM plus frozen verifier/execution successors; wrapper adds instance/session/request identity.'),dict(source='assessor/storage/store.c',use='Actual store engine and unchanged receipt adapter; real guest page/flush callbacks and explicit inactive-WAL damage controls.'),dict(source='assessor/amp_followthrough/core/amp.c',use='Actual bounded ring core and retained separate scheduler/cache/DMA/restart campaign. New H745 images are audited synthetic polling experiments.')],limits=['C17 baseline plus explicitly declared target assembly; no full published C17/C23 access claim.','Native host, instrumented original guest, audited target image and finite fixture evidence are distinct.','No physical H745, independent learner, external reviewer or all-interleaving proof promotion.','Original PR-T56-07 and its frozen provider remain unchanged.'])
 (here/'resources.json').write_text(json.dumps(resource,indent=2)+'\n');paths.add(ROOT/'research/v3/verify_fixtures.py')
 for base in ('assessor/services_followthrough','research/v5/expansion/legacy_services_followthrough'):paths|={p for p in (ROOT/base).rglob('*') if p.is_file() and '__pycache__' not in p.parts and p.name!='support.json'}
 paths|={ROOT/'tools'/p for p in ('run_services_followthrough.py','audit_services_followthrough.py','export_services_followthrough.py','close_services_followthrough.py','check_services_followthrough.py')}
 from check_services_followthrough import COVERAGE,validate_campaign
 result=dict(schema_version=1,passed=True,components=components,source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(paths)},fixtures=fixtures,coverage=COVERAGE,open_gates=OPEN,scope='All33 remaining original T56 criteria and33 exact V1 variants. Actual distinct native/guest/image/export checks, unchanged retained mechanism receipts and explicit physical/independent gates.')
 validate_campaign(result);destination=review/'campaign.json';assert not destination.exists();destination.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=True,sources=len(paths),fixtures=len(fixtures))))
if __name__=='__main__':main()
