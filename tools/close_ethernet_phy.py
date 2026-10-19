"""Preserve all three original T39.05 criterion/changed task pairs."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_ethernet_phy';PROJECTS=['PR-T39-04'];REPORT='research/v5/expansion/legacy_ethernet_phy/validation.json';SUPPORT='research/v5/expansion/legacy_ethernet_phy/completion_dispositions.json'
from research.v5.expansion.legacy_ethernet_phy.author_support import attach
ARTIFACTS=['labs/ethernet_followthrough/phy/CONTRACTS.md','labs/ethernet_followthrough/phy/BOARD_CAPTURE.md','assessor/ethernet_followthrough/phy/ANALYSIS.md','assessor/ethernet_followthrough/phy/phy.c','assessor/ethernet_followthrough/phy/target.c','labs/ethernet_followthrough/phy/tests.c','labs/ethernet_followthrough/phy/mapped.c','labs/ethernet_followthrough/phy/leases.c','labs/ethernet_followthrough/phy/guest.c','assessor/ethernet_followthrough/phy/oracle.py']
REMAINING=['Physical selected-board/PHY identity, clock, MDIO and link measurements remain required.','Actual MAC/DMA normal-ring initialization, RX/TX ownership and stop/recovery remain required separate T39 producer work.','Other T38–40 and mixed LRUN parents remain required, not closed by these three criterion pairs.','External review and independent original/seven-day learner attempts remain unperformed.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon(p):return ROOT/('curriculum/v5/expansion/legacy_assessments/'+p+'.json')
def all_rows(r):
 patterns={'T39.05.a':['.access','.mapped','.mutation.ADDRESS','.port.audit'],'T39.05.b':['.access','.lifecycle','.bounded','.faults','.mutation.RESET','.mutation.ACK','.mutation.BUSY','.mutation.BOUND','.closure','.stack'],'T39.05.c':['.lifecycle','.leases','.mutation.EPOCH','.mutation.LATCH','.mutation.READY']}
 why={'T39.05.a':'Actual F439 divisor/command adapter, excluded HCLK and primary route/clock analysis with object access audit.','T39.05.b':'Actual MDIO read/write/reset/restart-ack/identity/current-status implementation, finite-clock/poll budget and78 callback failure positions.','T39.05.c':'Resolved-mode admission, historical/current link retirement and actual retained EDTx copy/abort lease composition; no inference of physical DMA quiescence.'}
 rows=[]
 for p in PROJECTS:
  for c in read(canon(p))['criteria']:
   v=c['variants'][0]
   for delayed in [False,True]:
    dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:2]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    checks=[q['id'] for q in r['commands'] if any(x in q['id'] for x in patterns[c['objective']]) or q['id'] in [cc+'.O'+o+'.execute' for cc in ['arm-none-eabi-gcc','clang'] for o in ['0','3']]]
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=p,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks),closure_rationale=why[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 return rows
def integrate():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/ethernet_followthrough/phy').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/ethernet_depth/include/ethernet_depth.h','labs/ethernet_depth/src/ethernet_depth.c','labs/ethernet_depth/CONTRACTS.md','labs/interrupt_depth/frame/target.S']
 for p in PROJECTS:
  d=read(canon(p));old=HERE/(p+'.before-phy.json')
  if not old.exists():old.write_bytes(canon(p).read_bytes())
  write(HERE/('attachment_'+p+'.json'),dict(project=p,domain='T39',supplied_artifacts=public,shared_contracts=ARTIFACTS[:2]+['labs/ethernet_depth/CONTRACTS.md'],source_register='research/v5/expansion/legacy_ethernet_phy/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Three original criterion/V1 pairs; author implementation and bounded model execution, separate physical and independent gates.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(p),after)
 print('PASS additive PHY attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_ethernet_phy.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for r in reports:
  assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
  for k in ['source_sha256','outputs_sha256']:
   for n,h in r[k].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in r['source_sha256']}
 for p in PROJECTS:
  d=read(canon(p));old=read(HERE/(p+'.before-phy.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(p))
 r,e=reports;rows=all_rows(r);assert len(rows)==6
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-T39-04 all three original criterion/V1 pairs; six separate evidence dimensions',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_ethernet_phy/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact PHY dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
