"""Bind original checkpoint criteria to exact successor evidence without changing history."""
import argparse
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[4]
HERE=Path(__file__).resolve().parent
REGISTER='research/v5/expansion/legacy_completion/checkpoint_support.json'
LAB='labs/checkpoint_depth/'
PRIVATE='assessor/checkpoint_depth/'
REPORT='reviews/v5/expansion/legacy_completion/'

def block(core,tests,report='checkpoint_depth_campaign.json'):
 return {'implementation':[PRIVATE+x for x in core],'tests':[LAB+x for x in tests],'reports':[REPORT+report]}

SUPPORT={
 'memory':block(['checkpoint_memory.c'],['tests/memory.c']),
 'transfer_memory':block(['transfer/memory.c'],['transfer/memory_tests.c'],'checkpoint_transfer.json'),
 'adapters':block(['transfer/adapters.c'],['transfer/adapters_tests.c'],'checkpoint_transfer.json'),
 'dependencies':{'implementation':[LAB+'transfer/dependency_probe.c',PRIVATE+'checkpoint_memory.c'],'tests':['tools/run_checkpoint_dependencies.py'],'reports':[REPORT+'checkpoint_dependencies.json']},
 'measurement':block(['checkpoint_memory.c'],['tests/memory_experiment.c'],'checkpoint_memory.json'),
 'entry':block(['targets/entry.S','targets/image.c','targets/byte_helpers.c'],['tests/helper_probe.c','tests/resolved_forbidden.c'],'checkpoint_entry.json'),
 'writer':block(['checkpoint_writer.c','checkpoint_channel.c'],['tests/writer.c']),
 'protocol':block(['checkpoint_depth.c'],['tests/tests.c','tests/check_stream.py']),
 'transfer_protocol':block(['transfer/protocol.c'],['transfer/protocol_tests.c','transfer/check_protocol.py'],'checkpoint_transfer.json'),
 'auth':block(['transfer/auth.c'],['transfer/auth_tests.c','transfer/openssl_mac.c'],'checkpoint_transfer.json'),
 'durable':block(['transfer/durable.c'],['transfer/durable_tests.c'],'checkpoint_transfer.json'),
 'minimize':{'implementation':[PRIVATE+'checkpoint_depth.c'],'tests':['tools/minimize_checkpoint_stream.py'],'reports':[REPORT+'p3_minimized.json']},
 'firmware':block(['checkpoint_firmware.c'],['tests/firmware.c']),
 'firmware_target':block(['targets/cortex_start.S','targets/cortex_guest.c','targets/f439.c','targets/arm_helpers.c'],['targets/README.md'],'checkpoint_firmware.json'),
 'halfduplex':block(['transfer/halfduplex.c'],['transfer/halfduplex_tests.c'],'checkpoint_transfer.json'),
 'timing':{'implementation':['tools/verify_checkpoint_timing.py'],'tests':['fixtures/legacy_checkpoints/transfer/P4_timing.json'],'reports':[REPORT+'checkpoint_timing.json']},
 'async':block(['checkpoint_async.c'],['tests/async.c']),
 'linux':block(['checkpoint_linux.c'],['tests/linux.c']),
 'operation':block(['checkpoint_operation.c'],['tests/operation.c']),
 'numeric':block(['checkpoint_numeric.c'],['tests/numeric.c']),
 'vector':block(['checkpoint_vector.c'],['tests/vector.c']),
 'numeric_experiment':block(['checkpoint_numeric.c','checkpoint_vector.c'],['tests/numeric_experiment.c'],'checkpoint_numeric.json')
}

# Each line is a reviewed mechanism, not a quota-generated exercise. Original
# task/rubric/variant text remains canonical in the linked assessment record.
ROWS={
'PR-P1-01':[
 ('T14.01.c','memory transfer_memory','Declared byte/typed storage comparison and actual malloc-backed typed records; typed writes establish allocated object types.'),
 ('T14.05.c','memory transfer_memory','Four-slot FIFO prefix/atomic admission; changed linked queue with independent bounded node exhaustion and exact accepted prefix.'),
 ('T16.02.a','memory','32-byte/header3/alignment8 requests5,9,1; overflow/capacity preservation and supplied alignment weaker than required refusal.'),
 ('T18.04.b','memory transfer_memory','Arena pins distinguish queued and callback references; cancelled lease retains its pin until matching quiescent completion and rejects duplicates.'),
 ('T18.04.c','memory transfer_memory','Slot/generation validation, retirement and genuine backing relocation with pins/refusal and handle-preserving typed copy.')],
'PR-P1-02':[
 ('T13.01.c','dependencies','Actual native linked LTO/provider audit with explicit stack-protector dependency, resolved forbidden malloc and hidden blocking poll callback controls.'),
 ('T16.14.b','memory transfer_memory','Every initialization/node/growth acquisition cut; unchanged failed growth, alignment/capacity boundaries, same four inputs at half capacity.'),
 ('T16.14.c','measurement transfer_memory','744 raw operation samples separate instrumentation and allocation scope; fixed-capacity changed payload distribution derives arena retention/pool metadata honestly.'),
 ('T21.04.b','transfer_memory adapters','Three-stage acquisition/reverse cleanup, pending unregister ownership and actual OwOwner delayed-release comparison with size/callback observations.')],
'PR-P2-01':[
 ('T09.01.b','entry','Actual ET_EXEC/ET_DYN entry/load segments, zero-fill/data sentinels and relocation-free PIE with runtime load-bias observations.'),
 ('T10.01.c','entry','Original x86-64 Linux initial-stack adapter and changed arguments; explicit C-call alignment and syscall termination.'),
 ('T11.01.c','entry dependencies','Two compilers/two optimizations, explicit providers, emitted memory helpers, dependency closure and forbidden-resolved-provider falsification.')],
'PR-P2-02':[
 ('T10.07.b','writer adapters','3/EINTR/2/4 and zero/error partial progress plus actual nonblocking libc EAGAIN/readiness under one callback per turn.'),
 ('T23.04.c','writer','Pool-to-writer ownership retains active65/66 across retry/failure; arbitrary bounded spans retain exact accepted prefix.'),
 ('T23.04.b','writer adapters entry','Raw syscall, scripted and actual libc adapters preserve the same progress result while distinguishing error conventions/dependencies.'),
 ('T21.02.b','writer adapters','Primary and cleanup causes remain separate without logging; actual er_progress adapter preserves count/cause and records sizes/static branches/callbacks.')],
'PR-P3-01':[
 ('T04.06.b','protocol transfer_protocol','Exact unaligned LE wire, independent whole-buffer oracle and explicit big-endian numeric representation model; no physical big-endian target claim.'),
 ('T15.12.b','protocol transfer_protocol','Original v1 admission plus real v2 flags/timestamp and extended CRC extent; old parser explicitly rejects v2.'),
 ('T15.12.c','protocol auth','Original CRC differential plus local HMAC-SHA256 envelope with inspected primitive, bound header/key/epoch, replay/failure publication and forged-header controls.'),
 ('T18.01.b','protocol transfer_protocol','Invalid/incomplete/capacity output preservation and exact supported in-place payload shift with validated preconditions.')],
'PR-P3-02':[
 ('T15.13.b','protocol transfer_protocol','All original short partitions and paused consumer; changed queue capacities1/3 and explicit held-output/backpressure schedules.'),
 ('T15.13.c','protocol transfer_protocol','Nested-frame resynchronization; runtime limits and compile-time maxima16/64 change storage/adversarial rescan bound.'),
 ('T21.05.b','protocol durable','Queue refusal ownership plus actual retained receipt/store composition;37write/flush boundaries in two models separate held/written/durable/acknowledged state.'),
 ('T32.06.a','protocol transfer_protocol','Deterministic original-byte corpus and independent zlib oracle now includes bounded consumer-refusal decisions as fuzz input.'),
 ('T32.06.c','minimize transfer_protocol','Actual byte-deletion minimization of a compiled parser fault; changed chunk schedules and queue capacities replay the reduced/derived failures.')],
'PR-P4-01':[
 ('T27.08.b','firmware_target','Original reset/vector/data/BSS startup, actual Cortex-M4 emulator execution, two SRAM placements and crosslinked F439 DMA reachability/CCM negative control.'),
 ('T28.05.c','firmware halfduplex firmware_target','Stable byte ownership through DMA quiescence and wire TC; explicit half-duplex direction/turnaround adapter with wrap test. Electrical board behavior remains unobserved.'),
 ('T29.06.b','firmware','Release/acquire single-producer ring and source proof; explicit two-producer lost-update counterexample without executing a C data race.'),
 ('T29.06.c','firmware timing','Reduced physical capacities2..4 retain same arrival workload; explicit drop accounting and analytic blackout/backlog model. Physical latency remains pending.')],
'PR-P4-02':[
 ('T29.09.b','firmware firmware_target','Half-range timestamp comparisons and actual ARM multiword snapshot with atomic parts, torn-rollover cut and bounded retry; no volatile-only race claim.'),
 ('T30.05.b','timing','Original explicit one-interferer response recurrence and UART service/backlog derivation with ceiling/blocking/bit-rate negative controls; no unseen general theorem attribution.'),
 ('T30.05.c','firmware_target timing','Changed optimization and relocated images actually execute; physical clock/latency trace procedure authored, physical distributions not invented.'),
 ('T31.10.c','firmware','Timeout preserves first fault; failed DMA/UART quiescence acknowledgement retains buffers, generation/retry retirement and stale completion rejection.')],
'PR-P5-01':[
 ('T24.08.b','linux','Actual LT/ET epoll, fd reuse and stale generation tokens; parser-held full-queue progress resumes without requiring another edge.'),
 ('T25.02.b','async','Copied admission and mutex-protected blocking predicates; exact accepted3/5 versus refused4/6 trace and closure boundary.'),
 ('T25.02.c','async linux','Actual two pthread workers process64 frames exactly once; copied payload ownership and matching generation completion.'),
 ('T18.04.c','operation','Large128-byte actual allocated/copy-owned asynchronous payload, acquisition failure and exact release after trustworthy completion.')],
'PR-P5-02':[
 ('T24.11.b','async operation','Explicit close/drain/abort/deadline states; timeout returns pending with live ownership until later completion, not a fabricated successful join.'),
 ('T25.04.c','async','Predicate rechecks with signal-before-wait, spurious wake, competing consumers and blocked producer observing abort/closure.'),
 ('T21.06.b','operation','Actual callback from another pthread, generation match, retained cancellation context and synchronization/cleanup.'),
 ('T21.06.c','operation','Duplicate/stale/report-failure handling; callback reentry explicitly rejected while reporting and generation saturation retires state.')],
'PR-P6-01':[
 ('T17.02.b','numeric','Binary64 cancellation/order plus exact denominator4 fixed-point arithmetic, overflow bounds and ties-to-even conversion.'),
 ('T17.06.b','numeric','Specified [1,2,1]/4 FIR and actual first-order0.5 IIR with state/chunk impulse checks and stated stability proof.'),
 ('T17.06.c','numeric','Every short split, capacity/overlap refusal, exact in-place processing and every partial-failure cut under prefix versus atomic policies.'),
 ('T34.01.b','numeric numeric_experiment','Fixed rounding order/no contraction numerical policy and rejecting changed operation order for reproducible audit outputs.')],
'PR-P6-02':[
 ('T34.06.b','vector numeric_experiment','Compiler vectorization/disassembly and actual alias-permitting staging wrapper before restricted SIMD kernels.'),
 ('T34.07.b','vector numeric_experiment','CPUID/OSXSAVE/XGETBV guarded dispatch, all0..256 lengths and offsets; actual no-AVX QEMU CPU fallback, not physical-machine claim.'),
 ('T34.09.a','numeric_experiment','Actual paired timing, consumed results and native disassembly including LTO retained-work checks.'),
 ('T34.09.c','numeric_experiment','1404 raw samples across up to64MiB working sets and128MiB perturbation; operational warm/cold definitions do not prove exact cache residence.')]
}

def read(path):return json.loads((ROOT/path).read_text(encoding='utf-8'))
def sha(path):return hashlib.sha256((ROOT/path).read_bytes()).hexdigest()

def attach(record):
 """Pure additive attachment used by both canonical authoring and follow-through."""
 project=record['project']
 if project not in ROWS:return record
 record['execution_support']['observation_status']='historical-initial-authoring'
 record['execution_support']['observed_before_revision']='fd85773420a2fbb11003e83c8289417d62282ec4'
 record['execution_support']['active_successor']=REGISTER
 record['successor_support']={'path':REGISTER,'project':project,'criterion_ids':['AX-'+project+'-'+leaf for leaf,_,_ in ROWS[project]],
  'scope':'Active criterion-specific implementation/author-validation/variant readiness; the initial execution_support and reference_gaps retain their original observation scope.'}
 for contract in (LAB+'CONTRACTS.md',LAB+'transfer/CONTRACTS.md'):
  if contract not in record['shared_contracts']:record['shared_contracts'].append(contract)
 for criterion in record['criteria']:
  criterion['successor_support']={'path':REGISTER,'id':criterion['id']}
 return record

def report_status(path):
 if not (ROOT/path).is_file():return {'passed':False,'reason':'not-run'}
 value=read(path);pins=value.get('inspected_paths',value.get('source_sha256',{}))
 stale=[p for p,h in pins.items() if not (ROOT/p).is_file() or sha(p)!=h]
 if path.endswith('checkpoint_depth_campaign.json'):
  actual={p.relative_to(ROOT).as_posix() for directory in (LAB,PRIVATE) for p in (ROOT/directory).rglob('*') if p.is_file() and '__pycache__' not in p.parts}
  stale+=sorted(actual-set(pins))
 return {'passed':bool(value.get('passed')) and not stale,'stale_paths':stale,'report_sha256':sha(path)}

def main(attach_records=False):
 updates=[];pins={};all_reports={};remaining=[]
 for path in ('research/v5/expansion/legacy_completion/support.py',LAB+'CONTRACTS.md',LAB+'transfer/CONTRACTS.md',PRIVATE+'ANALYSIS.md',PRIVATE+'transfer/ANALYSIS.md','research/v5/expansion/legacy_completion/resources.json','research/v5/expansion/legacy_completion/research_gate.json'):
  pins[path]=sha(path)
 for project,cards in ROWS.items():
  path='curriculum/v5/expansion/legacy_assessments/'+project+'.json';record=read(path)
  if attach_records:
   record=attach(record);(ROOT/path).write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8',newline='\n')
  pins[path]=sha(path);criteria={c['objective']:c for c in record['criteria']}
  if set(criteria)!={c[0] for c in cards}:raise ValueError('Criterion mismatch '+project)
  for leaf,keys,scope in cards:
   criterion=criteria[leaf];implementation=[];tests=[];reports=[]
   for key in keys.split():
    item=SUPPORT[key];implementation+=item['implementation'];tests+=item['tests'];reports+=item['reports']
   reports=sorted(set(reports));implementation=sorted(set(implementation));tests=sorted(set(tests))
   for evidence in implementation+tests:
    if not (ROOT/evidence).is_file():raise ValueError('Missing evidence '+evidence)
    pins[evidence]=sha(evidence)
   for evidence in reports:all_reports[evidence]=report_status(evidence)
   ready=all(all_reports[p]['passed'] for p in reports)
   physical=project.startswith('PR-P4')
   limitations=['Author reference readiness only; no learner artifact, delayed attempt or independent reviewer is manufactured.',
                'Each test/experiment supports only the stated bounded mechanism; finite passes do not prove absolute mastery.']
   if physical:limitations+=['Real F439 electrical/peripheral/timing observations remain required where the original task asks for them; emulator/model/cross-link evidence is separate.']
   if 'timing' in keys:limitations+=['General blocking/jitter response-time paper remains a reference gap; this fixture supplies an original derivation for its stated simpler model.']
   dimensions={'teaching':{'state':'complete','evidence':[path,LAB+'CONTRACTS.md',LAB+'transfer/CONTRACTS.md']},
               'implementation':{'state':'complete-for-declared-reference-contract','evidence':implementation+tests},
               'author_validation':{'state':'passed-with-stated-scope' if ready else 'pending-current-source-validation','evidence':reports},
               'external_review':{'state':'not-run','evidence':[]},
               'independent_learner':{'state':'not-assessed','evidence':[]},
               'physical_observation':{'state':'required-not-run' if physical else 'not-applicable-to-declared-hosted-contract','evidence':[]}}
   updates.append({'id':criterion['id'],'original_source':path,'original_objective':leaf,'project':project,'dimensions':dimensions,
      'mechanism_scope':scope,'tests':tests,'variant_ids':[v['id'] for v in criterion['variants']],
      'variant_readiness':'Reference implementation/analysis and changed-condition checks provided; actual delayed learner attempt pending.',
      'closure_rationale':scope,'required_environment':record['environment']['target'],
      'external_blockers':['Unaided learner implementation and seven-day changed-condition attempt; actual non-author review.']+(['Dedicated-board physical observations.'] if physical else []),
      'evidence_limits':limitations})
   if not ready:remaining.append({'id':criterion['id'],'dimension':'author_validation','reason':'One or more evidence reports missing, failed or stale.'})
 data={'schema_version':1,'required':True,'scope':'Active successor support for49 original checkpoint criteria, preserving every original snapshot and null/nonselected domain obligation.',
       'source_sha256':pins,'report_status':all_reports,'updates':updates,'remaining_author_validation':remaining,
       'external_remaining':['Original unaided/delayed learner attempts and non-author reviews.','Physical board/electrical/timing observations where required.','General response-time theory primary passage remains unavailable; no unseen source inspection claim.'],
       'nonselected_objectives':'Continue to resolve through prior_disposition_successors.json to their original domain criteria; these49 selected interactions do not cover all parent leaves.'}
 (ROOT/REGISTER).write_text(json.dumps(data,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'criteria':len(updates),'pending_current_source_validation':len(remaining),'reports':len(all_reports)}))

if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--attach-records',action='store_true');main(parser.parse_args().attach_records)
