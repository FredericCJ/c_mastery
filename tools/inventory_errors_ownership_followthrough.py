"""Keep original T65/T66 author obligations explicit while bounded packets land."""
import json
from common import ROOT,dump,sha
TASKS={
'T65-001':'Full tagged option, zero-ID owning payload through actual T66 allocator, changed data model and C08 compiler comparison.',
'T65-002':'Actual explicit/goto and statement-macro parser forms, single evaluation/name collision, incremental signed first field and absolute consumed position.',
'T65-003':'Actual io.stream composition with copied staging, cancellation after short write and separate accepted/durable epoch authority.',
'T65-004':'Owned-result map/destruction and bounded nested cause/offset conversion; pure laws and eager-unused callback counterexample.',
'T65-005':'Dependent quota/denominator validation, bounded nested error paths and truncation without partial publication.',
'T65-006':'Actual third-resource acquisition failure and T66 scope composition, two nested jump frames and inner-error conversion.',
'T65-007':'Timer queue half-window ambiguity and explicit unbounded-epoch or finite nonwrapping replacement with counter-width reasoning.',
'T65-008':'Bounds-preserving core in every reporting mode, concurrent report ownership and actual assert/NDEBUG comparison.',
'T65-009':'Checked rational addition with cross-cancellation, signed-minimum reasoning and validated consumer boundary.',
'T66-001':'Partial-initialization DAG A before B/C, D after B, with C failure after acquiring storage and exact initialized-set cleanup.',
'T66-002':'Actual scope/nonlocal-jump composition and a fallible cleanup result retaining a recovery owner.',
'T66-003':'Self-referential or inline-owned representation move and opaque generation-token duplicate-handle boundary.',
'T66-004':'Strided views with exact address sets and padded two-dimensional tiles rather than interval-only disjointness.',
'T66-005':'Reentrant callback rejection or snapshot discipline and actual multithreaded borrow guard.',
'T66-006':'Strong-cycle leak/weak-edge repair and synchronized last-strong-drop versus weak-upgrade race.',
'T66-007':'Copy-on-write of owned resources with partial clone failure and concurrent snapshot reclamation.',
'T66-008':'Completion on another executor, timeout retaining DMA ownership and explicit acknowledgement/quiescence authority.',
'T66-009':'Upstream allocator resize accounting and hidden recursive-formatting allocation observed through actual consumers.',
'T66-010':'Serialized/foreign-boundary revalidation and read/write capability attenuation with an overstated-safety counterexample.',
'T66-011':'Reuse existing actual five-resource fallible-close recovery support; bind every original rubric to current evidence rather than infer from family pass.'}

def main():
    rows=[];pins={}
    for key,task in TASKS.items():
        domain,number=key.split('-');identity=f'X-{domain}-{int(number):03d}';path=f'curriculum/v5/exercises/{identity}.md';body=(ROOT/path).read_text(encoding='utf8');m=json.loads(body.split('```json\n',1)[1].split('\n```',1)[0]);pins[path]=sha(ROOT/path)
        packet={'T65-001':'errors_owning_option','T65-002':'errors_propagation','T65-003':'errors_progress','T65-004':'errors_composition','T65-005':'errors_validation','T65-006':'errors_jump_scope','T65-007':'errors_serial_timer','T65-008':'errors_contract_modes','T65-009':'errors_rational','T66-001':'ownership_frontier','T66-002':'ownership_scope','T66-003':'ownership_move','T66-004':'ownership_strides','T66-005':'ownership_cell_guard','T66-006':'ownership_references','T66-007':'ownership_cow_resources','T66-008':'ownership_pin_async','T66-009':'ownership_allocator_audit','T66-010':'ownership_authority_boundary','T66-011':'ownership_publication'}.get(key)
        state='author-support-implemented-and-source-validated' if packet else 'required-author-followthrough-pending'
        evidence=[f'research/v5/modern/{packet}_readiness.json'] if packet else []
        ids=[identity]+[r['id'] for r in m['rubric']]+m['delayed_transfer']['variant_ids']
        rows.append(dict(exercise=identity,original_ids=ids,objectives=m['objectives'],required=True,status=state,author_obligation=task,evidence=evidence,independent_learner='not-assessed',external_review='not-run',physical_observation='separate-target-specific-obligation',baseline_scope='Original exercise/rubric and actual errors/ownership contracts inspected; retained simple implementation is not evidence that its changed-condition variant exists.'))
    dump(ROOT/'research/v5/modern/errors_ownership_remaining.json',dict(schema_version=1,scope='All original T65/T66 roots, rubrics and delayed variants; no time ceiling',comprehensive_complete=False,rows=rows,input_sha256=pins,dependency_order=['Early T65 scalar/error primitives','T66 ownership and actual counting allocator','Return to T65 owning payload/cleanup consumers without reversing early prerequisite edges'],limits=['This inventory does not adjudicate human-independent learner or physical evidence.','Named unresolved tasks remain required; current passing packets close only their exact bounded author support.','Exact published C17/C23 source access remains open.']));print(json.dumps(dict(exercises=len(rows),original_ids=sum(len(r['original_ids']) for r in rows),ready_packets=sum(r['status']=='author-support-implemented-and-source-validated' for r in rows))))
if __name__=='__main__':main()
