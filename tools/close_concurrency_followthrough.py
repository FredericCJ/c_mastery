"""Compose only the completed T55 producers and exact original fixture results."""
import argparse,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_concurrency_followthrough.author_support import OPEN,PROJECTS,public_paths
from research.v3.verify_fixtures import compute
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser()
 for name in ('mechanisms','measurements','audit','exports'):p.add_argument('--'+name,type=Path,required=True)
 a=p.parse_args();components={};paths={ROOT/p for p in public_paths()}
 for name in ('mechanisms','measurements','audit','exports'):
  path=getattr(a,name);d=json.loads(path.read_text());assert d['passed'] and d['error'] is None
  for f,h in {**d['source_sha256'],**d['artifacts']}.items():assert sha(ROOT/f)==h,f
  paths|={ROOT/p for p in d['source_sha256']};components[name]=dict(path=path.as_posix(),sha256=sha(path))
 fixtures=[]
 for project in PROJECTS:
  path=ROOT/'fixtures/os_v3'/(project+'.json');d=json.loads(path.read_text());actual=compute(d['oracle'],d['input']);assert actual==d['expected'];fixtures.append(dict(path=path.relative_to(ROOT).as_posix(),sha256=sha(path),oracle=d['oracle'],actual=actual))
 here=ROOT/'research/v5/expansion/legacy_concurrency_followthrough';review=ROOT/'reviews/v5/expansion/legacy_concurrency_followthrough';resources=[]
 for family in ('spsc','identity','reclamation','grace','history'):
  path=f'research/v5/expansion/legacy_advanced_{family}/resources.json';d=json.loads((ROOT/path).read_text());resources.append(dict(family=family,path=path,sha256=sha(ROOT/path),registered_passages=[dict(id=x['id'],title=x['title'],locators=x.get('locators',[]),url=x.get('url')) for x in d['resources']],scope='Preserved primary-passage inspection registration plus current source/receipt guard; no fresh full-standard or all-document access claim.'))
 resource=dict(schema_version=1,resources=resources,frozen_provider_sha256={p:sha(ROOT/p) for p in json.loads((ROOT/'assessor/concurrency_followthrough/providers.json').read_text()).values()},original_fixture_oracle=dict(path='research/v3/verify_fixtures.py',sha256=sha(ROOT/'research/v3/verify_fixtures.py'),scope='Actual ring, cas, tag, pool, hazards, queue, epoch_reclaim, grace, history and speedup branches inspected and ten exact original fixtures executed.'),limits=['C17 baseline; N1570 evidence is a C11 committee draft and is not fresh access to published C17/C23.','Source/protocol arguments, finite host histories, instrumented runs, measured host distributions and selected target link evidence remain separate.','No all-history weak-memory proof, unbounded progress, physical cache, independent learner or external reviewer promotion.'])
 (here/'resources.json').write_text(json.dumps(resource,indent=2)+'\n')
 paths|={ROOT/'research/v3/verify_fixtures.py'}
 for base in ('assessor/concurrency_followthrough','research/v5/expansion/legacy_concurrency_followthrough'):paths|={p for p in (ROOT/base).rglob('*') if p.is_file() and '__pycache__' not in p.parts and p.name!='support.json'}
 paths|={ROOT/'tools'/p for p in ('run_concurrency_followthrough.py','measure_concurrency_followthrough.py','audit_concurrency_followthrough.py','export_concurrency_followthrough.py','close_concurrency_followthrough.py','check_concurrency_followthrough.py')}
 result=dict(schema_version=1,passed=True,components=components,source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(paths)},fixtures=fixtures,coverage=dict(original_criteria=30,delayed_variants=30,mechanism_commands=311,measurement_commands=357,observations=340,native_groups=64,paired_comparisons=8,isolated_packages=10,export_commands=180,provider_guards=5),open_gates=OPEN,scope='All original T55 tasks and changed conditions through exact mechanisms, premise analyses, finite independent history oracle and measured host workloads. No physical or independent learner completion.')
 destination=review/'campaign.json';assert not destination.exists();destination.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=True,sources=len(paths),fixtures=len(fixtures))))
if __name__=='__main__':main()
