"""Pure additive exact T49 attachment; historical criteria and receipts retained."""
import argparse,copy,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent;sys.path.insert(0,str(HERE));from criterion_notes import NOTES
LAB=ROOT/'labs/os_followthrough';REF=ROOT/'assessor/os_followthrough';CONTRACT='labs/os_followthrough/CONTRACTS.md';SUPPORT=HERE.relative_to(ROOT).as_posix()+'/support.json';CAMPAIGN='reviews/v5/expansion/legacy_os_followthrough/campaign.json';PROJECTS=[f'PR-T49-{i:02}' for i in range(1,11)]
def write(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf-8',newline='\n')
def records():return [json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(p+'.json')).read_text()) for p in PROJECTS]
def public_paths():
 retained=['labs/os_observatory/include/oslab.h','labs/os_observatory/src/oslab.c','labs/os_observatory/src/platform.c','labs/realtime_depth/linux/guest.py','labs/memory_followthrough/memory.h','labs/memory_followthrough/memory.c','labs/foundations/include/foundation.h','labs/foundations/src/foundation.c']
 return sorted({*[p.relative_to(ROOT).as_posix() for p in LAB.rglob('*') if p.is_file() and '__pycache__' not in p.parts],*[p.relative_to(ROOT).as_posix() for p in (ROOT/'fixtures/os_v3').glob('PR-T49-*.json')],*retained})
def attach(r):
 if r['project'] not in PROJECTS:return r
 for c in r['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 a=r.setdefault('additional_successor_support',[]);a[:]=[x for x in a if x.get('path')!=SUPPORT];a.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in r['criteria']],'scope':'Exact syscall,lifecycle,scheduling,fairness,paging,descriptor,readiness,cgroup andIO tasks/variants;author and external evidence separate.'});a.sort(key=lambda x:x['path'])
 if CONTRACT not in r['shared_contracts']:r['shared_contracts'].append(CONTRACT)
 for p in public_paths():
  if p not in r['supplied_artifacts']:r['supplied_artifacts'].append(p)
 return r
def prepare():
 rows=[];analysis=['# T49 assessor derivations','','Author observations do not establish independent or delayed learner performance. Hosted Linux,finite models,source comparisons and owned guest observations have separate scopes.','']
 for r in records():
  for c in r['criteria']:
   base,variant,checks=NOTES[c['objective']];rows.append({'id':c['id'],'project':r['project'],'objective':c['objective'],'original_task':c['task'],'original_expected':c['verification']['expected'],'changed_conditions':copy.deepcopy(c['variants']),'teaching':CONTRACT,'mechanism_checks':checks,'learner_deliverables':['Freeze the exact ABI,model,state,ownership and timing assumptions before execution.','Implement or derive the exact original task and changed condition.','Retain mechanism-specific raw outputs,negative controls,tool/source identity and cleanup.','Keep minimum-seven-day unseen independent assessment and physical observations separate.']});analysis+=['## '+c['id'],'','Original task: '+c['task'],'',base,'','Changed condition: '+c['variants'][0]['task'],'',variant,'','Evidence selectors: '+', '.join(checks)+'.','']
 assert len(rows)==30 and {r['objective'] for r in rows}==set(NOTES);write(LAB/'criteria.json',{'schema_version':1,'criteria':rows,'scope':'Exact30 original criteria and30 original delayed variants;no learner completion inferred.'});(REF/'ANALYSIS.md').write_text('\n'.join(analysis)+'\n',encoding='utf-8',newline='\n');print('Prepared30 criteria and30 variants')
def main():
 sys.path.insert(0,str(ROOT/'tools'));from check_os_followthrough import validate_campaign
 campaign=json.loads((ROOT/CAMPAIGN).read_text());validate_campaign(campaign);updates=[];paths=[]
 for r in records():
  path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(r['project']+'.json');write(path,attach(r));paths.append(path)
  for c in r['criteria']:
   base,variant,checks=NOTES[c['objective']];row={'id':c['id'],'project':r['project'],'original_objective':c['objective'],'original_source':path.relative_to(ROOT).as_posix(),'closure_rationale':base,'mechanism_checks':checks,'variant_ids':[v['id'] for v in c['variants']],'dimensions':{'teaching':{'state':'complete-exact-original-and-changed-condition-contracts','evidence':[CONTRACT,'labs/os_followthrough/criteria.json']},'implementation':{'state':'implemented-or-analyzed-exact-OS-mechanisms-and-premises','evidence':['assessor/os_followthrough/os.c','assessor/os_followthrough/ANALYSIS.md',*public_paths()]},'author_validation':{'state':'passed-source-bound-native-model-and-owned-guest-checks','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run;owned-guest-and-model-observations-separate','evidence':[]}},'required_environment':'C17;GCC15.2/Clang21.1.8 hosted Linux;private7.0.14 TCG guest and isolated64MiB image;strace6.19;AArch64 QEMU10.2.1;no host policy changes.','external_blockers':campaign['open_gates'],'evidence_limits':['Finite checks and same-author independent algorithm oracles are not independent learner assessment.','Scheduler models are not kernel equivalence or worst-case latency guarantees.','Logical IO,device counters,fsync acknowledgment and physical durability are different scopes.','Namespaces do not impose resource budgets;readiness does not transfer completion ownership.','Exact published C17/POSIX2024 remains unavailable;Linux manual/source authority is bounded.','Host exclusive scheduling/cold-cache control not observed.']};updates.append(row)
   for v in c['variants']:
    child=copy.deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],closure_rationale=variant,variant_ids=[v['id']],variant_readiness='Exact changed-condition code or premise analysis prepared;independent delayed gate remains unobserved.');updates.append(child)
 resources=json.loads((HERE/'resources.json').read_text());pins=[*paths,ROOT/CAMPAIGN,Path(__file__),HERE/'criterion_notes.py',HERE/'register_sources.py',HERE/'resources.json',HERE/'source_access.json',HERE/'research_gate.json',ROOT/'tools/check_os_followthrough.py',*[ROOT/p for p in campaign['inspected_paths']],*[ROOT/p for p in public_paths()],HERE/'inspected/linux-6.6-fair.c']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':(HERE/'resources.json').relative_to(ROOT).as_posix(),'passages':r['locators']} for r in resources['resources']],'source_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_os_followthrough.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':campaign['open_gates'],'scope':'All30 originalT49 criteria and30 exact original delayed variants;independent,physical and source authority remain separate.'});print(json.dumps({'criteria':30,'updates':len(updates)}))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--prepare',action='store_true');a=p.parse_args();prepare() if a.prepare else main()
