"""Exact T54 additive sidecar; original contracts and frozen providers preserved."""
import argparse,copy,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent;sys.path.insert(0,str(HERE))
from criterion_notes import NOTES
PROJECTS=[f'PR-T54-{i:02}' for i in range(1,11)];LAB=ROOT/'labs/amp_followthrough';REF=ROOT/'assessor/amp_followthrough';SUPPORT='research/v5/expansion/legacy_amp_followthrough/support.json';CAMPAIGN='reviews/v5/expansion/legacy_amp_followthrough/campaign.json';CONTRACT='labs/amp_followthrough/CONTRACTS.md'
OPEN=['Independent external review not run.','Original minimum-seven-day independent learner variants not administered.','Physical H745 dual-core, cache/DMA, reset, clock calibration and external timing observations not run.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,v):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
def records():return [json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(p+'.json')).read_text()) for p in PROJECTS]
def public_paths():
 paths=[p for p in LAB.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
 paths += [ROOT/v for v in json.loads((LAB/'providers.json').read_text()).values()]
 paths += [ROOT/p for p in ('labs/execution/include/cmx.h','labs/execution/include/cmx_port.h','labs/peripheral_depth/dma/dma.h','labs/interrupt_depth/lease/lease.h','labs/interrupt_depth/lease/bridge.h','labs/firmware_lifecycle/targets/h745_cache.h','curriculum/modules/T54.md')]
 paths += [ROOT/'fixtures/os_v3'/(p+'.json') for p in PROJECTS]+[ROOT/'curriculum/projects'/(p+'.md') for p in PROJECTS]
 return sorted({p.relative_to(ROOT).as_posix() for p in paths})
def attach(r):
 if r['project'] not in PROJECTS:return r
 for c in r['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 entries=r.setdefault('additional_successor_support',[]);entries[:]=[x for x in entries if x.get('path')!=SUPPORT];entries.append({'path':SUPPORT,'criterion_ids':[x['id'] for x in r['criteria']],'scope':'Exact T54 original and changed-condition native/model/image/emulator author support; physical and independent gates remain open.'});entries.sort(key=lambda x:x['path'])
 if CONTRACT not in r['shared_contracts']:r['shared_contracts'].append(CONTRACT)
 # The source-checkout fixture producer is research evidence, outside the
 # learner export allowlist. Keep it in resources, never as an exported asset.
 r['supplied_artifacts']=[p for p in r['supplied_artifacts'] if p!='research/v3/verify_fixtures.py']
 for p in public_paths():
  if p not in r['supplied_artifacts']:r['supplied_artifacts'].append(p)
 return r
def prepare():
 rows=[];analysis=['# T54 private assessor derivations','','These author derivations are separate from the incomplete public learner package. Evidence kinds are restricted to each exact mechanism.','']
 for r in records():
  unit=['# '+r['project']+' followthrough','','Read the original module/project/fixture and `../CONTRACTS.md`. Use the appropriate native mechanism and target mode; predict before executing or reading assessor material.','']
  for c in r['criteria']:
   key=c['objective'].removeprefix('T54.');teach,base,changed,selectors,scope=NOTES[key]
   unit += ['## '+c['objective'],'',teach,'','Original task: '+c['task'],'','Changed condition: '+c['variants'][0]['task'],'','Deliver your initial prediction, owned storage/identity ledger, completed source or explicit premise derivation, a causal counterexample, raw commands/source hashes and changed-condition explanation. Independent delayed assessment and physical observation remain separate.','']
   analysis += ['## '+c['id'],'',base,'','Changed condition: '+changed,'','Evidence selectors: '+selectors+'.','','Scope: '+scope,'']
   rows.append(dict(id=c['id'],objective=c['objective'],project=r['project'],original_task=c['task'],original_expected=c['verification']['expected'],changed_conditions=copy.deepcopy(c['variants']),teaching='labs/amp_followthrough/units/'+r['project']+'.md',mechanism_checks=selectors.split(),evidence_scope=scope))
  path=LAB/'units'/(r['project']+'.md');path.parent.mkdir(parents=True,exist_ok=True);path.write_text('\n'.join(unit)+'\n',encoding='utf-8')
 assert len(rows)==len(NOTES)==30
 write(LAB/'criteria.json',dict(schema_version=1,criteria=rows));(REF/'ANALYSIS.md').write_text('\n'.join(analysis)+'\n',encoding='utf-8');print('Prepared30 original criteria and30 exact original variants.')
def freeze():
 sys.path[:0]=[str(ROOT),str(ROOT/'tools')];from check_amp_followthrough import validate_campaign
 campaign=json.loads((ROOT/CAMPAIGN).read_text());validate_campaign(campaign);updates=[];canonical=[]
 for r in records():
  path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(r['project']+'.json');write(path,attach(r));canonical.append(path)
  for c in r['criteria']:
   teach,base,changed,selectors,scope=NOTES[c['objective'].removeprefix('T54.')]
   row=dict(id=c['id'],project=r['project'],original_objective=c['objective'],original_source=path.relative_to(ROOT).as_posix(),closure_rationale=base,mechanism_checks=selectors.split(),evidence_scope=scope,variant_ids=[v['id'] for v in c['variants']],dimensions={'teaching':{'state':'complete-exact-original-and-changed-condition-contracts','evidence':[CONTRACT,'labs/amp_followthrough/criteria.json','labs/amp_followthrough/units/'+r['project']+'.md']},'implementation':{'state':'implemented-or-derived-exact-native-model-target-mechanisms','evidence':['assessor/amp_followthrough/ANALYSIS.md','assessor/amp_followthrough/core/ack_probe.c','assessor/amp_followthrough/core/amp.c','assessor/amp_followthrough/core/buffers.c','assessor/amp_followthrough/core/doorbell.c','assessor/amp_followthrough/core/ledger.c','assessor/amp_followthrough/target/board.c','assessor/amp_followthrough/target/pipeline.inc','assessor/amp_followthrough/models/analysis.py']},'author_validation':{'state':'passed-source-bound-native-image-and-actual-single-core-emulator-checks','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run;H745-image-audit-and-single-core-AN386-distinct','evidence':[]}},external_blockers=OPEN,required_environment='C17 hosted GCC/Clang, explicit original ARM GCC/Clang objects, ARM binutils and AN386 QEMU; no target runtime archives.',evidence_limits=['AN386 executes the same task bodies on one core, not H745 parallel/cache/DMA hardware.','Synthetic event/cache/clock/register models retain their stated premises.','Native callback snapshots are not physical DMA observations.','Controlled drained rendezvous is distinct from unexpected peer reset and pending-effect recovery.','Per-function frame/map and actual canary checks do not prove all-interrupt stack or WCET bounds.','Author execution does not administer independent delayed assessment.'])
   updates.append(row)
   for v in c['variants']:
    d=copy.deepcopy(row);d.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],closure_rationale=changed,variant_ids=[v['id']],variant_readiness='Exact changed-condition implementation or explicit premise/counterexample prepared; independent original minimum delay unobserved.');updates.append(d)
 pins={**campaign['source_sha256'],CAMPAIGN:sha(ROOT/CAMPAIGN),**{p.relative_to(ROOT).as_posix():sha(p) for p in canonical}}
 write(ROOT/SUPPORT,dict(schema_version=1,required=True,updates=updates,source_sha256=pins,source_records=[{'id':'R-T54-AMP-FOLLOWTHROUGH','path':'research/v5/expansion/legacy_amp_followthrough/resources.json','passages':['Exact source, primary manual locators, original contracts and provider boundaries']}],campaigns=[{'producer':'tools/run_amp_followthrough.py','report':CAMPAIGN}],remaining_author_work=[],remaining_external_work=OPEN,scope='All30 original T54 criteria and30 canonical V1 variants; exact source-bound author work, no physical or independent mastery promotion.'));print(json.dumps(dict(criteria=30,updates=len(updates))))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--prepare',action='store_true');a=p.parse_args();prepare() if a.prepare else freeze()
