"""Finite84 actual-stack refinement; does not change accepted identities/budgets."""
import json,hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
BASE='research/v5/expansion/corpus_treiber'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def dump(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def main():
 resources=[]
 for ident,title,url,authors,edition,year,locators,payload in [
  ('R-CT-STACK','Generality and Speed in Dual Containers: author pseudocode','https://www.cs.rochester.edu/research/synchronization/pseudocode/generic_duals.html',['Joe Izraelevitz','Michael L. Scott'],'TOPC2016 author-hosted pseudocode',2016,['Peekable Treiber stack: Node/top, complete push and pop CAS loops'],'build/corpus_treiber_sources/duals.html'),
  ('R-CT-HAZARD','Hazard Pointers: Safe Memory Reclamation for Lock-Free Objects','https://www.cs.otago.ac.nz/cosc440/readings/hazard-pointers.pdf',['Maged Michael'],'IEEE TPDS15(6)2004',2004,['Section2.3 ABA and live-node reinsertion, printed493','Sections3.1-3.4 scan, continuous protection and safety argument, printed493-496'],'build/advanced-reclamation-pilot/tmp/pdfs/hazard-pointers.pdf')]:
  resources.append(dict(id=ident,title=title,url=url,authors=authors,edition=edition,year=year,kind='author-source' if ident.endswith('STACK') else 'paper',access='Public primary passages inspected',domains=['T55'],role='primary',reason='Actual single-head stack mechanism and lifetime/ABA assumptions',prerequisites=['C atomics','Pool ownership','Hazard publication'],assignable=True,locators=locators,inspection=dict(date='2026-10-18',method='Actual author HTML read; retained PDF text and rendered PDFpage4 figure3 inspected',scope='; '.join(locators),limits='Selected passages only; pseudocode is not C memory-model proof; no original IBM1986 inspection',private_payload=payload,sha256=sha(ROOT/payload))))
 dump(ROOT/BASE/'resources.json',dict(resources=resources))
 meta=json.loads((ROOT/'curriculum/v5/exercises/X-T15-019.md').read_text(encoding='utf8').split('```json\n')[1].split('\n```')[0])
 ident='X-T55-004';objectives=['T55.05.a','T55.05.b','T55.05.c']
 claims=[('T55.05.a','Implement actual single-head Treiber push/pop with protected revalidation, immutable published links and precise LIFO linearization.'),('T55.05.b','Bind reclamation to the exact retired generation and diagnose a collector delayed across reuse and a new protected incarnation.'),('T55.05.c','Implement finite pool refusal, stalled-reader cancellation and generation exhaustion without reclaiming a live borrow; distinguish progress classes.'),('T55.05.a','Falsify publication and validation omissions using the actual C implementation, independent real-time stack histories and precise target/resource limits.')]
 meta.update(id=ident,title='Treiber push/pop, protected lifetime and bounded reuse',project='PR-T55-05',domain='T55',objectives=objectives,prerequisite_exercises=['X-T55-002'],prerequisite_artifacts=[],profile='C',lab=dict(family='corpus_treiber',scaffold='labs/corpus_treiber',tests=['corpus_treiber.'+s for s in ['stack','reclaim','retirement']]),fixtures=['fixtures/corpus_treiber/cases.json'],shared_contracts=['labs/corpus_treiber/CONTRACTS.md'],assessor_analysis='assessor/corpus_treiber/ANALYSIS.md')
 meta['environment']=dict(language='ISO C17 baseline; C11 draft source comparison accurately labeled',target='Linux native pthreads and finite GenMC RC11; strict M4 object/link audit separately',toolchain=['GCC','Clang','CMake','Python3','GenMC0.19.0 LLVM21.1.8','Arm GCC'],dependencies='Core uses C atomics and caller storage only; no allocator/container/lock/thread library. Pthreads/GenMC/Python are host verification tools.')
 meta['source_records']=[dict(id=r['id'],path=BASE+'/resources.json',passages=r['locators']) for r in resources]
 reused='research/v5/expansion/legacy_advanced_reclamation/resources.json'
 r=next(r for r in json.loads((ROOT/reused).read_text())['resources'] if r['id']=='R-AP-N1570')
 meta['source_records'].append(dict(id=r['id'],path=reused,passages=r['locators']))
 meta['rubric']=[dict(id=ident+f'-R{i+1}',objective=o,criterion=c,levels={'0':'Violates publication, lifetime, identity or evidence contract.','1':'Reproduces ordinary LIFO fixture without concurrent invariants.','2':c+' Supply the causal counterexample and changed-condition proof.','3':'Independently reconstruct after at least7days under changed contention/reuse assumptions.'}) for i,(o,c) in enumerate(claims)]
 meta['objective_coverage']=[dict(objective=o,teaching=['#problem','#reasoning'],implementation=['D1','D2'],boundary=dict(applicable=True,evidence=['D3']),independent_assessment=[r['id'] for r in meta['rubric'] if r['objective']==o],assessed_claim='; '.join(r['criterion'] for r in meta['rubric'] if r['objective']==o)) for o in objectives]
 meta['status']['limitations']=['Finite author RC11/native runs are not arbitrary-history proof, physical timing or independent learner assessment.','Original IBM1986 and exact published C17/C23 remain access gaps; bounded index stack is an explicit adaptation.']
 body='''# X-T55-004: Treiber stack with protected lifetime

## Prerequisites
Complete X-T55-002 and retain its protect/publish/revalidate argument and paused-reader trace. Study PR-T55-02 strong/weak CAS progress, PR-T55-03 ABA and PR-T55-04 finite pool ownership. Supply the learner CAS, hazard and nonwrapping-identity diagrams. No private implementation is copied automatically: the stack's exact fixed-domain adapter is implemented here from C atomics.

## Environment
Use C17 Profile C and the valid-call/storage premises in CONTRACTS.md. Native pthreads, Python and GenMC are host verification only. Selected M4 translation/link dependency evidence is distinct from board execution.

## Problem
Implement all public operations in src/treiber.c for up to8 copied uint32 values and4 fixed participants. Push20,push21,pop,pop must return21,20. Add split protected pop and split retired-generation collection, including unchanged output on refusal. Run the exact one-slot paused-reader schedule and delayed-old-collector schedule in the supplied tests; then integrate full pop/push under real competing threads. A pool with CAS but no stack links, a FIFO queue or work-stealing deque is not a substitute.

## Necessity
This finite corpus names Treiber's actual stack, not merely tagged identity. Atomic head replacement does not protect ordinary node fields, a successful address comparison does not imply unchanged incarnation, and a scan from the old incarnation cannot reclaim the current one. A finite static pool avoids dangling addresses but not data races on reused contents or logical ABA.

## Reasoning
Derive the head/link invariant, successful CAS linearization points, protection total-order proof and unique retired-generation authority. Separate claimed storage from reachable values and failed allocation from logical stack fullness. Explain why a protected head's successor needs no separate hazard for this algorithm and why reinserting a removed live node invalidates that argument. Compare C++ atomics and garbage-collected language mechanisms without asserting automatic ABA or reclamation equivalence. Classify ISO C/runtime/caller-discipline enforcement.

## Deliverables
D1 representation, pre/postconditions, ownership state machine, SC proof and plausible incorrect explanations. D2 complete actual stack and reclamation code with unchanged-output boundaries. D3 independent sequential and concurrent history checks, six distinct causal controls and finite model graphs. D4 both changed implementations, helper/object audit, measured storage and explicit progress/resource/source limits.

## Verification
Run stack/reclaim/retirement anchors in six C17 compiler profiles. Reject six compiled semantic controls only after successful compilation. Compare12288 transitions/profile with an independent list/slot model and256 actual concurrent histories/profile with an independent real-time linearization search. Four native participants repeatedly transfer4000 unique values. Execute the actual implementation under finite GenMC RC11 models, with relaxed publication and missing revalidation controls; a timeout is unresolved evidence. Audit M4 atomic instructions, undefined symbols and emitted storage/stack separately. No finite pass proves absolute mastery.

## Rubric
Score R1-R4 separately. Level1 is ordinary fixture reproduction; level2 includes derived invariants, failed schedules, repaired controls and both variants. Level3 requires independent delayed reconstruction; assessor execution never awards it.

## Hints
H1 mark the successful head CAS rather than free-slot reservation. H2 a hazard must be published before revalidating the source, and remain until the last hazardous use. H3 carry the retirement generation across the scan; a later load of the current state is not the old scan's authority.

## Variants
V1 delays a protected reader and then an old collector across two node lifetimes, reduces the generation limit to3 and exhausts it completely. Preserve cleanup while refusing new reuse; explain finite memory denial of service.

V2 changes from serialized traces to actual competing participants, real-time history checking and RC11 exploration. Remove only release publication or only revalidation, retain causal counterexamples, and compare the original defined all-atomic live-reinsertion ABA specimen with the production API exclusion.

## Security
The shared contract defines the original local synthetic target, attacker scheduling/reuse capabilities, isolation, harmless membership/value failure, exploitability premises, repairs and regression controls. No third-party process, credentials or deployed target is involved. A model data race is not demonstrated arbitrary code execution.

## Validation
Use current source-bound reviews/v5/expansion/corpus_treiber reports. Primary Rochester push/pop and Michael hazard passages were inspected; IBM original retrieval failed. Published C17/C23 access remains separate. No board, independent human review or delayed learner assessment is fabricated.
'''
 (ROOT/'curriculum/v5/exercises'/f'{ident}.md').write_text('```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body,encoding='utf8')
 dump(ROOT/BASE/'taxonomy_extensions.json',dict(kind='required-competency-refinement-of-existing-leaf',scope_rationale='Finite84 lock-free-stack-treiber requires actual head/link push/pop, beyond retained queue/deque/tagged pool.',refinements=[dict(exercise=ident,required=True,parent_objectives=objectives,competencies=[dict(id=r['id'],outcome=r['criterion']) for r in meta['rubric']])]))
 names=[BASE+'/resources.json',BASE+'/taxonomy_extensions.json']
 dump(ROOT/BASE/'research_gate.json',dict(status='passage-inspected',research_input_sha256={p:sha(ROOT/p) for p in names},limitations='Selected author primary inspection, not independent review or IBM original access.'))
 dump(ROOT/BASE/'audit.json',dict(original_corpus_id='lock-free-stack-treiber',status='confirmed-required-mechanism-added',inspected_providers=['assessor/execution/concurrent.c','assessor/messaging/messaging.c','assessor/advanced_concurrency_depth/identity/identity.c'],finding='Actual providers are Michael-Scott queue, work-stealing deque and tagged pool/CAS respectively; none implements Treiber stack.',reference_gaps=['IBM RJ5118 official host timed out','Exact published C17/C23'],reused_inspection=dict(id='R-AP-N1570',path=reused,limits='Existing exact draft clauses, not newly claimed published edition.')))
 dump(ROOT/'fixtures/corpus_treiber/cases.json',dict(operations=[['push',20],['push',21],['pop',0],['pop',0],['pop',0]],expected=[21,20,None]))
 dump(ROOT/'labs/corpus_treiber/family.json',dict(version=1,family='corpus_treiber',title='Actual Treiber stack and bounded reclamation',build_system='cmake',assessor_root='assessor/corpus_treiber',dependencies=[],projects=[],units=[],exercises=[dict(id=ident,project='PR-T55-05',tests=meta['lab']['tests'])],tests=meta['lab']['tests'],campaign=dict(producer='tools/run_corpus_treiber.py',receipt='corpus_treiber_campaign.json',profiles=[c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan','tsan']],mutants_per_anchor=2,assertion_marker='TS_CHECK:',fixture_oracle='tools/verify_corpus_treiber.py',oracle_driver='treiber')))
 print('Authored X-T55-004 required actual Treiber stack')
if __name__=='__main__':main()
