"""Additive T33/T34 bindings; never replace baseline criteria or receipts."""
import argparse,copy,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent;sys.path.insert(0,str(HERE))
from criterion_notes import NOTES
SUPPORT='research/v5/expansion/legacy_build_followthrough/support.json';CAMPAIGN='reviews/v5/expansion/legacy_build_followthrough/campaign.json';CONTRACT='labs/build_followthrough/CONTRACTS.md';LAB=ROOT/'labs/build_followthrough';REF=ROOT/'assessor/build_followthrough'
PROJECTS=[f'PR-T{t}-{i:02d}' for t in (33,34) for i in range(1,7)]
def write(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf-8',newline='\n')
def records():return [json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(p+'.json')).read_text(encoding='utf-8')) for p in PROJECTS]
def public_paths():return sorted(p.relative_to(ROOT).as_posix() for p in LAB.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
def attach(record):
 if record['project'] not in PROJECTS:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 additions=record.setdefault('additional_successor_support',[]);additions[:]=[a for a in additions if a.get('path')!=SUPPORT];additions.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Exact original criterion and changed-condition analysis with separate native, solver, toolchain, counter and physical dimensions.'});additions.sort(key=lambda x:x['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for path in public_paths():
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def prepare():
 rows=[];analysis=['# T33/T34 assessor derivations','', 'Withheld from normal learner export. Exact task and variant identities are unchanged. Native tests, deductive goals, finite models, host measurements and physical observations have separate authority.','']
 for record in records():
  for c in record['criteria']:
   base,variant,checks=NOTES[c['objective']]
   rows.append({'id':c['id'],'project':record['project'],'objective':c['objective'],'original_task':c['task'],'original_expected':c['verification']['expected'],'changed_conditions':copy.deepcopy(c['variants']),'teaching':CONTRACT,'mechanism_checks':checks,'learner_deliverables':['Pre-execution prediction and concrete representation/extent/effect contract.','Implementation or premise/counterexample analysis of the exact task.','Raw result, exact tools/flags and a discriminating negative case.','Separate original delayed changed-condition artifact after at least seven days.']})
   analysis += ['## '+c['id'],'',base,'','Changed condition: '+c['variants'][0]['task'],'',variant,'','Evidence selectors: '+', '.join(checks)+'.','']
 assert len(rows)==66 and set(NOTES)=={r['objective'] for r in rows}
 write(LAB/'criteria.json',{'schema_version':1,'criteria':rows,'scope':'Tasks and teaching map only; actual execution and independent assessment are separate.'})
 (REF/'ANALYSIS.md').write_text('\n'.join(analysis)+'\n',encoding='utf-8',newline='\n')
 print('Prepared 66 exact criteria and 66 original changed conditions')
def main():
 data=json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8'))
 if not data.get('passed') or not data.get('source_stable'):raise ValueError('Run current campaign first')
 for p,h in data['inspected_paths'].items():
  if hashlib.sha256((ROOT/p).read_bytes()).hexdigest()!=h:raise ValueError('Stale campaign: '+p)
 updates=[];paths=[]
 for record in records():
  p=ROOT/'curriculum/v5/expansion/legacy_assessments'/(record['project']+'.json');attached=attach(record);write(p,attached);paths.append(p)
  for c in record['criteria']:
   base,variant,checks=NOTES[c['objective']];is_physical=c['objective'].startswith('T34.12');is_counter=c['objective'] in ('T34.02.c','T34.10.a','T34.10.c')
   evidence=[CAMPAIGN];gaps=[]
   if is_counter:gaps += ['Actual supported PMU/sampler evidence is governed by the campaign open_gates; timing/model results cannot discharge it.']
   if is_physical:gaps += ['Dedicated F439 physical timing/stack/energy capture remains unobserved.']
   row={'id':c['id'],'project':record['project'],'original_objective':c['objective'],'original_source':p.relative_to(ROOT).as_posix(),'closure_rationale':base,'mechanism_checks':checks,'dimensions':{'teaching':{'state':'complete-concrete-contracts-and-exact-changed-condition-analysis','evidence':[CONTRACT,'labs/build_followthrough/criteria.json']},'implementation':{'state':'implemented-or-analyzed-exact-task-with-explicit-model-boundaries','evidence':['assessor/build_followthrough/ANALYSIS.md',*(['assessor/build_followthrough/formal.c','assessor/build_followthrough/proof.c','labs/build_followthrough/models.py'] if c['objective'].startswith('T33') else ['assessor/build_followthrough/performance.c','assessor/build_followthrough/simd.c','assessor/build_followthrough/avx2.c','labs/build_followthrough/models.py'])]},'author_validation':{'state':'passed-bounded-author-checks-with-required-external-gates-open' if gaps else 'passed-source-bound-native-model-solver-or-analysis-checks','evidence':evidence},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-no-physical-board-observation-claimed','evidence':[]}},'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC15.2/Clang21.1.8 Linux; CBMC6.11; Frama-C33 WP/Eva with Z3; explicit x86 SIMD extension; separate Cortex-M4 freestanding builds. No host security setting changes.','external_blockers':gaps+['Independent human review and original minimum-seven-day unaided learner assessment remain unattempted.'],'evidence_limits':['WP retains the deliberately false goal and unsupported guards; no blanket proof success.','CBMC is finite and only covers its explicit object/environment model.','Host performance is scoped to actual workload/flags with uncontrolled affinity/frequency.','Actual physical target, energy and independent learner evidence remain separate.','Exact published C17 source remains unavailable; original source limitations preserved.']}
   updates.append(row)
   for v in c['variants']:
    child=copy.deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],closure_rationale=variant,variant_ids=[v['id']],variant_readiness='Exact changed condition implemented or analyzed in the separate assessor derivation; elapsed delay and independent learner execution are not claimed.');updates.append(child)
 pins=[*paths,ROOT/CAMPAIGN,HERE/'author_support.py',HERE/'criterion_notes.py',HERE/'register_sources.py',HERE/'resources.json',HERE/'source_access.json',HERE/'research_gate.json',ROOT/'tools/check_build_followthrough.py',*map(lambda p:ROOT/p,data['inspected_paths'])]
 resources=json.loads((HERE/'resources.json').read_text(encoding='utf-8'))
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':(HERE/'resources.json').relative_to(ROOT).as_posix(),'passages':r['locators']} for r in resources['resources']],'source_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_build_followthrough.py','report':CAMPAIGN}],'prior_attempts':sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'reviews/v5/expansion/legacy_build_followthrough').glob('campaign_attempt*.json')),'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed learner assessment.','Required supported PMU/sampling observations when current host denies access.','Dedicated F439 physical timing/stack/energy capture; host/cross-build evidence is not physical.'],'scope':'All 66 original T33/T34 task and 66 delayed identities, with criterion-specific implementation or worked premise/counterexample analysis; actual native/model/solver/toolchain observations bounded separately.'})
 print(json.dumps({'criteria':66,'updates':len(updates),'attachments':len(paths)}))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--prepare',action='store_true');a=p.parse_args();prepare() if a.prepare else main()
