"""Bind twelve unchanged original criterion/V1 pairs to exact author evidence."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_ethernet_wire';PROJECTS=['PR-T39-01','PR-T39-02','PR-T39-03'];REPORT='research/v5/expansion/legacy_ethernet_wire/validation.json';SUPPORT='research/v5/expansion/legacy_ethernet_wire/completion_dispositions.json'
from research.v5.expansion.legacy_ethernet_wire.author_support import attach
ARTIFACTS=['labs/ethernet_followthrough/wire/CONTRACTS.md','labs/ethernet_followthrough/wire/BOARD_LEDGER.md','assessor/ethernet_followthrough/wire/wire.c','labs/ethernet_followthrough/wire/tests.c','assessor/ethernet_followthrough/wire/oracle.py','assessor/ethernet_followthrough/wire/ANALYSIS.md']
REMAINING=['Exact current normative IEEE802.3/802.1Q source bodies remain uninspected; historical restricted profile does not claim their conformance.','Required actual changed-board/PHY/clock/wire/duplex observations remain not-run.','Independent original/delayed learner assessment and external review remain not-run.','T39-08/T40 and mixed LRUN23/LRUN24 plus exact X-T39 sidecar reconciliation remain required.']
PATTERNS={'T39.01.a':['.layers','.stream','.mutation.WRAP','.mutation.SUFFIX'],'T39.01.b':['.layers','.stream'],'T39.01.c':['.stream','.bridge','.frame','.mutation.SERVICE','.mutation.ENDIAN'],'T39.02.a':['.layers'],'T39.02.b':['.layers'],'T39.02.c':['.layers'],'T39.03.a':['.frame','.vlan','.mutation.TAG'],'T39.03.b':['.frame','.vlan'],'T39.03.c':['.frame','.vlan','.mutation.CRC','.mutation.FRAMECRC','.mutation.PAD'],'T39.04.a':['.bridge'],'T39.04.b':['.bridge','.mutation.EVICT','.mutation.INGRESS'],'T39.04.c':['.loop','.bridge']}
WHY={'T39.01.a':'Actual copied record service plus every two-piece split0..256 and concatenated/zero/terminal/failure boundaries; retained wrapper arithmetic.','T39.01.b':'Seven-purpose and delegated-MCU responsibility analysis, with actual record boundary evidence; does not substitute adapter for a complete stack.','T39.01.c':'Explicit first-owning-contract analysis and executable same-signature opposite-byte-order counterexample.','T39.02.a':'Source-bound MAC/RMII/PHY/magnetics route and changed external SPI-controller responsibility analysis; no invented chip implementation.','T39.02.b':'Actual selected PHY negotiation/parallel-detection analysis and serialization arithmetic; full-only refusal premise and unobserved physical rates explicit.','T39.02.c':'Actual Rev8/Rev11 primary comparison and mandatory fresh board ledger; emulator evidence is explicitly rejected as physical acceptance.','T39.03.a':'Original rejected type/length inputs plus complete separate historical single-tag encoder/parser/removal contract and payload bounds.','T39.03.b':'Exact capture-span/padding derivation and executable tag-pop repair; no double FCS subtraction.','T39.03.c':'Executable payload-only CRC counterexample, full-frame integrity and different TX/RX FCS contracts.','T39.04.a':'Actual bit classification and same-local-MAC learning/misdirection trace; no IP or authentication equivalence.','T39.04.b':'Original expiry trace, atomic table-full policy, explicit oldest eviction and malformed duplicate-key refusal.','T39.04.c':'Promiscuous-receive argument plus executable parallel-link broadcast cycle and static acyclic repair, with induction rather than claimed spanning-tree implementation.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon(p):return ROOT/('curriculum/v5/expansion/legacy_assessments/'+p+'.json')
def all_rows(r):
 rows=[]
 for project in PROJECTS:
  for c in read(canon(project))['criteria']:
   v=c['variants'][0]
   for delayed in [False,True]:
    dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:2]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    checks=[q['id'] for q in r['commands'] if any(p in q['id'] for p in PATTERNS[c['objective']]) or q['id'] in [cc+'.O'+o+'.execute' for cc in ['arm-none-eabi-gcc','clang'] for o in ['0','3']]]
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=project,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks,analysis='assessor/ethernet_followthrough/wire/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=WHY[c['objective']],remaining_author_work=[],required_physical_work=[REMAINING[1]] if project=='PR-T39-02' else [],evidence_limits=r['limits']+REMAINING[:1]))
 return rows
def integrate():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/ethernet_followthrough/wire').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/ethernet_depth/include/ethernet_depth.h','labs/ethernet_depth/src/ethernet_depth.c','labs/ethernet_depth/CONTRACTS.md','labs/interrupt_depth/frame/target.S']
 for project in PROJECTS:
  d=read(canon(project));old=HERE/(project+'.before-wire.json')
  if not old.exists():old.write_bytes(canon(project).read_bytes())
  write(HERE/('attachment_'+project+'.json'),dict(project=project,domain='T39',supplied_artifacts=public,shared_contracts=ARTIFACTS[:2]+['labs/ethernet_depth/CONTRACTS.md'],source_register='research/v5/expansion/legacy_ethernet_wire/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Original twelve criterion/V1 pairs: actual retained core/new adapters and separate source/physical argument obligations.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(project),after)
 print('PASS additive wire attachments')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_ethernet_wire.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for r in reports:
  assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in r[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in r['source_sha256']}
 for project in PROJECTS:
  d=read(canon(project));old=read(HERE/(project+'.before-wire.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(project))
 r,e=reports;rows=all_rows(r);assert len(rows)==24
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='T39 PR01–03 twelve exact criterion/V1 pairs; author, physical and independent dimensions remain distinct',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_ethernet_wire/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=12,delayed=12,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS24 exact wire dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
