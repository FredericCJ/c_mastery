"""Bind all16 original T36 root/rubric/variant identities to exact evidence."""
import copy,json,pathlib
from common import ROOT,dump,sha
from register_certificate_transfer import BASE,IDS
from reconcile_implementation_v5 import variant_requirement
REVIEW='reviews/v5/expansion/certificate_transfer/'
REPORTS={'certificate':BASE+'/validation.json','match':'research/v5/expansion/match_transfer/validation.json','b04':REVIEW+'b04_depth_successor.json','systems':'reviews/v5/expansion/integrated/systems_certificates_campaign.json'}
PROVIDERS={n:'research/v5/expansion/legacy_algorithm_'+n+'/completion_dispositions.json' for n in ['frontier','tree','lookup']}
def read(n):return json.loads((ROOT/n).read_text(encoding='utf8'))
def bindings():
 out={};provider_rows={n:{r['id']:r for r in read(p)['updates']} for n,p in PROVIDERS.items()}
 def old(n,objective,variant=False):
  project={'frontier':'07','tree':'05','lookup':'06'}[n];key='AX-PR-T36-'+project+'-'+objective+('-V1' if variant else '')
  r=provider_rows[n][key];return dict(provider=PROVIDERS[n],provider_id=key,**copy.deepcopy(r['author_validation']))
 def original(n):return dict(report=REPORTS[n],scope='Original family contract and named ordinary certificate/tree/matches anchors; wider fixture bounds are retained, not replaced by successor profiles.')
 def new(n,part):
  r=read(REPORTS[n]);tags=[q['id'] for q in r['commands'] if (('.case'+part in q['id']) or '.execute' in q['id'] or '.reject.' in q['id'] or(n=='certificate' and part=='1' and ('.independent' in q['id'] or '.cost.' in q['id'] or q['id']=='costs')))]
  return dict(report=REPORTS[n],checks=tags,scope='Exact new stream/publication or explicit-stack mechanism, with compiling semantic controls and actual target closure.')
 graph=[old('frontier','T36.09.a'),old('frontier','T36.09.c')]
 tree=[old('tree','T36.07.'+x) for x in ['a','b','c']]
 lookup=[old('lookup','T36.08.'+x) for x in ['b','c']]
 out.update({'X-T36-001':[original('b04'),*graph,new('certificate','0')],'X-T36-001-R1':[original('b04'),graph[0]],'X-T36-001-R2':[original('b04'),graph[1]],'X-T36-001-V1':[old('frontier','T36.09.c',True)],'X-T36-001-V2':[new('certificate','0')]})
 out.update({'X-T36-002':[original('systems'),*tree,new('certificate','1')],'X-T36-002-R1':[original('systems'),tree[0]],'X-T36-002-R2':[original('systems'),tree[1]],'X-T36-002-R3':[original('systems'),tree[2],new('certificate','1')],'X-T36-002-V1':[new('certificate','1')],'X-T36-002-V2':[old('tree','T36.07.a',True)]})
 out.update({'X-T36-003':[original('systems'),*lookup,new('match','0'),new('match','1'),new('match','2')],'X-T36-003-R2':[original('systems'),lookup[0]],'X-T36-003-R3':[original('systems'),lookup[1]],'X-T36-003-V1':[new('match','0'),new('match','1'),new('match','2')],'X-T36-003-V2':[old('lookup','T36.08.c',True),new('match','2')]})
 return out
def rows():
 result=[];mapped=bindings()
 for name in IDS:
  path='curriculum/v5/exercises/'+name+'.md';text=(ROOT/path).read_text(encoding='utf8');d=json.loads(text.split('```json\n')[1].split('\n```')[0]);body=text.split('\n```',1)[1];variant_ids=d['delayed_transfer']['variant_ids'];tasks=[(name,d['title'],d['objectives'],False)]+[(r['id'],r['criterion'],[r['objective']],False) for r in d['rubric']]+[(name+'-'+v,variant_requirement(body,v,variant_ids),d['objectives'],True) for v in variant_ids]
  folder='match_transfer' if name.endswith('003') else 'certificate_transfer';artifacts=[path,'labs/'+folder+'/CONTRACTS.md','assessor/'+folder+'/ANALYSIS.md']
  for id,task,objectives,delayed in tasks:
   evidence=mapped[id];reports=list(dict.fromkeys(r['report'] for r in evidence));dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:2]),('implementation','complete',artifacts),('author_validation','passed',reports),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-required-for-host-and-emulator-functional-contract',[])]}
   result.append(dict(id=id,parent_exercise=name,parent_project=d['project'],objectives=objectives,original_task=task,minimum_days=d['delayed_transfer']['minimum_days'] if delayed else None,dimensions=dimensions,artifacts=artifacts,assessment_bindings=evidence,closure_rationale='Exact retained ordinary contract plus criterion-specific provider/variant evidence; new missing mechanisms are implemented and executed separately. No successor capacity/policy silently replaces the original exercise.',remaining_author_work=[],evidence_limits=['Author execution and explicit reference composition only; external review and independent seven-day learner evidence remain pending.','Physical observations are not required for these declared host/emulator functional contracts, and no physical result is claimed.','Published C17/C23 access remains in G-CONTRACT-C17; no new final-standard inspection claimed.']))
 assert set(mapped)=={r['id'] for r in result};return result
def main():
 from check_certificate_transfer import validate as certificate
 from check_match_transfer import validate as match
 import check_expansion_campaigns as ordinary
 assert not certificate(read(REPORTS['certificate']));assert not match(read(REPORTS['match']))
 for name,family in [('b04','b04_depth'),('systems','systems_certificates')]:
  r=read(REPORTS[name]);m=read('labs/'+family+'/family.json');assert r['passed'];assert not ordinary.pin_errors(r)+ordinary.execution_errors(r)+ordinary.special_errors(family,m,r)
 reports=[*REPORTS.values(),BASE+'/exports.json',REVIEW+'guard.json','reviews/v5/expansion/match_transfer/guard.json',*[REVIEW+n+'_provider_guard.json' for n in PROVIDERS]]
 files={pathlib.Path(__file__),ROOT/'tools/check_certificate_transfer_package.py',ROOT/'tools/register_certificate_transfer.py',ROOT/BASE/'resources.json',ROOT/BASE/'research_gate.json'};rws=rows()
 for n in [*reports,*PROVIDERS.values()]:
  d=read(n);files.add(ROOT/n)
  if n in reports:assert d.get('passed') is True or d.get('status')=='pass',n
  for k in ['source_sha256','inspected_paths','outputs_sha256']:
   for p,h in d.get(k,{}).items():assert sha(ROOT/p)==h,(n,p);files.add(ROOT/p)
 for row in rws:
  files.update(ROOT/p for p in row['artifacts'])
  for b in row['assessment_bindings']:
   if 'checks' in b:assert set(b['checks'])<={q['id'] for q in read(b['report'])['commands']},row['id']
 dump(ROOT/BASE/'objective_mapping.json',dict(scope='Exact16 original exercise obligations; no new exercise IDs or baseline changes',rows=rws));files.add(ROOT/BASE/'objective_mapping.json')
 dump(ROOT/BASE/'completion_dispositions.json',dict(schema_version=1,scope='X-T36-001/002/003 original root/rubric/V1/V2 completion, preserving wider ordinary contracts',updates=rws,source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=reports,remaining_author_work=[],external_evidence=['Non-author technical review','Actual independent learner provider substitution and delayed transfer'],source_gap_ids=['G-CONTRACT-C17']))
 print(dict(rows=len(rws),files=len(files),scope='author implementation and bounded execution'))
if __name__=='__main__':main()
