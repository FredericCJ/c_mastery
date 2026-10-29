"""Bind each original B01 root/rubric/delayed identity to exact authored support."""
import json
from common import ROOT,dump,read_json,sha
from check_foundation_followthrough import check,REPORT,TRANSFER,EXPORT,BASE,IDS

NOTES={
'X-T02-001':[
'Expression grouping versus sequencing has explicit sum/comma/separate-full-expression counterexamples; actual nested function-body probe admits both orders.',
'Exact mixed-sign verdict and two stated narrow-integer promotion models are derived before observations; no16-bit host execution is invented.',
'Separate/shared nested bodies execute against exact allowed result/final pairs; the unsequenced argument-expression alternative stays analysis-only.',
'The ten-case verdict analysis and shared-body versus argument-expression counterexample distinguish undefined from unspecified behavior without an execution oracle for undefined cases.',
'Original repair checks extrema and output alias refusal; actual struct fd_step return-by-value transfer removes the contradictory two-output postcondition.',
'Actual pointer-constraint translation continues with a diagnostic under an explicit severity policy and rejects under pedantic policy; no resulting object is executed.',
'Two isolated diagnostic sources and exact GCC/Clang commands distinguish required diagnostic from warnings-as-errors and wording; unfamiliar Packet argument rejects independently.',
'Exact verdict/type/lifetime dossiers and promotion models state why translation/repeatability cannot prove portable definedness; source editions and access gaps remain explicit.'],
'X-T03-001':[
'Actual nested base/subview and owner-absolute-offset enumeration preserves admitted same-array bounds; partition equivalence adds495 real-array cases per profile.',
'Null-empty, one-past, aliased metadata and unchanged refusal tests run; form-before-check failure is analyzed without executing an invalid pointer.',
'Index/remaining arithmetic avoids pointer subtraction; explicit PTRDIFF_MAX changed-model argument separates valid indices from unrepresentable pointer difference.'],
'X-T03-002':[
'Actual native and PowerPC64 big-endian live character observations record layout and representation independently of the same canonical wire.',
'The actual decoder composes extent/cursor and arithmetic signed decoding; version2 little-endian transfer and least-width range adapters execute separately.',
'Memberwise round trips pass for a deliberately symmetric wrong count pair, while fixed independent bytes reject it; canonical-byte equality premises are explicit.',
'Invalid cast is analyzed, never executed; actual bytewise decoder code/disassembly preserves the cost investigation under valid access contracts.'],
'X-T03-003':[
'Actual allocated slot phases, symbolic operation strings, typed storage and explicit wrapper move preserve distinct allocation/construction/publication/cleanup authority.',
'Actual typed-store and character-array-copy constructors publish equal members from genuine allocations; declared-struct fd_value transfer separately revises the effective-type premise.',
'Original allocation failures and3125 operation strings exercise state/resource transitions; two additional compiling controls discriminate premature publication and full destructor on failed initialization.',
'Exact allocated versus declared storage reasoning and copied-owner counterexample distinguish C discipline from language-enforced move/destruction; A/B rollback and source revocation execute.'],
'X-T03-004':[
'Object/access/block restriction analysis distinguishes equal bases with no access from positive overlap; prohibited overlapping restrict calls are not executed.',
'Disjoint XOR and separate snapshot movement execute; actual single-pointer in-place transfer revises the contract explicitly.',
'Actual GCC/Clang O0/O2/O3 paired separate-translation-unit comparison records20samples/profile, assembly and size with identical correctness and no speedup threshold.',
'Snapshot oracle and compiled forward-only control remain supplied; actual strided transfer preserves gaps and compaction advances/refuses finite invalidation epoch.'],
'X-T03-005':[
'Independent symbolic issue-count model runs32769 histories/profile and explicit finite retirement; unchanged-address destroyed-owner scenario remains analysis-only.',
'Actual external registry checks epoch before local handle lookup and relocates under exclusive lifetime premises; three distinct test owners now each initialize once.',
'Finite issue/retirement history and compiled wrapping control reject identity revival; withheld reset history supplies exact epoch1 resurrection and maintenance/quiescence premises.',
'Actual copied-value and borrowed const lookup tests discard the borrow before release/stale lookup; registry relocation explicitly excludes surviving native borrows.'],
'X-T04-001':[
'Actual signed-count admission and negative/extreme checks accompany rank-independent uintmax_t comparison proof; wider-source/size_t reasoning does not claim target execution.',
'Unbounded integer oracle and independently compiled premature-product control expose defined unsigned arithmetic violating the application extent predicate.',
'Actual two-endpoint interval constructor preserves both fields;65536 tuples, extrema and early-publication/exclusive-bound controls complement signed two-segment layout and inclusive-limit analysis.',
'Actual C17 builtin and separately labeled C23-draft checked-macro adapters use temporary outputs;65542cases and primitive output-on-overflow checks establish bounded equivalent contracts.'],
'X-T35-001':[
'Original counter asset, byte/length trust boundary and separate authorization are explicit; actual delayed staging captures first-fragment authority under the same local threat model.',
'Independent2474-case oracle distinguishes malformed/truncated versus denied requests; timeout/cancel reasoning avoids assuming malicious origin.',
'Actual compiling premature-publication control returns normally with an independent expected-state mismatch; cancellation/reset has a separate stale-authority control and no escalation claim.',
'Actual extent/cursor call chain and unbounded count/value oracle are distinct from the concrete changed variable-payload analysis with checked header/count/record budget.',
'Exact wire syntax, cross-field arithmetic and single-commit behavior execute; all128 delayed fragment chunkings and over-capacity/trailing policy have current successor evidence.',
'All four wire flags remain syntax only; actual wire-authority mutant and trusted-authority oracle fail as intended, while QUERY has deliberately distinct admission.',
'Fixed core retains no partial input; delayed frame owns at most8bytes, rejects over-capacity transactionally and cancels staged authority; broader variable-size work has an explicit separate analysis.']}

CASES={'X-T02-001':'semantic','X-T03-001':'cursor','X-T03-002':'codec','X-T03-003':'lifecycle','X-T03-004':'movement','X-T03-005':'registry','X-T04-001':'layout','X-T35-001':'frame'}
DELAYED={
'X-T02-001':'struct fd_step and unfamiliar-header/constraint/runtime-precondition dossier, including two integer models and explicit sequencing counterexamples.',
'X-T03-001':'Two independent arrays with transactional segmented copy-out plus owner-relative views and rollback consumer.',
'X-T03-002':'Version2 reserved-zero little-endian wire, least-width storage checks and memberwise equality.',
'X-T03-003':'Actual declared-struct fd_value reservation, A/B partial-resource rollback and explicit move/source invalidation.',
'X-T03-004':'Positive-stride snapshot move, unchanged gap bytes and finite compaction epoch invalidation.',
'X-T03-005':'Finite external registry, exclusive relocation and concrete reset-revival history; no reset is silently implemented.',
'X-T04-001':'Signed-length two-segment layout with combined budget and unchanged three-field refusal; separate half-open interval now implemented.',
'X-T35-001':'Trusted QUERY plus eight-byte fragment staging, all128chunkings, cancellation/reset and distinct authority transitions.'}

def main():
    reports=[REPORT,TRANSFER,EXPORT];d,t,e=[read_json(ROOT/p) for p in reports];check(d,t,e)
    rows=[];updates=[];inputs={**d['input_sha256'],**t['inspected_paths'],**e['input_sha256']}
    proof='assessor/foundation_followthrough/REASONING.md';shared='labs/foundation_followthrough/CONTRACTS.md';ready=BASE+'/readiness.json'
    for id in IDS:
        brief=f'curriculum/v5/exercises/{id}.md';text=(ROOT/brief).read_text(encoding='utf8');spec=json.loads(text.split('```json\n',1)[1].split('\n```',1)[0]);assert len(spec['rubric'])==len(NOTES[id])
        teaching=[brief,shared,proof,f'assessor/foundation_depth/analyses/{id}.md','assessor/foundation_depth/transfer/ASSIGNMENTS.md','assessor/foundation_depth/transfer/ANALYSIS.md','assessor/foundation_depth/transfer/SEMANTIC_CASES.md']
        implementation=['assessor/foundation_depth/depth.c','assessor/foundation_depth/transfer/reference.c','assessor/foundation_depth/transfer/tests.c','labs/foundation_depth/tests/tests.c','assessor/foundation_followthrough/follow.c','labs/foundation_followthrough/tests.c',proof]
        claims={id:'Original root and all listed ordinary/changed-condition criteria have concrete code or explicit semantic analysis as appropriate. '+DELAYED[id],id+'-DELAYED':DELAYED[id]+' Current author transfer case '+CASES[id]+' executed; release after seven days and unaided learner performance remain unobserved.'}
        claims.update({r['id']:note for r,note in zip(spec['rubric'],NOTES[id])})
        for stable_id,claim in claims.items():
            dims=dict(teaching=dict(state='complete',evidence=teaching),implementation=dict(state='complete',evidence=implementation),author_validation=dict(state='passed',evidence=reports+[ready]),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-run',evidence=[shared]))
            updates.append(dict(id=stable_id,dimensions=dims,closure_rationale=claim+' Bounded author support, not an assessment score or universal proof.'))
            rows.append(dict(id=stable_id,parent=id,author_readiness='ready-bounded',demonstration=claim,transfer_case=CASES[id],teaching=teaching,implementation=implementation,validation=reports,external_review='not-run',independent_learner='not-assessed',physical_observation='not-run'))
        for p in teaching+implementation:inputs[p]=sha(ROOT/p)
    for p in reports+[BASE+'/resources.json',BASE+'/research_gate.json','tools/check_foundation_followthrough.py','tools/register_foundation_followthrough.py','tools/build_foundation_followthrough_readiness.py','tools/check_expansion_campaigns.py','tools/check_replicated_replay_depth.py']:inputs[p]=sha(ROOT/p)
    dump(ROOT/ready,dict(schema_version=1,criteria=rows,input_sha256=inputs,current_reports={p:sha(ROOT/p) for p in reports},limits=['Exactly8original roots,38rubric criteria and8delayed variants; no whole-leaf or whole-edition completion inference.','Native/PPC64 executions are recorded; M4 is link-only and physical hardware is unobserved.','N1570/N3096 remain drafts; exact published C17/C23 access remains open.','Author solutions, finite controls and source review do not establish human independence or learner mastery.']))
    inputs[ready]=sha(ROOT/ready)
    dump(ROOT/BASE/'completion_dispositions.json',dict(schema_version=1,updates=updates,current_reports=reports,input_sha256=inputs))
    print(dict(passed=True,updates=len(updates)))
if __name__=='__main__':main()
