"""Bind three unchanged original criterion/V1 pairs to exact author evidence."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_edu7_pipeline';PROJECTS=['PR-T40-08'];REPORT='research/v5/expansion/legacy_edu7_pipeline/validation.json';SUPPORT='research/v5/expansion/legacy_edu7_pipeline/completion_dispositions.json'
from research.v5.expansion.legacy_edu7_pipeline.author_support import attach
ARTIFACTS=['labs/edu7_followthrough/pipeline/CONTRACTS.md','labs/edu7_followthrough/pipeline/pipeline.h','assessor/edu7_followthrough/pipeline/pipeline.c','labs/edu7_followthrough/pipeline/tests.c','assessor/edu7_followthrough/pipeline/oracle.py','assessor/edu7_followthrough/pipeline/ANALYSIS.md']
REMAINING=['Actual dedicated three-board/two-segment physical capture remains required and not-run.','Independent original/delayed learner assessment and external review remain not-run.','Exact original X-T40/mixed-parent and source-gap reconciliations remain separately required.']
PATTERNS={'T40.08.a':['.exchange','.mutation.PLANT','.mutation.DUPLICATE'],'T40.08.b':['.identities','.mutation.HOP','.mutation.LINK','.mutation.RESTART','.real.'],'T40.08.c':['.faults','.adversarial','.mutation.PHYSICAL','.mutation.TOKENSTALL']}
WHY={'T40.08.a':'All seven actual layers carry SET42/duplicate/invalidSET/GET and reverse replies through three independent nodes/four UART owners; response refusal and cached replay preserve one effect.','T40.08.b':'Independent CRC/TLV oracle over all four actual transmitted software wire captures; exact namespaces/hop/local addresses and all-node quiescent transport reset with retained session reply.','T40.08.c':'Concrete full-path disconnect/retry, fresh encoded-byte split, corruption, route, malformed TLV, negotiation, token, credit and reboot boundaries; mandatory physical evidence remains not-run and cannot be supplied by host output.'}
EXPORT_PREREQUISITES=['labs/edu7_followthrough/prerequisite_stubs/pipeline.c',*[('labs/edu7_followthrough/'+n+'/learner.c') for n in ['session','transport','foundation']]]

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
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=project,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks,analysis='assessor/edu7_followthrough/pipeline/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=WHY[c['objective']],remaining_author_work=[],required_physical_work=[REMAINING[0]] if project=='PR-T40-08' else [],evidence_limits=r['limits']+REMAINING[:1]))
 return rows
def integrate():
 import run_edu7_pipeline as runner
 public=sorted({n for n in runner.pins() if n.startswith('labs/')}|set(EXPORT_PREREQUISITES)|{'fixtures/edu7_foundation/original_t40.json','fixtures/edu7_transport/profiles.json','fixtures/networking/PR-T40-08.json'})
 for project in PROJECTS:
  d=read(canon(project));old=HERE/(project+'.before-pipeline.json')
  if not old.exists():old.write_bytes(canon(project).read_bytes())
  write(HERE/('attachment_'+project+'.json'),dict(project=project,domain='T40',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/edu7_depth/CONTRACTS.md','labs/edu7_followthrough/transport/CONTRACTS.md','labs/edu7_followthrough/session/CONTRACTS.md','labs/edu7_followthrough/physical/CONTRACTS.md','labs/edu7_followthrough/foundation/CONTRACTS.md'],source_register='research/v5/expansion/legacy_edu7_pipeline/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Three unchanged criterion/V1 pairs: full forward/reverse pipeline, exact layer/namespace captures and failure composition; physical work remains separate.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(project),after)
 print('PASS additive EDU7 pipeline attachments')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_edu7_pipeline.py',ROOT/'tools/make_edu7_foundation_fixture.py',ROOT/'fixtures/edu7_foundation/original_t40.json',ROOT/'fixtures/edu7_transport/profiles.json'}|{p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for r in reports:
  assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in r[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in r['source_sha256']}
 for project in PROJECTS:
  d=read(canon(project));old=read(HERE/(project+'.before-pipeline.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(project))
 r,e=reports;rows=all_rows(r);assert len(rows)==6
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='T40 PR08 three exact criterion/V1 pairs; author, physical and independent dimensions remain distinct',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_edu7_pipeline/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact EDU7 pipeline dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
