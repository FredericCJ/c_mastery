"""Bind six unchanged original criterion/V1 pairs to exact author evidence."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_ethernet_handoff';PROJECTS=['PR-T39-08'];REPORT='research/v5/expansion/legacy_ethernet_handoff/validation.json';SUPPORT='research/v5/expansion/legacy_ethernet_handoff/completion_dispositions.json'
from research.v5.expansion.legacy_ethernet_handoff.author_support import attach
ARTIFACTS=['labs/ethernet_followthrough/handoff/CONTRACTS.md','labs/ethernet_followthrough/handoff/handoff.h','assessor/ethernet_followthrough/handoff/handoff.c','labs/ethernet_followthrough/handoff/tests.c','labs/ethernet_followthrough/handoff/stack.c','labs/ethernet_followthrough/handoff/reassembly.c','assessor/ethernet_followthrough/handoff/oracle.py','assessor/ethernet_followthrough/handoff/ANALYSIS.md']
REMAINING=['No routed HTTP stack: the route envelope is separate from the actual same-link struct nb_net/struct nh_http composition; full Internet conformance remains unclaimed.','Actual physical captures and bounded clock/uncertainty observations remain not-run.','Independent original/delayed learner assessment and external review remain not-run.','T40 and mixed LRUN23/LRUN24 plus exact X-T39 sidecar reconciliation remain required; exact final standards source gaps persist.']
PATTERNS={'T39.09.a':['.route','.mutation.GATEWAY','.mutation.IP_IDENTITY'],'T39.09.b':['.stack','.mutation.HTTP','.missing_application'],'T39.09.c':['.reassembly','.mutation.HOLE','.mutation.OVERLAP','.mutation.DEADLINE'],'T39.10.a':['.clock','.mutation.DRIFT','.mutation.UNCERTAINTY'],'T39.10.b':['.divergence','.route'],'T39.10.c':['.budget','.mutation.BUDGET']}
WHY={'T39.09.a':'Actual local/gateway selection and unchanged final-IP serializer with independent namespaces and explicit narrow route profile.','T39.09.b':'Actual frozen struct en_ring/ARP/IP/TCP/stream/HTTP providers and raw peer; partial service, ACK/DMA retirement, absolute deadlines, stale epochs and active-close TIME_WAIT.','T39.09.c':'Actual bounded reassembly extension with explicit representation, holes, overlap poison, deadline and tombstone failures; peer interoperability remains separately scoped.','T39.10.a':'Exact offset and checked interval widening, nonwrapping order, ceiling division, extrema and all-or-nothing publication.','T39.10.b':'Executable corrupt-frame admission/repair and subsequent missing-route divergence, retaining actual first-prerequisite contract.','T39.10.c':'Original exact ordinary/changed wire-slot arithmetic and padding mutation; serialization distinguished from complete exchange latency.'}

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
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=project,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks,analysis='assessor/ethernet_followthrough/handoff/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=WHY[c['objective']],remaining_author_work=[],required_physical_work=[REMAINING[1]] if c['objective']=='T39.10.a' else [],evidence_limits=r['limits']+REMAINING[:1]))
 return rows
def integrate():
 import run_ethernet_handoff as runner
 public=sorted({n for n in runner.pins() if n.startswith('labs/')}|{'labs/ethernet_depth/src/ethernet_depth.c'})
 for project in PROJECTS:
  d=read(canon(project));old=HERE/(project+'.before-handoff.json')
  if not old.exists():old.write_bytes(canon(project).read_bytes())
  write(HERE/('attachment_'+project+'.json'),dict(project=project,domain='T39',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/ethernet_depth/CONTRACTS.md',*[('labs/network_followthrough/'+n+'/CONTRACTS.md') for n in ['driver','http','ip','tcp','arp','scope']]],source_register='research/v5/expansion/legacy_ethernet_handoff/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Six unchanged criterion/V1 pairs: actual cross-layer composition, routing/clock/reassembly and explicit physical limits.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(project),after)
 print('PASS additive handoff attachments')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_ethernet_handoff.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for r in reports:
  assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in r[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in r['source_sha256']}
 for project in PROJECTS:
  d=read(canon(project));old=read(HERE/(project+'.before-handoff.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(project))
 r,e=reports;rows=all_rows(r);assert len(rows)==12
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='T39 PR08 six exact criterion/V1 pairs; author, physical and independent dimensions remain distinct',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_ethernet_handoff/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=6,delayed=6,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS12 exact handoff dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
