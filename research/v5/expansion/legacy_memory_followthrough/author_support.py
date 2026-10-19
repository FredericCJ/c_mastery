"""Pure additive T45 attachment producer; historical criterion objects retained."""
import argparse,copy,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent;sys.path.insert(0,str(HERE));from criterion_notes import NOTES
LAB=ROOT/'labs/memory_followthrough';REF=ROOT/'assessor/memory_followthrough';CONTRACT='labs/memory_followthrough/CONTRACTS.md';SUPPORT=HERE.relative_to(ROOT).as_posix()+'/support.json';CAMPAIGN='reviews/v5/expansion/legacy_memory_followthrough/campaign.json';PROJECTS=[f'PR-T45-{i:02}' for i in range(1,11)]
def write(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf-8',newline='\n')
def records():return [json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(p+'.json')).read_text()) for p in PROJECTS]
def public_paths():
 retained=['labs/foundations/include/foundation.h','labs/foundations/src/foundation.c','labs/performance_followthrough/stack.c']
 return sorted({*[p.relative_to(ROOT).as_posix() for p in LAB.rglob('*') if p.is_file() and '__pycache__' not in p.parts],*[p.relative_to(ROOT).as_posix() for p in (ROOT/'fixtures/memory_lab').glob('PR-T45-*.json')],*retained})
def attach(r):
 if r['project'] not in PROJECTS:return r
 for c in r['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 a=r.setdefault('additional_successor_support',[]);a[:]=[x for x in a if x.get('path')!=SUPPORT];a.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in r['criteria']],'scope':'Exact extents, sizing, stack, layout, resources, resize, initialization, async and diagnosis tasks and variants;author and external evidence separate.'});a.sort(key=lambda x:x['path'])
 if CONTRACT not in r['shared_contracts']:r['shared_contracts'].append(CONTRACT)
 for p in public_paths():
  if p not in r['supplied_artifacts']:r['supplied_artifacts'].append(p)
 return r
def prepare():
 rows=[];analysis=['# T45 assessor derivations','','Author observations do not establish independent or delayed learner performance. Raw negative controls are bounded original specimens.','']
 for record in records():
  for c in record['criteria']:
   base,variant,checks=NOTES[c['objective']];rows.append({'id':c['id'],'project':record['project'],'objective':c['objective'],'original_task':c['task'],'original_expected':c['verification']['expected'],'changed_conditions':copy.deepcopy(c['variants']),'teaching':CONTRACT,'mechanism_checks':checks,'learner_deliverables':['Freeze predictions, image/symbol/input/config/environment identity and finite timeout.','Implement the exact mechanism or derive the named premise/counterexample.','Retain actual mechanism-specific raw observations and repaired/negative controls.','Keep seven-day independent diagnosis and physical observations separate.']});analysis+=['## '+c['id'],'','Original task: '+c['task'],'',base,'','Changed condition: '+c['variants'][0]['task'],'',variant,'','Evidence selectors: '+', '.join(checks)+'.','']
 assert len(rows)==30 and {r['objective'] for r in rows}==set(NOTES);write(LAB/'criteria.json',{'schema_version':1,'criteria':rows,'scope':'Exact30 original criteria and30 original variants;no learner completion inferred.'});(REF/'ANALYSIS.md').write_text('\n'.join(analysis)+'\n',encoding='utf-8',newline='\n');print('Prepared30 criteria and30 variants')
def main():
 sys.path.insert(0,str(ROOT/'tools'));from check_memory_followthrough import validate_campaign
 campaign=json.loads((ROOT/CAMPAIGN).read_text());validate_campaign(campaign);updates=[];paths=[]
 for r in records():
  path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(r['project']+'.json');write(path,attach(r));paths.append(path)
  for c in r['criteria']:
   base,variant,checks=NOTES[c['objective']];row={'id':c['id'],'project':r['project'],'original_objective':c['objective'],'original_source':path.relative_to(ROOT).as_posix(),'closure_rationale':base,'mechanism_checks':checks,'variant_ids':[v['id'] for v in c['variants']],'dimensions':{'teaching':{'state':'complete-exact-original-and-changed-condition-contracts','evidence':[CONTRACT,'labs/memory_followthrough/criteria.json']},'implementation':{'state':'implemented-or-analyzed-exact-diagnostic-mechanisms-and-premises','evidence':['assessor/memory_followthrough/memory.c','assessor/memory_followthrough/ANALYSIS.md',*public_paths()]},'author_validation':{'state':'passed-source-bound-native-tool-and-model-checks','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run;captured-and-emulated-target-records-separate','evidence':[]}},'required_environment':'C17 core;GCC15.2/Clang21.1.8 Linux;GDB17.1,ASan/UBSan,Memcheck3.26;Cortex-M4 QEMU;no host security changes.','external_blockers':campaign['open_gates'],'evidence_limits':['Finite and bounded executions do not establish absence of all defects.','Saved GDB text is not a native core and omitted memory is unknown.','ASan silence after optimization does not prove absence of invalid source paths.','Cancellation request is not cancellation delivery or ownership return.','Captured MCU premises and QEMU are not physical-board evidence.','Exact published C17/C23 not obtained; optional checked comparison scoped to N3096 draft and installed headers.']};updates.append(row)
   for v in c['variants']:
    child=copy.deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],closure_rationale=variant,variant_ids=[v['id']],variant_readiness='Exact changed-condition implementation or premise analysis prepared;independent delayed gate remains unobserved.');updates.append(child)
 resources=json.loads((HERE/'resources.json').read_text());pins=[*paths,ROOT/CAMPAIGN,Path(__file__),HERE/'criterion_notes.py',HERE/'register_sources.py',HERE/'resources.json',HERE/'source_access.json',HERE/'research_gate.json',ROOT/'tools/check_memory_followthrough.py',*[ROOT/p for p in campaign['inspected_paths']],*[ROOT/p for p in public_paths()]]
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':(HERE/'resources.json').relative_to(ROOT).as_posix(),'passages':r['locators']} for r in resources['resources']],'source_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_memory_followthrough.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':campaign['open_gates'],'scope':'All30 original T45 criteria and30 exact original delayed variants;independent/physical authority remains separate.'});print(json.dumps({'criteria':30,'updates':len(updates)}))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--prepare',action='store_true');a=p.parse_args();prepare() if a.prepare else main()
