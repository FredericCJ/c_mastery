"""Pure additive exact T48.02–14 attachment; original contracts stay intact."""
import argparse,copy,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent;sys.path.insert(0,str(HERE));from criterion_notes import NOTES
LAB=ROOT/'labs/kernel_followthrough';REF=ROOT/'assessor/kernel_followthrough';CONTRACT='labs/kernel_followthrough/CONTRACTS.md';SUPPORT=HERE.relative_to(ROOT).as_posix()+'/support.json';CAMPAIGN='reviews/v5/expansion/legacy_kernel_followthrough/campaign.json';PROJECTS=[f'PR-T48-{i:02}' for i in range(2,15)]
def write(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf-8',newline='\n')
def records():return [json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(p+'.json')).read_text()) for p in PROJECTS]
def public_paths():
 paths=[p for p in LAB.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
 for directory in ('labs/kernel/include','labs/kernel/learner','labs/kernel/platform','labs/runtime/include'):
  paths.extend(p for p in (ROOT/directory).glob('*') if p.is_file())
 paths += [ROOT/'labs/kernel/README.md',ROOT/'labs/runtime/SPEC.md',ROOT/'labs/storage/FORMAT.md',ROOT/'labs/storage/INTEGRATION.md']
 paths += list((ROOT/'fixtures/kernel').glob('PR-T48-*.json'))
 return sorted({p.relative_to(ROOT).as_posix() for p in paths})
def attach(r):
 if r['project'] not in PROJECTS:return r
 for c in r['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 entries=r.setdefault('additional_successor_support',[]);entries[:]=[x for x in entries if x.get('path')!=SUPPORT];entries.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in r['criteria']],'scope':'Exact checked-memory,trap,isolation,scheduling,ELF,syscall,device,persistence,shell,runtime,service and transfer mechanisms; independent evidence remains separate.'});entries.sort(key=lambda x:x['path'])
 if CONTRACT not in r['shared_contracts']:r['shared_contracts'].append(CONTRACT)
 for path in public_paths():
  if path not in r['supplied_artifacts']:r['supplied_artifacts'].append(path)
 return r
def prepare():
 rows=[];analysis=['# T48.02–14 private assessor derivations','','Do not export this file or the completed providers. Each exact original and delayed task retains its own evidence selectors. Author execution does not establish independent learner, external review or physical observations.','']
 for r in records():
  for c in r['criteria']:
   base,variant,checks=NOTES[c['objective']];rows.append({'id':c['id'],'project':r['project'],'objective':c['objective'],'original_task':c['task'],'original_expected':c['verification']['expected'],'changed_conditions':copy.deepcopy(c['variants']),'teaching':CONTRACT,'mechanism_checks':checks,'learner_deliverables':['Predict the exact ownership/state/failure relation before execution.','Implement the specified mechanism or derive the named changed-premise counterexample.','Retain raw mechanism-specific observations,negative controls and exact source/tool/image identities.','Keep minimum-seven-day unseen independent assessment separate from author runs.']});analysis += ['## '+c['id'],'','Original task: '+c['task'],'',base,'','Changed condition: '+c['variants'][0]['task'],'',variant,'','Evidence selectors: '+', '.join(checks)+'.','','Assessment:0 for wrong ownership/result or inflated evidence;1 for one nominal case without the causal boundary;2 for the complete original relation and independent discriminator;3 for the exact changed condition with its new premises justified. Matching output with an invalid language/lifetime explanation fails.','']
 assert len(rows)==39 and {r['objective'] for r in rows}==set(NOTES)
 write(LAB/'criteria.json',{'schema_version':1,'criteria':rows,'scope':'39 exact original criteria and39 original delayed variants;T48.01 is in the separate frozen boot package.'});(REF/'ANALYSIS.md').write_text('\n'.join(analysis)+'\n',encoding='utf-8',newline='\n');print('Prepared39 exact criteria and39 variants')
def evidence_index(v):
 data={k:json.loads((ROOT/row['path']).read_text()) for k,row in v['components'].items()};index={}
 for selector in sorted({s for row in NOTES.values() for s in row[2]}):
  kind,name=selector.split(':');entry={'scope':kind,'records':[]}
  if kind in ('contracts','device'):
   needle='-killed' if name=='mutation-controls' else name
   selected=[x['name'] for x in data[kind]['checks'] if needle in x['name']];assert selected,selector;entry['records']=[{'report':v['components'][kind]['path'],'checks':selected}]
  elif kind in ('guest','runtime'):
   component='matrix' if kind=='guest' else 'runtime';rows=data[component]['guests'];selected=[r for r in rows if r['case']==(15 if name=='quantum-delay-matrix' else int(name[4:]))];assert selected,selector;entry['records']=[{'report':x['receipt'],'sha256':x['sha256'],'case':x['case']} for x in selected]
  elif kind=='smp':
   entry['records']=[{'report':x['receipt'],'sha256':x['sha256'],'cpus':x['cpus']} for x in data['smp']['runs']] if name=='1-2-4' else [{'report':str(Path(v['components']['smp']['path']).parent/'mutations/mutations.json'),'mutations':['tlb_ack','msix_mask']}]
  elif kind=='services':entry['records']=[{'report':v['components']['services']['path'],'checks':['gcc-debug 0 execute','clang-debug 0 execute'],'source':'labs/runtime_followthrough/services/tests.c','mechanism':'74 crash cuts:72 old,2 new' if name=='74-cuts' else 'latest exact duplicate,changed payload,older/gap/zero IDs and full64-bit exhaustion'}]
  elif kind=='guard':entry['records']=[{'procedure':'tools/check_kernel_followthrough.py','controls':['stale-source','wrong-image-campaign','missing-device-campaign','missing-guest-profile','timeout-success','unearned-external-completion','failed-campaign','wrong-actual-exit','stale-image']}]
  else:raise ValueError(selector)
  index[selector]=entry
 write(HERE/'evidence_index.json',{'schema_version':1,'selectors':index});return index
def main():
 sys.path.insert(0,str(ROOT/'tools'));from check_kernel_followthrough import validate_campaign
 v=json.loads((ROOT/CAMPAIGN).read_text());validate_campaign(v);index=evidence_index(v);updates=[];canonical=[]
 for r in records():
  path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(r['project']+'.json');write(path,attach(r));canonical.append(path)
  for c in r['criteria']:
   base,variant,checks=NOTES[c['objective']];row={'id':c['id'],'project':r['project'],'original_objective':c['objective'],'original_source':path.relative_to(ROOT).as_posix(),'closure_rationale':base,'mechanism_checks':checks,'variant_ids':[x['id'] for x in c['variants']],'dimensions':{'teaching':{'state':'complete-exact-original-and-changed-contracts','evidence':[CONTRACT,'labs/kernel_followthrough/criteria.json']},'implementation':{'state':'implemented-or-derived-exact-mechanisms-and-premise-counterexamples','evidence':['assessor/kernel_followthrough/ANALYSIS.md','assessor/kernel_followthrough/contracts/contracts.c','assessor/kernel_followthrough/device/device.c','assessor/kernel_followthrough/guest/kernel.c','assessor/kernel_followthrough/runtime/cvm_scratch.c']},'author_validation':{'state':'passed-source-bound-hosted-and-actual-owned-guest-mechanisms','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run;TCG-and-finite-models-separate','evidence':[]}},'external_blockers':v['open_gates'],'evidence_limits':['Finite hosted models do not establish actual guest,physical or all-interleaving behavior.','SMP transfer uses a separate frozen CMK2 provider;the T48 successor remains one CPU.','Strict selected PCI/ELF/ABI profiles are narrower than general specifications.','Clean emulator restart is not physical power-loss evidence.','Historical Intel/psABI editions and unavailable published C17 wording retain their authority limits.']};updates.append(row)
   for variant_row in c['variants']:
    child=copy.deepcopy(row);child.update(id=variant_row['id'],parent_id=c['id'],original_variant_task=variant_row['task'],minimum_days=variant_row['minimum_days'],closure_rationale=variant,variant_ids=[variant_row['id']],variant_readiness='Exact author implementation or explicit premise/counterexample prepared;independent delayed gate unobserved.');updates.append(child)
 resources=json.loads((HERE/'resources.json').read_text());pins=[*canonical,ROOT/CAMPAIGN,Path(__file__),HERE/'criterion_notes.py',HERE/'evidence_index.json',HERE/'resources.json',HERE/'source_access.json',HERE/'research_gate.json',ROOT/'tools/check_kernel_followthrough.py',*[ROOT/p for p in public_paths()],REF/'ANALYSIS.md',*[ROOT/p for p in v['inspected_paths']]]
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in pins},'evidence_index':(HERE/'evidence_index.json').relative_to(ROOT).as_posix(),'source_records':[{'id':x['id'],'path':(HERE/'resources.json').relative_to(ROOT).as_posix(),'passages':x['locators']} for x in resources['resources']],'campaigns':[{'producer':'tools/freeze_kernel_followthrough.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':v['open_gates'],'scope':'All39 original T48.02–14 criteria and39 exact delayed variants;T48.01 remains the separate frozen6-ID package.'});print(json.dumps({'criteria':39,'updates':len(updates)}))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--prepare',action='store_true');a=p.parse_args();prepare() if a.prepare else main()
