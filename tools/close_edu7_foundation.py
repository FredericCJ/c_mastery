"""Bind twelve unchanged original criterion/V1 pairs to exact author evidence."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_edu7_foundation';PROJECTS=['PR-T40-01','PR-T40-03','PR-T40-04','PR-T40-07'];REPORT='research/v5/expansion/legacy_edu7_foundation/validation.json';SUPPORT='research/v5/expansion/legacy_edu7_foundation/completion_dispositions.json'
from research.v5.expansion.legacy_edu7_foundation.author_support import attach
ARTIFACTS=['labs/edu7_followthrough/foundation/CONTRACTS.md','labs/edu7_followthrough/foundation/foundation.h','assessor/edu7_followthrough/foundation/foundation.c','labs/edu7_followthrough/foundation/tests.c','assessor/edu7_followthrough/foundation/oracle.py','assessor/edu7_followthrough/foundation/ANALYSIS.md']
REMAINING=['T40 PR02/05/06/08 physical adapter/grant, epoch-bound transport/expiry, two-endpoint token and whole exchange remain required.','Actual dedicated two-segment RS485 timing/termination/relay/cable observations remain not-run; no radio execution claimed.','Independent original/delayed learner assessment and external review remain not-run.','Exact full standards/held-source gaps and original X-T40/mixed parent reconciliation remain separate obligations.']
PATTERNS={'T40.01.a':['.ledger','.framing'],'T40.01.b':['.ledger','.mutation.BOUND'],'T40.01.c':['.router','.mutation.BORROW','.mutation.RELEASE'],'T40.03.a':['.framing','.mutation.ESCAPE'],'T40.03.b':['.framing','.mutation.CANONICAL'],'T40.03.c':['.framing'],'T40.04.a':['.router','.mutation.NAMESPACE'],'T40.04.b':['.router'],'T40.04.c':['.router','.mutation.BORROW','.mutation.RELEASE'],'T40.07.a':['.codec','.mutation.SIGNED','i386.'],'T40.07.b':['.codec','.mutation.CAPACITY'],'T40.07.c':['.codec']}
WHY={'T40.01.a':'Actual local-body/grant witness plus model/peer/radio responsibility analysis; no ISO suite or physical RF inference.','T40.01.b':'Separate canonical/envelope arithmetic and explicitly versioned wider-field extension with independent byte/length oracle.','T40.01.c':'Actual copied delayed storage and generation-bound borrowed lifetime, with poison/backpressure/stale-release counterexamples.','T40.03.a':'Retained actual CRC/encoder plus distinct version2 escaping and polynomial-division oracle.','T40.03.b':'All stream splits, overlong/escape recovery, fault isolation and strict canonicality successor to diagnosed permissive raw decoder.','T40.03.c':'Actual wrong-address/ungranted refusal and harmless modified-valid-CRC original-target counterexample; upper authority remains independent.','T40.04.a':'Actual both-direction next-hop reconstruction preserving opaque end-to-end bytes.','T40.04.b':'Actual decrement versus local positive-hop delivery and explicit route68 versioned extension without application API authority.','T40.04.c':'Actual single-slot copied/borrowed ownership, full/generation/protocol/length/hop/route refusals.','T40.07.a':'Actual schema1 signed codec and explicit bounded i32 schema2, observed on LP64/ILP32 hosts and M4 CPU.','T40.07.b':'Capacity-aware staged publication, malformed canonical UTF8/TLV checks, every multibyte split and exact private sentinels.','T40.07.c':'Actual representable-but-disallowed SET and display-label changes with no routing/session/unit authority.'}

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
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=project,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks,analysis='assessor/edu7_followthrough/foundation/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=WHY[c['objective']],remaining_author_work=[],required_physical_work=[REMAINING[1]] if project in ['PR-T40-03','PR-T40-04'] else [],evidence_limits=r['limits']+REMAINING[:1]))
 return rows
def integrate():
 import run_edu7_foundation as runner
 public=sorted({n for n in runner.pins() if n.startswith('labs/')}|{'fixtures/edu7_foundation/original_t40.json'})
 for project in PROJECTS:
  d=read(canon(project));old=HERE/(project+'.before-foundation.json')
  if not old.exists():old.write_bytes(canon(project).read_bytes())
  write(HERE/('attachment_'+project+'.json'),dict(project=project,domain='T40',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/edu7_depth/CONTRACTS.md'],source_register='research/v5/expansion/legacy_edu7_foundation/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Twelve unchanged criterion/V1 pairs: actual retained EDU7 plus foundation adapters, versions and explicit physical limits.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(project),after)
 print('PASS additive EDU7 foundation attachments')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_edu7_foundation.py',ROOT/'tools/make_edu7_foundation_fixture.py',ROOT/'fixtures/edu7_foundation/original_t40.json'}|{p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for r in reports:
  assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in r[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in r['source_sha256']}
 for project in PROJECTS:
  d=read(canon(project));old=read(HERE/(project+'.before-foundation.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(project))
 r,e=reports;rows=all_rows(r);assert len(rows)==24
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='T40 PR01/03/04/07 twelve exact criterion/V1 pairs; author, physical and independent dimensions remain distinct',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_edu7_foundation/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=12,delayed=12,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS24 exact EDU7 foundation dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
