"""Close only original author-side service obligations against current evidence."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[4];sys.path.insert(0,str(ROOT/'tools'))
from common import read_json,dump,sha,parse
from check_service_adapters_v5 import validate
from check_expansion_campaigns import pin_errors,execution_errors,special_errors
campaign='reviews/v5/expansion/followthrough/service_adapter_campaign.json'
report=read_json(ROOT/campaign);errors=validate(report)
core='reviews/v5/expansion/integrated/service_depth_campaign.json'
data=read_json(ROOT/core)
errors+=pin_errors(data)+execution_errors(data)+special_errors('service_depth',read_json(ROOT/'labs/service_depth/family.json'),data)
if not data.get('passed') or data.get('adapter_campaign',{}).get('sha256')!=sha(ROOT/campaign):errors.append('Core/successor binding missing')
cost='reviews/v5/expansion/followthrough/service_image_costs.json'
costs=read_json(ROOT/cost);errors+=pin_errors(costs)+execution_errors(costs)
if not costs.get('passed') or len(costs.get('images',[]))!=8:errors.append('Image cost scope incomplete')
if errors:raise SystemExit(str(errors))
rows=[];gaps=[];readiness=[]
observations={
 2:[('memory allocation fail0..2, full16-frame ownership and saturated-reference rejection; guest allocation rollback at every actual segment/stack acquisition; LOAD_LEAK control', 'Static page tables are not dynamically allocated; acquired data/stack frames are.'),
    ('memory child copy, failed-copy unchanged state, parent isolation and sole-owner no-copy; guest child write and held-child two-CPU faults', 'Finite guest interleavings do not prove every possible schedule.'),
    ('memory mapping-kind/thread-count preflight; guest multithreaded and shared-writable clone rejection', 'This restricted clone policy is additional to the retained broader CMK2 clone.'),
    ('memory denied copy leaves source frame and refs intact; actual guest acknowledged invalidation and remote reaping; immutable-write rejection', 'Copy-allocation exhaustion is injected in the portable core; guest concurrent COW exercises the actual retained allocator/locking path.')],
 3:[('channel close/send denial/drain-once/reopen stale epoch; guest cancellation, destroyed capability and old-incarnation wake rejection', 'Queue capabilities in this adapter are granted within one owning process.'),
    ('channel full-span validator failure leaves head/output/count untouched; guest cross-boundary copy rejection and exact partial suffix', 'The guest has no user unmap syscall; RETRY retains no blocked output pointer.'),
    ('channel send-only receive denial, amplification denial, full admission and UINT32_MAX retirement; guest RIGHTS control', 'Portable epoch exhaustion is deliberate boundary injection, not billions of guest reuses.')],
 4:[('files separate-open offsets, dup shared offset and final-reference close; actual guest cloned process descriptions and FILE_FORK control', 'Archive backing has guest lifetime; mutable versions have explicit root/snapshot references.'),
    ('files signed seek/EOF/null-zero and rejected-read sentinel; actual guest INT64_MAX/negative seek and cross-page output rejection; OFFSET control', 'Successful reads admit at most256 bytes per syscall; no unrestricted POSIX file contract.'),
    ('files duplicate/invalid namespace rejection, stale closes, exhausted handles and UINT32_MAX generation; guest imports retained checked CMKA parser', 'The core uses a validated SDEntry namespace; the adapter uses actual CMKA bytes and the retained parser.')],
 5:[('journal lost-reply retry/conflicting payload and alternating slots; actual receipt/store commits, REAPPLY/PAYLOAD/SPLIT controls, VM restart', 'Exactly-once effect is conditional on the declared transaction/durability/admission model.'),
    ('journal four cut outputs remain sentinel;185 media cuts and18176 sector cases; each actual VM write/flush/reply cut', 'A reset or failed flush is uncertain; host cached I/O is not physical power-loss evidence.'),
    ('independent old/new0-or7 recovery oracle,75 guest interruption sites with reboot/retry; bounded multi-client retention and CRC counterexample analysis', 'Enumeration is not all joint corruption states; an unkeyed checksum cannot authenticate arbitrary malicious replacements.')]
}
parents=[('SD-KERNEL-LOAD-COW',2,'load.inc','Transactional actual ELF acquisition, private/immutable clone admission, actual PTE/fault/remote-quiescence behavior.'),
 ('SD-KERNEL-CHANNEL',3,'services.inc','Protected directional tokens, stable copyout, copied blocked sends, cancellation, partial offsets and incarnation-aware teardown/reuse.'),
 ('SD-KERNEL-FILES',4,'services.inc','Actual checked archive import, independent/shared descriptions, process inheritance and snapshot/live version ownership.'),
 ('SD-DISK-RECEIPT',5,'receipt.c','Actual store transaction atomically binds effect, request payload and bounded per-client receipt metadata; complete VM restart and declared sector-tear models.')]
for ident,number,impl,reason in parents:
    exercise=f'X-T56-{number:03}';brief=f'curriculum/v5/exercises/{exercise}.md';meta,_=parse(ROOT/brief)
    teaching=[brief,'labs/service_depth/adapters/CONTRACTS.md','assessor/service_depth/kernel/ANALYSIS.md']
    implementation=['assessor/service_depth/kernel/'+impl,'labs/service_depth/adapters/guest.py','labs/service_depth/adapters/kernel_abi.h']
    implementation += ['labs/service_depth/adapters/receipt.c','labs/service_depth/adapters/tests/receipt_user.c'] if number==5 else ['labs/service_depth/adapters/load.inc','labs/service_depth/adapters/services.inc','labs/service_depth/adapters/tests/user.c']
    evidence=[core,campaign,'reviews/v5/expansion/followthrough/service_adapter_check.json',cost,'research/v5/expansion/followthrough/service_image_analysis.md']
    def update(key,scope):
        return {'id':key,'dimensions':{
          'teaching':{'state':'complete','evidence':teaching},
          'implementation':{'state':'complete','evidence':implementation},
          'author_validation':{'state':'passed','evidence':evidence},
          'external_review':{'state':'not-run','evidence':[]},
          'independent_learner':{'state':'not-assessed','evidence':[]},
          'physical_observation':{'state':'not-run; not required to establish the stated QEMU/host-model observation','evidence':[]}},
          'closure_rationale':scope+' Original portable core remains separately checked. This closes author support/execution only; the seven-day learner attempt and external review remain pending.',
          'required_environment':'C17 GCC15/Clang21 hosted Linux; x86-64 QEMU TCG2/4CPUs, modern virtio, native Linux synthetic image; Cortex-M4 cross-link is inspection only.',
          'validation_method':'Actual imported reference images, exact positive/semantic-negative markers and exits, full object/image closure, all declared reboot boundaries and independent receipt outcomes.',
          'evidence_limits':['Not all joint media-corruption states, physical power loss, latency bounds, independent reviewer evidence or learner mastery.']}
    rows.append(update(ident,reason))
    rows.append(update(exercise,'Complete author-facing support for each individually mapped criterion below. '+reason))
    for criterion,(observation,limit) in zip(meta['rubric'],observations[number],strict=True):
        row=update(criterion['id'],criterion['criterion']+' Evidence: '+observation)
        row['evidence_limits'].append(limit);rows.append(row)
        readiness.append({'id':criterion['id'],'objective':criterion['objective'],'exercise':exercise,
          'criterion':criterion['criterion'],'teaching':teaching,'implementation':implementation,
          'verification_procedure':observation,'observations':evidence,'limit':limit,
          'learner_score':'not-assessed; author-reference readiness never assigns rubric level3'})
    variants=[]
    for local in ['V1','V2']:
        key=exercise+'-'+local
        variants.append({'id':key,'original_local_id':local,'minimum_days':7,'exercise':exercise,
          'requirement':'Complete the original '+local+' changed-condition mechanism and analysis in '+brief+'#variants; assess its learner independently after the delay.',
          'status':'author-implemented-validated; independent-delayed-evidence-pending'})
        rows.append(update(key,'Concrete implementation and assessor package for the original '+exercise+' '+local+'. '+reason))
    gaps.append({'id':ident,'originating_record':'research/v5/expansion/service_depth/remaining.json',
      'exercise':exercise,'objectives':meta['objectives'],'status':'author-implemented-validated; external-evidence-separate',
      'requirement':reason,'variants':variants,'dispositions':'research/v5/expansion/followthrough/service_adapter_dispositions.json',
      'evidence':evidence,'historical_limit':'The original remaining.json and original research gate remain unchanged snapshots; this additive successor supplies new evidence.'})
folder=Path(__file__).parent
dump(folder/'service_adapter_dispositions.json',{'schema_version':1,'updates':rows})
dump(folder/'service_adapter_gap_ledger.json',{'schema_version':1,'gaps':gaps})
support=set(p for row in readiness for key in ['teaching','implementation','observations'] for p in row[key])
support.add('labs/service_depth/tests/tests.c')
dump(folder/'service_objective_readiness.json',{'schema_version':1,'criteria':readiness,
 'input_sha256':{p:sha(ROOT/p) for p in sorted(support)}})
target=ROOT/'curriculum/v5/expansion/implementation_updates.json';state=read_json(target);index={r['id']:r for r in state['updates']}
for row in rows:index[row['id']]=row
state['updates']=list(index.values());dump(target,state)
print({'closed_author_obligations':len(rows),'physical_or_independent_evidence_claimed':False})
