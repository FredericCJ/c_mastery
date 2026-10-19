"""Attach the actual normal-ring composition without rewriting accepted tasks."""
import argparse, copy, hashlib, json, pathlib, subprocess, sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
HERE=ROOT/'research/v5/expansion/legacy_diagnostic_ring'
PROJECT='curriculum/v5/expansion/legacy_assessments/PR-T28-07.json'
SUPPORT='research/v5/expansion/legacy_diagnostic_ring/completion_dispositions.json'
ATTACH='research/v5/expansion/legacy_advanced_diagnostic/attachment_ring_PR-T28-07.json'
OLD='research/v5/expansion/legacy_advanced_diagnostic/completion_dispositions.json'
REPORT='research/v5/expansion/legacy_diagnostic_ring/validation.json'
ARTIFACTS=['labs/diagnostic_ring/CONTRACTS.md','labs/diagnostic_ring/bridge.h','labs/diagnostic_ring/tests.c','assessor/diagnostic_ring/bridge.c','assessor/diagnostic_ring/ANALYSIS.md','assessor/diagnostic_ring/oracle.py']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+PROJECT],cwd=ROOT,text=True,encoding='utf8'))
def rows(r):
 out=[]
 for prior in read(ROOT/OLD)['updates']:
  if prior['objective']!='T28.12.a':continue
  row=copy.deepcopy(prior);row['artifacts']=list(dict.fromkeys(row['artifacts']+ARTIFACTS))
  row['dimensions']['teaching']['evidence']+=ARTIFACTS[:3]
  row['dimensions']['implementation']['evidence']+=ARTIFACTS
  row['dimensions']['author_validation']['evidence']+=[REPORT,'research/v5/expansion/legacy_diagnostic_ring/exports.json']
  row['author_validation']['composition']=dict(report=REPORT,checks=[q['id'] for q in r['commands'] if '.tx' in q['id'] or '.rx' in q['id'] or '.mutation.' in q['id'] or q['id'].endswith('.execute')])
  row['closure_rationale']+=' Additional actual DG/EnRing composition distinguishes copied admission, OWN publication, DMA completion and released application publication; it retains uncertain device ownership after a failed doorbell.'
  row['evidence_limits']+=r['limits'];out.append(row)
 return out
def integrate():
 import run_diagnostic_ring as run
 from research.v5.expansion.legacy_advanced.author_support import attach
 d=read(ROOT/PROJECT);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 before=HERE/'PR-T28-07.before-ring.json'
 if not before.exists():before.write_bytes((ROOT/PROJECT).read_bytes())
 public=sorted({n for n in run.pins() if n.startswith('labs/')}|{'labs/peripheral_depth/diagnostic/diagnostic.c','labs/peripheral_depth/diagnostic/ethernet.c','labs/ethernet_followthrough/ring/learner.c','labs/ethernet_followthrough/phy/learner.c','labs/ethernet_depth/src/ethernet_depth.c','labs/bringup_depth/bringup.c','labs/foundations/src/foundation.c','labs/ethernet_followthrough/ring/CONTRACTS.md','labs/ethernet_followthrough/phy/CONTRACTS.md'})
 put(ROOT/ATTACH,dict(project='PR-T28-07',domain='T28',key='diagnostic_ring_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_diagnostic_ring/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria'] if c['objective']=='T28.12.a'],scope='Actual DG diagnostic over EnRing; original USB/changed-hardware tasks remain bound to their separate complete campaign. Physical Ethernet and independent learner evidence are separate.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;put(ROOT/PROJECT,after)
 print('PASS original diagnostic tasks preserved; actual ring public composition attached')
def package():
 import run_diagnostic_ring as run
 r=read(HERE/'validation.json');e=read(HERE/'exports.json');assert r['source_sha256']==run.pins()
 files={pathlib.Path(__file__),ROOT/'tools/check_diagnostic_ring.py',ROOT/PROJECT,ROOT/ATTACH,ROOT/OLD,ROOT/'research/v5/expansion/legacy_advanced/author_support.py'}
 for report in [r,e]:
  assert report['status']=='pass' and report['commands'] and all(q['passed'] for q in report['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in report[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in report['source_sha256']}
 files|={p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 old=read(ROOT/OLD)
 for n,h in old['source_sha256'].items():assert sha(ROOT/n)==h,('old diagnostic metadata stale',n)
 files|={ROOT/n for n in old['source_sha256']}
 put(HERE/'completion_dispositions.json',dict(schema_version=1,scope='T28.12.a ordinary and its unchanged delayed task, including actual EnRing composition; all prior USB/host/frame evidence retained',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_diagnostic_ring/exports.json',*old['current_reports']],updates=rows(r),remaining_parents=['LRUN-24 must also bind the separate CAN/USB routes and selected PHY/ring prerequisites.','Physical board/cable, external review and independent delayed learner attempts remain unperformed.']))
 print('PASS two exact diagnostic composition dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
