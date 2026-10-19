"""Prepare exact T50 teaching; attach only after frozen campaign/export guards."""
import argparse,copy,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(HERE));from criterion_notes import NOTES
from evidence_map import EVIDENCE
LAB=ROOT/'labs/mcu_followthrough';REF=ROOT/'assessor/mcu_followthrough'
PROJECTS=[f'PR-T50-{i:02}' for i in range(1,13)]
SUPPORT=HERE.relative_to(ROOT).as_posix()+'/support.json'
CONTRACT='labs/mcu_followthrough/CONTRACTS.md'
OPEN=['Independent external review not run.','Minimum-seven-day independent learner assessment not observed.','Physical board timing,energy,clock calibration and DMA observations not run.','Published C17 wording and full current silicon errata not newly inspected.']
def write(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf-8',newline='\n')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def records():return [json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(p+'.json')).read_text()) for p in PROJECTS]
def public_paths():
 paths=[p for p in LAB.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
 paths += [ROOT/'labs/execution/include/cmx.h',ROOT/'labs/execution/src/cmx.c',ROOT/'labs/execution/src/support.c']
 paths += [ROOT/'fixtures/os_v3'/(p+'.json') for p in PROJECTS]
 paths += [ROOT/'curriculum/modules/T50.md',*[ROOT/'curriculum/projects'/(p+'.md') for p in PROJECTS]]
 return sorted({p.relative_to(ROOT).as_posix() for p in paths})
def attach(r):
 if r['project'] not in PROJECTS:return r
 for c in r['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 entries=r.setdefault('additional_successor_support',[]);entries[:]=[x for x in entries if x.get('path')!=SUPPORT]
 entries.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in r['criteria']],'scope':'Exact T50 original and delayed MCU mechanisms;model,native,emulator and external evidence remain distinct.'});entries.sort(key=lambda x:x['path'])
 if CONTRACT not in r['shared_contracts']:r['shared_contracts'].append(CONTRACT)
 for path in public_paths():
  if path not in r['supplied_artifacts']:r['supplied_artifacts'].append(path)
 return r
def selectors(key):
 return EVIDENCE[key][0].split()

def prepare():
 analysis=['# T50 private assessor derivations','','Completed reference work is never exported as a learner solution. Each record below addresses one exact criterion and its original changed condition. Models,source inspection,native execution and actual emulation have separate scopes.',''];rows=[]
 for r in records():
  unit=['# '+r['project']+' followthrough','','Read `../CONTRACTS.md`, the original project and fixture first. Predict each result before compiling. Use the matching core/target mode and preserve the original delayed minimum-seven-day requirement.','']
  for c in r['criteria']:
   key=c['objective'].removeprefix('T50.');teach,base,changed=NOTES[key]
   unit += ['## '+c['objective'],'',teach,'','Original task: '+c['task'],'','Changed condition: '+c['variants'][0]['task'],'','Deliver: initial prediction,source/ownership or frame ledger,completed mechanism,named counterexample,raw commands/identities,and a revised premise argument. Keep actual learner assessment and physical observations unclaimed.','']
   analysis += ['## '+c['id'],'',base,'','Delayed: '+changed,'','Evidence selectors: '+', '.join(selectors(key))+'.','','Exact observation scope: '+EVIDENCE[key][1],'','Reject copied markers without causal explanation; reject native/model results offered as target or physical evidence.','']
   rows.append({'id':c['id'],'objective':c['objective'],'project':r['project'],'original_task':c['task'],'original_expected':c['verification']['expected'],'changed_conditions':copy.deepcopy(c['variants']),'teaching':'labs/mcu_followthrough/units/'+r['project']+'.md','mechanism_checks':selectors(key),'evidence_scope':EVIDENCE[key][1],'scope':'Exact task plus explicit changed-premise implementation or worked counterexample; no independent learner administration.'})
  path=LAB/'units'/(r['project']+'.md');path.parent.mkdir(parents=True,exist_ok=True);path.write_text('\n'.join(unit)+'\n',encoding='utf-8')
 assert len(rows)==36 and len(NOTES)==36
 write(LAB/'criteria.json',{'schema_version':1,'criteria':rows});(REF/'ANALYSIS.md').write_text('\n'.join(analysis)+'\n',encoding='utf-8')
 print('Prepared36 exact original criteria and36 delayed variants; no attachments made.')
def main():
 sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
 from check_mcu_followthrough import validate_campaign,CAMPAIGN
 campaign=json.loads((ROOT/CAMPAIGN).read_text());validate_campaign(campaign);updates=[];canonical=[]
 for r in records():
  path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(r['project']+'.json');write(path,attach(r));canonical.append(path)
  for c in r['criteria']:
   key=c['objective'].removeprefix('T50.');teach,base,changed=NOTES[key]
   row={'id':c['id'],'project':r['project'],'original_objective':c['objective'],'original_source':path.relative_to(ROOT).as_posix(),'closure_rationale':base,'mechanism_checks':selectors(key),'evidence_scope':EVIDENCE[key][1],'variant_ids':[v['id'] for v in c['variants']],'dimensions':{'teaching':{'state':'complete-exact-original-and-changed-condition-contracts','evidence':[CONTRACT,'labs/mcu_followthrough/criteria.json','labs/mcu_followthrough/units/'+r['project']+'.md']},'implementation':{'state':'implemented-or-derived-exact-native-model-target-mechanisms','evidence':['assessor/mcu_followthrough/ANALYSIS.md','assessor/mcu_followthrough/core/mcu.c','assessor/mcu_followthrough/target/direct.S','assessor/mcu_followthrough/target/context.S','assessor/mcu_followthrough/target/preemptive.c','assessor/mcu_followthrough/target/sleep.c']},'author_validation':{'state':'passed-source-bound-native-and-actual-emulator-checks','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run;model-and-emulator-distinct','evidence':[]}},'external_blockers':OPEN,'required_environment':'C17 GCC15.2/Clang21.1.8 hosted Linux; explicit ARM GCC/Clang objects, ARM binutils and QEMU10.2.1 AN386/AN500 softfp; no target runtime archives.','evidence_limits':['Author execution does not administer independent delayed learner assessment.','Finite history and cancellation/publication models are not hardware/all-interleaving proofs.','Borrowed software leases and modeled DMA abandonment are not physical DMA observations.','Selected linked call graphs and compiler frames bound declared paths only; new priorities, callbacks or code invalidate those premises.','QEMU clock counts and semihost transport do not establish board timing,energy or oscillator calibration.','C17 freestanding, AAPCS, exception architecture, linker policy and board map are separate authorities.']};updates.append(row)
   for v in c['variants']:
    delayed=copy.deepcopy(row);delayed.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],closure_rationale=changed,variant_ids=[v['id']],variant_readiness='Exact changed-condition implementation or explicit premise/counterexample prepared;independent minimum-seven-day learner gate unobserved.');updates.append(delayed)
 resources=json.loads((HERE/'resources.json').read_text());pins={**campaign['source_sha256'],CAMPAIGN:sha(ROOT/CAMPAIGN),**{p.relative_to(ROOT).as_posix():sha(p) for p in canonical}}
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':(HERE/'resources.json').relative_to(ROOT).as_posix(),'passages':r['locators']} for r in resources['resources']],'source_sha256':pins,'campaigns':[{'producer':'tools/run_mcu_followthrough.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':OPEN,'scope':'All36 original T50 criteria and36 exact original delayed variants; distinct author/model/native/emulator/external evidence.'})
 print(json.dumps({'criteria':36,'updates':len(updates)}))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--prepare',action='store_true');a=p.parse_args()
 if a.prepare:prepare()
 else:main()
