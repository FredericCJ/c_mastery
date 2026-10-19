"""Bind exact T52.08 criteria to modular-time and actual two-timer evidence."""
from copy import deepcopy
import hashlib,json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_realtime_tickless/support.json'
CONTRACT='labs/realtime_depth/tickless/CONTRACTS.md';PROJECT='PR-T52-08'
CAMPAIGN='reviews/v5/expansion/legacy_realtime_tickless/campaign.json'
CORRECTION='research/v5/expansion/legacy_realtime_tickless/stopped_state_correction.json'
SCOPES={
 'T52.08.a':'Original forward-distance9, equality, half-range ambiguity and separate infinite state; all65,536 eight-bit pairs independently compared with explicit temporal provenance premises.',
 'T52.08.b':'Checked full-width rational ceiling and absolute not-before conversion distinguish nominal3ticks from call-phase elapsed time; independent zero/extremal/rational-rate cases preserve outputs on failure and avoid implicit divide helpers.',
 'T52.08.c':'Actual retained CMX sleep/tick/cancel operations compare periodic and tickless wrap/equality histories, two-phase earlier-insertion/pending-IRQ checks and retained-fraction auxiliary resume; actual owned QEMU SysTick/CMSDK adapter separately observes these mechanisms.'}

def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')

def attach(record):
    if record['project']!=PROJECT:return record
    for criterion in record['criteria']:
        criterion['successor_support']={'path':SUPPORT,'id':criterion['id']}
    links=record.setdefault('additional_successor_support',[])
    links[:]=[r for r in links if r.get('path')!=SUPPORT]
    links.append({'path':SUPPORT,'criterion_ids':[r['id'] for r in record['criteria']],
      'scope':'Exact modular-time, rational-conversion, actual CMX and dedicated emulator timer evidence; finite histories and deliberately stopped SysTick are not physical sleep-state or board-timing validation.'})
    links.sort(key=lambda r:r['path'])
    if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
    public=[p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/realtime_depth/tickless').iterdir() if p.is_file()]
    public+=['labs/execution/include/cmx.h','labs/execution/src/cmx.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in sorted(public):
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record

def main():
    sys.path.insert(0,str(ROOT/'tools'))
    from expansion_evidence import require_batch_research
    from check_tickless_depth import validate_campaign
    require_batch_research(BASE)
    validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
    canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json')
    record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record)
    updates=[]
    for criterion in record['criteria']:
        row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],
          'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
          'dimensions':{
            'teaching':{'state':'complete-for-explicit-modular-clock-and-adapter-contract','evidence':[CONTRACT]},
            'implementation':{'state':'complete-for-original-finite-criteria-and-changed-conditions','evidence':['assessor/legacy_realtime/tickless.c','assessor/legacy_realtime/tickless_port.c','labs/realtime_depth/tickless/oracle.py','assessor/execution/cmx.c']},
            'author_validation':{'state':'passed-hosted-oracle-strict-M4-and-actual-emulated-timers','evidence':[CAMPAIGN,CORRECTION]},
            'external_review':{'state':'not-run','evidence':[]},
            'independent_learner':{'state':'not-assessed','evidence':[]},
            'physical_observation':{'state':'not-run-physical-clock-and-sleep-not-claimed','evidence':[]}},
          'variant_ids':[v['id'] for v in criterion['variants']],
          'required_environment':'C17 externally serialized four-slot queue with explicit actual CMX import; six hosted GCC/Clang profiles and independent Python integer oracle; three audited strict Cortex-M4 profiles and owned QEMU MPS2-AN386 SysTick/CMSDK image.',
          'external_blockers':['Independent review and delayed unaided learner attempts remain separate.','Physical oscillator/interrupt bounds and auxiliary-clock availability in a selected board sleep state require physical evidence.'],
          'evidence_limits':['Modular ordering requires independently justified half-range provenance and observation intervals; raw pairs cannot detect arbitrary missed wraps.','The71,216 independent rows and finite reprogram traces falsify named defects but do not prove every schedule.','Actual QEMU instruction-clock timer counts and deliberate SysTick disable are emulator observations, not physical low-power entry, energy or worst-case interrupt latency.','The adapter implements24-bit chunking; this campaign does not separately exercise a deadline exceeding one hardware counter chunk.','Inspected upstream QEMU10.2.1 source identifies model behavior; the installed distribution patch set was not exhaustively audited.','The recorded PM0214 single-shot wording inconsistency is not silently treated as a resolved exact physical-cycle guarantee.']}
        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row)
            child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],
              original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
              variant_readiness='Exact eight-bit enumeration, changed rational frequency or separate-clock resume implemented and author-executed; the independent seven-day learner attempt remains unassessed.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective')
    gap.update(id='G-LCON-PR-T52-08',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
      closure_rationale='All three original timing criteria and exact changed conditions have source-bound implementation and falsification evidence; source limitations, physical clock/sleep behavior and independent assessment remain explicit.',
      criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']])
    updates.append(gap)
    resource_path='research/v5/expansion/legacy_realtime_tickless/resources.json'
    resources=json.loads((ROOT/resource_path).read_text(encoding='utf-8'))
    pins=[CONTRACT,CAMPAIGN,CORRECTION,canonical.relative_to(ROOT).as_posix(),'tools/check_tickless_depth.py',
      'assessor/legacy_realtime/tickless.md','research/v5/expansion/legacy_realtime_tickless/author_support.py',resource_path,
      'research/v5/expansion/legacy_realtime_tickless/research_gate.json','research/v5/expansion/legacy_realtime_tickless/source_notes.json']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
      'source_records':[{'id':r['id'],'path':resource_path,'passages':r['locators']} for r in resources['resources']],
      'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
      'campaigns':[{'producer':'tools/run_tickless_depth.py','report':CAMPAIGN}],
      'remaining_author_work':[],
      'remaining_external_work':['Independent review and delayed unaided learner assessment.','Physical clock/sleep-state and timing observations before asserting a physical guarantee.'],
      'scope':'Only T52.08 original criteria and exact changed conditions; separate finite core/actual CMX/actual emulated-device observations, with no closure of other realtime topics or physical/learner claims.'})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))

if __name__=='__main__':main()
