"""Close exact T07 author obligations; retain original tasks and evidence snapshots."""
from pathlib import Path
import sys,json
ROOT=Path(__file__).resolve().parents[4];sys.path.insert(0,str(ROOT/'tools'))
from common import read_json,dump,sha
from check_systems_toolchain import validate
from expansion_evidence import require_batch_research
folder=Path(__file__).parent
require_batch_research(folder)
receipt='reviews/v5/expansion/legacy_systems_toolchain/toolchain_validation.json'
report=read_json(ROOT/receipt);errors=validate(report)
if errors:raise SystemExit(str(errors))
public='labs/foundations/toolchain/CONTRACTS.md';analysis='assessor/legacy_systems/toolchain/ANALYSIS.md'
support=folder/'support.json';relative=support.relative_to(ROOT).as_posix()
observations={(r['objective'],r['variant']):r for r in report['observations']}
updates=[];criteria=[];canonical=[]
for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T07-*.json')):
    record=read_json(path);origin=path.relative_to(ROOT).as_posix()
    public_inputs=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/foundations/toolchain').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
    record['supplied_artifacts']=list(dict.fromkeys(record['supplied_artifacts']+public_inputs))
    for criterion in record['criteria']:
        oid=criterion['objective'];criteria.append({'id':criterion['id'],'objective':oid,'project':record['project'],
          'original_source':origin,'task':criterion['task'],'teaching':[public,origin],
          'assessment_analysis':[analysis,record['assessor_analysis']],
          'ordinary_observation':observations[oid,False],'transfer_observation':observations[oid,True],
          'independent_assessment':'not-assessed; minimum7days unchanged','receipt':receipt})
        for ident,variant in [(criterion['id'],False),*[(v['id'],True) for v in criterion['variants']]]:
            observation=observations[oid,variant]
            updates.append({'id':ident,'original_source':origin,'original_objective':oid,'project':record['project'],
              'dimensions':{'teaching':{'state':'complete','evidence':[public,origin,analysis]},
                'implementation':{'state':'complete-for-declared-reference-contract','evidence':['assessor/legacy_systems/toolchain/'+p for p in ['stages.py','graphs.py','packages.py','provenance.py']]},
                'author_validation':{'state':'passed-with-stated-scope','evidence':[receipt,'reviews/v5/expansion/legacy_systems_toolchain/validation_check.json']},
                'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},
                'physical_observation':{'state':'not-applicable-to-hosted-or-explicit-emulator-contract; no physical observation claimed','evidence':[]}},
              'closure_rationale':observation['claim'],'required_environment':'Linux GCC15/Clang21, Make/CMake/Ninja/binutils, Arm GCC and explicit QEMU Cortex-M3 transfer; matching LLVM bitcode tools.',
              'validation_method':'Exact argv and original specimen bytes; independent arithmetic/byte expectations; intended diagnostic versus infrastructure classification; actual graph, package, foreign object and emulator checks.',
              'evidence_limits':observation['limitations']+report['limits']})
        criterion['successor_support']={'path':relative,'criterion_id':criterion['id']}
    record['successor_support']={'path':relative,'criterion_ids':[r['id'] for r in record['criteria']],
      'scope':'Source-bound author experiments and assessor transfer package; independent learner evidence remains separate.'}
    dump(path,record);canonical.append(origin)
parent={'id':'LSYS-EXEC-T07','dimensions':updates[0]['dimensions'],
 'closure_rationale':'All36 original criteria and36 delayed packages have individual implementation/analysis and recorded observations; no generic test is used as blanket evidence.',
 'evidence_limits':report['limits'],'criteria':[r['id'] for r in criteria]}
updates.append(parent)
pins=[public,analysis,receipt,'reviews/v5/expansion/legacy_systems_toolchain/validation_check.json',*canonical]
dump(support,{'schema_version':1,'required':True,'scope':'All36 original T07 supplemental criteria, their36 transfers and LSYS-EXEC-T07; historical snapshots retained.',
 'source_sha256':{p:sha(ROOT/p) for p in pins},'criteria':criteria,'updates':updates,
 'source_records':[{'id':r['id'],'path':'research/v5/expansion/legacy_systems_toolchain/resources.json','passages':r['locators']} for r in read_json(folder/'resources.json')['resources']]})
dump(folder/'gap_ledger.json',{'schema_version':1,'required':True,'gaps':[{'id':'LSYS-EXEC-T07','originating_record':'research/v5/expansion/legacy_systems/gap_ledger.json',
 'status':'author-implemented-validated; external-dimensions-separate','requirement':'36 exact original criteria and all36 original changed conditions','support':relative,
 'remaining':['Non-author technical review','Independent ordinary and delayed learner attempts','Physical evidence only for a later separately claimed board environment']}],
 'historical_note':'Original gap/remaining ledgers retain their earlier unattempted snapshot; this additive successor is source-bound current evidence.'})
path=ROOT/'curriculum/v5/expansion/implementation_updates.json';state=read_json(path);index={r['id']:r for r in state['updates']}
for row in updates:index[row['id']]=row
state['updates']=list(index.values());dump(path,state)
print({'criteria':len(criteria),'closed_author_obligations':len(updates),'independent_learner_claims':False})
