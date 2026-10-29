"""Close named integration children from the actual driver campaign, not headings."""
from pathlib import Path
import argparse,copy,hashlib,json
from check_network_driver import validate as campaign_validate
from check_network_driver_package import validate as package_validate
ROOT=Path(__file__).resolve().parents[1]
BASE='research/v5/expansion/legacy_network_driver/'
REVIEW='reviews/v5/expansion/legacy_network_driver/'
CHILDREN={'NW-ARP-DRIVER':('arp',['T41.04.a','T41.04.b','T41.04.c']),
          'NW-IP-DRIVER':('ip',['T41.05.a','T41.05.b','T41.05.c']),
          'NW-TCP-DRIVER':('tcp',['T41.06.a','T41.06.b','T41.06.c','T41.07.a','T41.07.b','T41.07.c'])}
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def put(p,d):(ROOT/p).write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def validate_evidence(campaign,support,guard):
 errors=campaign_validate(campaign)+package_validate(support)
 if guard.get('passed') is not True or len(guard.get('negative_controls',[]))!=14 or not all(c.get('rejected') for c in guard['negative_controls']):errors.append('driver package rejection evidence')
 for p,h in guard.get('input_sha256',{}).items():
  if sha(p)!=h:errors.append('stale package guard '+p)
 if len(campaign.get('checks',[]))!=198:errors.append('actual composition scope')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--write',action='store_true');p.add_argument('--self-test',action='store_true');p.add_argument('--report',required=True);a=p.parse_args()
 c=read(REVIEW+'current_validation.json');s=read(BASE+'completion_dispositions.json');g=read(REVIEW+'package_validation.json');errors=validate_evidence(c,s,g);controls=[]
 if a.self_test and not errors:
  for kind in ['failed-campaign','lost-negative','lost-physical-boundary','stale-driver','unrelated-test']:
   x,y,z=copy.deepcopy((c,s,g))
   if kind=='failed-campaign':x['passed']=False
   elif kind=='lost-negative':z['negative_controls'].pop()
   elif kind=='lost-physical-boundary':y['updates'][0]['dimensions']['physical_observation']['state']='passed'
   elif kind=='stale-driver':y['source_sha256']['assessor/network_followthrough/driver/driver.c']='0'*64
   else:x['checks']=[r for r in x['checks'] if r['label']!='gcc-debug 3 execute']
   rejected=bool(validate_evidence(x,y,z));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 if a.write and not errors:
  artifacts=['labs/network_followthrough/driver/CONTRACTS.md','assessor/network_followthrough/driver/driver.c','assessor/network_followthrough/driver/board.c','assessor/network_followthrough/driver/ANALYSIS.md']
  evidence=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json','package_validation.json']]
  dims={k:dict(state=state,evidence=ev) for k,state,ev in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',evidence),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
  updates=[];files={BASE+'completion_dispositions.json',*evidence,'tools/reconcile_network_driver.py'}
  for ident,(area,objectives) in CHILDREN.items():
   path=f'research/v5/expansion/legacy_network_{area}/remaining_work.json';d=read(path);row=next(r for r in d['required_work'] if r['id']==ident)
   if 'before_driver_closure' not in row:row['before_driver_closure']=copy.deepcopy(row)
   row.update(state='author-implementation-and-execution-complete',dimensions=copy.deepcopy(dims),artifacts=list(dict.fromkeys(row.get('artifacts',row.get('current_artifacts',[]))+artifacts)),evidence=evidence,remaining_author_work=[],closure_rationale='Actual frozen struct en_ring/struct ep_phy/struct na_arp/struct ni_reassembly/struct nc_stack composition;198 source-bound positive/negative/native/target checks,20 exported-tool checks and22+14 receipt controls. Physical and independent dimensions remain separate.',limits='Author reference integration executed; subsequent learner substitution, physical observation and external assessment remain required separate evidence.')
   put(path,d);files.add(path)
   updates.append(dict(id=ident,domain='T41',originating_records=[path],objectives=objectives,dimensions=copy.deepcopy(dims),artifacts=artifacts,evidence=evidence,closure_rationale=row['closure_rationale'],remaining_author_work=[]))
  path='research/v5/expansion/legacy_network_scope/remaining_work.json';d=read(path);r=next(x for x in d['required_work'] if x['id']=='NW-CORPUS-REASSEMBLY');r.setdefault('before_driver_closure',copy.deepcopy(r));r.update(state='author-implementation-and-execution-complete',remaining_author_work=[],dimensions=copy.deepcopy(dims));r['closure_rationale']+=' The named actual driver child is now closed by its source-bound composition package.' if 'named actual driver child is now closed' not in r['closure_rationale'] else '';r['evidence']=list(dict.fromkeys(r['evidence']+evidence));put(path,d);files.add(path)
  put(BASE+'integration_completion_dispositions.json',dict(schema_version=1,updates=updates,source_sha256={p:sha(p) for p in sorted(files)},remaining_parents=['Physical board and independent/delayed learner evidence remain separate.','Other required curriculum domains remain in the canonical recursive inventory.']))
 out=dict(passed=not errors,errors=errors,negative_controls=controls,written=a.write and not errors,input_sha256={p:sha(p) for p in [BASE+'completion_dispositions.json',REVIEW+'current_validation.json',REVIEW+'package_validation.json','tools/reconcile_network_driver.py']});put(a.report,out);print(dict(passed=not errors,written=out['written'],controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
