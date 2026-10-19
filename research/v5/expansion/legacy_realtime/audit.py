"""Keep the inspected original T52 obligations visible while coherent successors are implemented."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
MECHANISMS={
 '01':'Stable RM/DM assignment with transactional output; actual multi-release jobs, jitter/nonpreemptive variants and sufficient-bound counterexample.',
 '02':'Multiple simultaneous job identities, deadline-before-release accounting, explicit bounded backlog, absolute EDF selection, overhead/nonpreemptive variants.',
 '03':'Full-width checked recurrence, explicit outcome/publication contract, independent arithmetic oracle and interrupt-arrival variant.',
 '04':'Declared resource-use table, nested immediate ceilings and donations, rejected dynamic/undeclared clients; SMP proof boundary.',
 '05':'Background/polling/per-dispatch and grouped service; fixed ledger saturation, conservative merge and sustained-overload conservation.',
 '06':'Soft CBS versus throttling, equality policy, competing reservations; inspected different-deadline policy and mutex charging.',
 '07':'Atomic admission, explicit request/job dispositions, bounded queue/backpressure dependency, supervisor, criticality and blocked-I/O changes.',
 '08':'Exhaustive modular horizons, rational not-before conversion and actual timer reprogram/enrollment boundary; sleep-clock accounting.',
 '09':'Transactional stable FFD placement, original IDs, exact per-core demand and exhaustive small assignments; affinity and migration.',
 '10':'Sequential-job ownership on multiple CPUs, Dhall/migration/interference traces; explicit DAG readiness and pinned restrictions.',
 '11':'Separate original and pinned FreeRTOS queue/inversion adapters with timeout/priority controls; actual static MCU port comparison.',
 '12':'Disposable-VM Linux scheduling classes with real readback, bounded supervisor, permission/refusal accounting and raw distribution/affinity variants.'}
def main():
    paths=[ROOT/'curriculum/modules/T52.md',ROOT/'labs/execution/README.md',ROOT/'labs/execution/include/cmx_advanced.h',
      ROOT/'assessor/execution/advanced.c',ROOT/'assessor/execution/cmx.c',ROOT/'labs/execution/tests/contracts.c']
    rows=[]
    for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T52-*.json')):
        value=json.loads(path.read_text(encoding='utf-8'));project=value['project'];number=project[-2:];paths.append(path)
        if number in ('01','02','03','04','05'):paths.append(ROOT/'curriculum/projects'/(project+'.md'))
        rows.append({'project':project,'original_gap_id':'G-LCON-'+project,'criteria':[c['id'] for c in value['criteria']],
          'changed_conditions':[v['id'] for c in value['criteria'] for v in c['variants']],
          'inspected_scope':'Complete original leaf tasks/variants and module teaching; '+('complete parent brief and bounded corresponding retained implementation.' if number in ('01','02','03','04','05') else 'bounded retained model source only where named; full project-specific implementation/source inspection remains author work.'),
          'required_mechanism':MECHANISMS[number],
          'current_successor':'research/v5/expansion/legacy_realtime_response/support.json' if number=='03' else 'research/v5/expansion/legacy_realtime_jobs/support.json' if number in ('01','02') else 'research/v5/expansion/legacy_realtime_ceiling/support.json' if number=='04' else 'research/v5/expansion/legacy_realtime_server/support.json' if number=='05' else None,
          'author_state':'current-reference-and-changed-model-conditions-executed' if number in ('01','02','03','04','05') else 'required-author-implementation-pending',
          'external_state':'Independent review/learner and physical claims remain separate; missing author work is not relabeled an external blocker.'})
    result={'schema_version':1,'scope':'Original T52 source/depth audit and required continuation; no whole-domain closure.',
      'counts':{'projects':12,'leaf_criteria':36,'changed_conditions':36,'original_gap_rows':12},'projects':rows,
      'source_findings':['Retained cmx_priority_order writes prefixes before invalid late input; transactional wrapper needed.',
        'Retained cmx_simulate supports constrained deadlines with one remaining/deadline slot per task and replaces at release; arbitrary-deadline/backlog variants require distinct jobs.',
        'Retained cmx_partition uses input order rather than first-fit-decreasing and publishes prefixes before late refusal.',
        'Retained cmx_response has bounded<=1e6 parameters and128 iterations; completed successor adds explicit statuses/full-width checked arithmetic.',
        'Book pages inspected so far do not establish all server/multicore/jitter proofs; accessible remaining source passages must still be read before those extensions.'],
      'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
      'limits':['Inspecting a criterion does not execute it. Baseline model/test names alone do not close its variants.','Original finite/physical/learner records remain unchanged.']}
    (BASE/'depth_audit.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n')
if __name__=='__main__':main()
