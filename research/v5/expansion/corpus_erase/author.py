"""Add the exact owning erase/remove mechanism without changing accepted leaves."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def write(name,value):
 p=ROOT/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf8')
ident='X-T14-017'
sources=[('R-CER-REMOVE','C++ working draft: remove algorithms','https://eel.is/c++draft/alg.remove',['[alg.remove] paragraphs1-7: predicate, move assignment, logical end, stable prefix, count and unspecified tail']),('R-CER-VECTOR','C++ working draft: vector modifiers','https://eel.is/c++draft/vector.modifiers',['[vector.modifiers] paragraphs4-6: erase invalidation, exceptions and destructor/assignment counts'])]
resources=[]
for id,title,url,locators in sources:
 resources.append(dict(id=id,title=title,authors=['ISO/IEC JTC1/SC22/WG21 contributors'],edition='Current working-draft HTML rendering inspected2026-10-18, not published C++17 text',year=2026,kind='standard-draft',url=url,access='public',domains=['T14'],role='reference',reason='Distinguish logical compaction, moved-from tail, physical erase and resource lifetime before constructing a stronger bounded C contract.',prerequisites=['Typed arrays','Ownership and callback effects'],locators=locators,assignable=True,inspection=dict(status='passage-inspected',date='2026-10-18',method='Direct web read of named rendered primary draft paragraphs',scope='; '.join(locators),limits='Only selected current-draft paragraphs inspected. Live revision and actual C++17 compiler experiment are separate; no published-edition identity or full standard inspection claimed.')))
write('research/v5/expansion/corpus_erase/resources.json',dict(resources=resources))
criteria=[('T14.03.b','Derive and implement the logical-end/size distinction, stable retained prefix and exact assignment cost; explain index/reference meaning across compaction and erase without claiming storage shrink.'),('T14.03.c','Implement fallible owning filtering with unchanged owner/output on callback failure, explicit callback effects and exactly-once resource release; compare the stronger staged contract with actual C++ moving/destruction.'),('T14.15.b','Verify every ownership/invalidation transition, including aliased predicate context, all-kept/all-removed boundaries, stale tokens, pending clear and generation retirement, using an independent model and causal mutants.')]
rubric=[];coverage=[]
for n,(objective,criterion) in enumerate(criteria,1):
 rid=ident+'-R'+str(n);rubric.append(dict(id=rid,objective=objective,criterion=criterion,levels={'0':'Conflates logical range, size or ownership, or supplies no mechanism.','1':'Ordinary scalar example works but failure/lifetime argument is missing.','2':criterion+' Supply concrete predictions, implementation and positive/negative evidence.','3':'Independently adapt the named changed contract, explain a minimized counterexample and defend the scope of each guarantee.'}));coverage.append(dict(objective=objective,teaching=['#problem','#necessity','#reasoning'],implementation=['D1','D2'],boundary=dict(applicable=True,evidence=['D3']),independent_assessment=[rid],scope='Specific mutation/invalidation/failure mechanism; other original leaf obligations remain separate.'))
public=['labs/corpus_erase/'+n for n in ['erase.h','tests.c','driver.c','comparison.cpp','CMakeLists.txt','reject.py']]
meta=dict(schema_version=1,id=ident,title='Stable compaction, owning tails and physical erase',project='PR-T14-02',domain='T14',objectives=[x[0] for x in criteria],families=['A','B','C','D','E','F','G','H','I','J'],required=True,prerequisite_exercises=[],prerequisite_artifacts=[],profile='C',environment=dict(language='C17 component; separate hosted C++17 comparison',target='Original typed twelve-item owner and hosted test process',toolchain=['GCC','Clang','CMake','C++17 compiler','Python3'],dependencies='No container/filter/allocator/external memory primitives in C component; permitted compiler helpers audited; C++ library and Python only independent host comparisons'),source_records=[dict(id=r['id'],path='research/v5/expansion/corpus_erase/resources.json',passages=r['locators']) for r in resources],lab=dict(family='corpus_erase',scaffold='labs/corpus_erase',tests=['corpus_erase.compact']),assessor_analysis='assessor/corpus_erase/ANALYSIS.md',status=dict(content='authored',execution='see-source-bound-campaign',independent_review='not-run',limitations=['Bounded serialized owner, no generic C move/destructor enforcement.','Selected current C++ draft paragraphs and separately recorded C++17 implementation observations.','No strict final-image or physical claim.'],campaign_receipt='reviews/v5/expansion/corpus_erase/corpus_erase_campaign.json',learner_assessment='not-assessed'),fixtures=public+['fixtures/corpus_erase/cases.json'],shared_contracts=['labs/corpus_erase/CONTRACTS.md'],delayed_transfer=dict(minimum_days=7,variant_ids=[ident+'-DELAYED']),objective_coverage=coverage,rubric=rubric)
body='''# X-T14-017: Stable compaction, owning tails and physical erase

## Prerequisites

Know array bounds, typed object lifetime, value versus reference arguments, callback contracts and resource ownership. Bring an ordinary vector mutation/invalidation trace from PR-T14-02 and a simple sequence model; no allocator or library filter may implement this mechanism. This is an additional required contract, not a claim that the retained vector was absent.

## Environment

The explicit [contract1](../../../labs/corpus_erase/CONTRACTS.md) supplies representations, all operations/statuses, capacities, pointer premises, callback effects, resource costs and containment. Implement `src/erase.c`. The C core is Profile C. Hosted C++17 and Python are separate comparison/oracle dependencies. Record actual C and C++ compiler selections independently and do not infer strict freestanding closure from a host build.

## Problem

First compact `[1,2,1,3]` by value1, returning a stable prefix, logical end and assignment count without changing array extent. Then construct IDs1..4 with values1,2,1,3, evaluate a predicate borrowing the first original value, and produce the retained IDs2,4 while retaining removed rights1,3 in a pending tail. Physical erase must drop exactly1,3; clear must later drop exactly2,4. Fail each predicate call, reuse an old tail, clear a pending tail and exhaust epoch headroom. Implement every operation in the shared API; provide the exact state/output/drop ledger at each step.

## Necessity

A correct-looking retained prefix can hide leaked or duplicated rights. Logical removal does not shrink a container; an unspecified tail is not a reliable list of removed original objects. Moving an owning C++ value may destroy an overwritten resource before physical erase. Aliasing the comparison value can change later decisions. These are distinct mechanisms with separate counterexamples.

## Reasoning

Derive the read/write frontier invariant for scalar stable compaction. For the owning adaptation, prove that every original ID appears exactly once across retained and removed partitions, decisions finish before publication, and failure preserves owner/output while callback observations remain. State all preconditions, postconditions, ownership transfers and complexity bounds. Explain generation headroom and why public token fields do not provide unforgeable capabilities or protection after owner lifetime. Compare ISO C guarantees, diagnostics, runtime checks, external models and caller discipline with the inspected C++ contracts; include an incorrect claim and a minimal counterexample for each rubric criterion.

## Deliverables

- D1: Logical-end/size/lifetime derivation, invalidation table, actual C++ observations and bounded source-claim comparison.
- D2: Original C implementation, callback/ownership ledger and precise stack, copy, comparison and destruction costs.
- D3: Independent sequence/right model, all removal masks and failure positions through length8, capacity12 checks, stale/clear/exhaustion histories and two causal semantic mutants.
- D4: Named delayed owning/fallible adaptation with predictions made before execution, changed-environment obligations and limits.

## Verification

For `[1,2,1,3]`, require prefix `[2,3]`, end2 and two scalar assignments. On owning success require original count until erase, zero early drops, stable retained IDs and one final drop per original ID after cleanup. On every failed predicate require unchanged owner/output and exactly the attempted callback count. Compare4097 finite sequence/failure cases to the independent model, then run the specified larger boundaries. Execute C++ comparison separately and retain the observed pre-erase destruction mask without asserting universal timing. Run six C compiler/optimization/sanitizer profiles and both original harmless mutants. The compiling learner stub must fail `corpus_erase.compact` at `ER_ASSERT:`. Passing tests alone do not satisfy reasoning or independent transfer.

## Rubric

Apply each JSON criterion separately. Level2 requires the exact mechanism and ordinary/failure evidence. Level3 requires independent prediction and changed-condition implementation, including the limits of the C/C++ comparison. A finite passing suite does not prove absolute mastery or satisfy a delayed learner assessment.

## Hints

1. Draw the retained prefix, unread suffix and physical allocation as separate objects in your explanation.
2. A fallible predicate must finish before you overwrite the object it may inspect through its context.
3. Count rights, not copies of ID bits. Compare when an owning move assignment can release its destination with this exercise's deliberately delayed drop policy.

## Variants

X-T14-017-DELAYED: after at least seven days, change scalar filtering to the staged owning contract with a fallible predicate whose context borrows the first original value. Predict each failure cut, then implement pending-tail cancellation, stale-token refusal and terminal epoch cleanup. Compare with actual C++ owning moves and quantify the extra staging needed for the stronger unchanged-owner guarantee. Repeat under a changed compiler/optimization and revise only implementation-dependent observations.

## Security

The target is the original local teaching owner and synthetic drop ledger. Attacker-controlled valid removal masks, callback failures and stale operation order can expose wrong-resource release or retained-order corruption. Success means an incorrect synthetic ID drop or order result, not arbitrary code execution. Reproduce, repair and retain both distinct regressions; state that forged owner memory and dangling pointers violate the API premises and are not made safe by scalar token checks. No third-party data or external service is involved.

## Validation

Use the named source-bound campaign and isolated export receipts for author execution. The six-profile C results, separately selected C++ toolchain, independent finite model, external review, learner delay and physical evidence are distinct dimensions. Only the recorded primary draft passages were inspected; no entire standard or whole-container objective is claimed from this one exercise.
'''
(ROOT/'curriculum/v5/exercises'/f'{ident}.md').write_text('```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body,encoding='utf8')
write('labs/corpus_erase/family.json',dict(version=1,family='corpus_erase',title='Logical compaction and owning physical erase',build_system='cmake',assessor_root='assessor/corpus_erase',projects=[],units=[],modules=['T14'],dependencies=[],tests=['corpus_erase.compact'],exercises=[dict(id=ident,project='PR-T14-02',tests=['corpus_erase.compact'],capabilities=['linux-c17'],scope='Stable prefix, two-stage owner and fallible changed contract')],contract='labs/corpus_erase/CONTRACTS.md',fixture='fixtures/corpus_erase/cases.json',campaign=dict(producer='tools/run_corpus_erase.py',profiles=[c+'-'+p for c in ['gcc','clang'] for p in ['debug','release','asan']],assertion_marker='ER_ASSERT:',mutants_per_anchor=2,fixture_oracle='assessor/corpus_erase/oracle.py',oracle_driver='erase_driver')))
write('fixtures/corpus_erase/cases.json',dict(schema_version=1,synthetic=True,scalar=dict(input=[1,2,1,3],remove=1,prefix=[2,3],logical_end=2,assignments=2,physical_extent=4),owning=dict(ids=[1,2,3,4],values=[1,2,1,3],retained=[2,4],removed=[1,3],drops_before_erase=[],drops_after_erase=[1,3],drops_after_clear=[1,2,3,4]),scope='Specified finite expected values, not execution evidence.'))
write('research/v5/expansion/corpus_erase/taxonomy_refinement.json',dict(schema_version=1,subtopics=['Stable logical compaction versus physical erase','Owning tail, fallible predicate publication and callback lifetime','Aliased comparison value and move/destructor comparison'],exercise=ident,objectives=[x[0] for x in criteria],rationale='The finite84 erase-remove description identifies a mechanism missing from retained vector and stable-removal implementations: explicit logical-end/physical-erase separation with owning moved values. Refine existing mutation/invalidation/failure objectives; preserve accepted leaf wording, IDs and historical hours.'))
print('PASS erase canonical authoring')
