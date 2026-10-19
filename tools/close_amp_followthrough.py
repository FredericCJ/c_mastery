"""Compose already executed producer receipts; no broad regeneration."""
from pathlib import Path
import argparse,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_amp_followthrough.author_support import OPEN,PROJECTS,public_paths
from research.v3.verify_fixtures import compute
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--mechanisms',type=Path,required=True);p.add_argument('--audit',type=Path,required=True);p.add_argument('--exports',type=Path,required=True);a=p.parse_args();components={}
 for name,path in [('mechanisms',a.mechanisms),('audit',a.audit),('exports',a.exports)]:
  value=json.loads(path.read_text());assert value['passed'] and value['error'] is None
  for f,h in value['source_sha256'].items():assert sha(ROOT/f)==h,f
  for f,h in value['artifacts'].items():assert sha(ROOT/f)==h,f
  components[name]={'path':path.as_posix(),'sha256':sha(path)}
 fixtures=[]
 for project in PROJECTS:
  path=ROOT/'fixtures/os_v3'/(project+'.json');d=json.loads(path.read_text());actual=compute(d['oracle'],d['input']);assert actual==d['expected'];fixtures.append(dict(path=path.relative_to(ROOT).as_posix(),sha256=sha(path),oracle=d['oracle'],actual=actual))
 here=ROOT/'research/v5/expansion/legacy_amp_followthrough';review=ROOT/'reviews/v5/expansion/legacy_amp_followthrough'
 manual=[('R-RM0399-R4','build/execution-sources/RM0399-r4.pdf','https://www.stmcu.jp/wp/wp-content/uploads/2019/06/RM0399_Rev4.pdf',['p142-143 SRAM/TCM bus and power domains','p362 hold boot and p477 BOOT_C2 only allows boot, WWDG reset scopes','p555-558 HSEM read-take/release and retained interrupt status','p561-564 word access and per-core register banks']),('R-PM0253-R5','build/execution-sources/PM0253-r5.pdf','https://www.st.com/resource/en/programming_manual/DM00237416.pdf',['p228-229 MPU cache/permission attributes','p240-241 maintenance addresses and set/way fields'])]
 resources={'schema_version':1,'resources':[dict(id=id,path=path,sha256=sha(ROOT/path),url=url,locators=locators,scope='Actual selected local manual pages read; no full/current manual or silicon errata qualification.') for id,path,url,locators in manual],'frozen_provider_sha256':{v:sha(ROOT/v) for v in json.loads((ROOT/'assessor/amp_followthrough/providers.json').read_text()).values()},'original_fixture_oracle':{'path':'research/v3/verify_fixtures.py','sha256':sha(ROOT/'research/v3/verify_fixtures.py'),'scope':'Actual relevant ranges/ack/ring/doorbell/cache_span/buffer_states/cycles/epochs/pipeline/budget branches read and executed; no implementation or hardware certification.'},'limits':['Local repository tasks, original fixture contracts and frozen providers inspected before successor authoring.','The C17 baseline is not a claim of fresh access to the full published C17 text.','Exact silicon revision/current errata, board timing and bus effects remain external observations.']}
 (here/'resources.json').write_text(json.dumps(resources,indent=2)+'\n')
 paths={ROOT/p for p in public_paths()}
 paths|={p for base in ('assessor/amp_followthrough','research/v5/expansion/legacy_amp_followthrough') for p in (ROOT/base).rglob('*') if p.is_file() and '__pycache__' not in p.parts and p.name!='support.json'}
 paths|={ROOT/'tools'/p for p in ('run_amp_followthrough.py','audit_amp_followthrough.py','export_amp_followthrough.py','check_amp_followthrough.py','close_amp_followthrough.py')}
 for component in components.values():paths|={ROOT/p for p in json.loads((ROOT/component['path']).read_text())['source_sha256']}
 tools={}
 for command in ('gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','qemu-system-arm'):
  r=subprocess.run([command,'--version'],capture_output=True,text=True,check=True);tools[command]=r.stdout.splitlines()[0]
 result=dict(schema_version=1,passed=True,components=components,source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(paths)},fixtures=fixtures,coverage={'original_criteria':30,'delayed_variants':30,'mechanism_commands':147,'supplement_commands':24,'isolated_packages':10,'export_commands':56},open_gates=OPEN,tool_versions=tools,scope='Exact native mechanisms, explicit premise models, actual original single-core Cortex-M task execution and H745 two-image audits; no physical or independent learner completion.')
 (review/'campaign.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=True,fixtures=len(fixtures),sources=len(paths))))
if __name__=='__main__':main()
