"""Write only the three additive original sketch briefs and inspected registrations."""
import hashlib,json
from pathlib import Path
from common import ROOT,dump,sha
BASE='research/v5/expansion/corpus_sketch'
SOURCES=[
('count_min','R-SKETCH-CM','An improved data stream summary: The Count-Min sketch and its applications',['Graham Cormode','S. Muthukrishnan'],2005,'Journal of Algorithms55(1), April2005, pp58-75; author PDF',
 'https://www.cs.ox.ac.uk/people/graham.cormode/pubs/papers/cm-full.pdf',
 ['Section3, PDF6: width/depth counters, pairwise hash family and weighted update','Section4.1, PDF6-7: point minimum, nonnegative premise and Markov analysis'],
 'Original finite Boolean-affine hash family and checked merge are derived companions. No general turnstile/range/inner-product implementation claim.'),
('hyperloglog','R-SKETCH-HLL','HyperLogLog: the analysis of a near-optimal cardinality estimation algorithm',['Philippe Flajolet','Eric Fusy','Olivier Gandouet','Frederic Meunier'],2007,'AofA2007, DMTCS Proceedings AH, pp127-146',
 'https://algo.inria.fr/flajolet/Publications/FlFuGaMe07.pdf',
 ['Figure2 and Definition1/Theorem1, PDF4-5: rank/harmonic estimator and ideal multiset premise','Section4 Figure3, PDF13-15: finite32-bit registers, alpha constants and small/large range corrections'],
 'Figure3 was visually inspected. Original implementation makes the small/intermediate/large branches disjoint, following Section4 explanations rather than interpreting overlapping printed if clauses as sequential overwrites. Precision fold is derived, not a claimed paper implementation. No asymptotic proof reproduced.'),
('minhash','R-SKETCH-MH','Syntactic Clustering of the Web',['Andrei Z. Broder','Steven C. Glassman','Mark S. Manasse','Geoffrey Zweig'],1997,'SRC Technical Note1997-015, July25 1997',
 'https://www.microsoft.com/en-us/research/wp-content/uploads/1997/01/src-tn-1997-015.pdf',
 ['Defining similarity of documents, PDF3-4: set shingles and resemblance','Estimating resemblance and containment, PDF4-5: uniform permutation and MIN-s theorem'],
 'PDF5 theorem/formulas visually inspected. Companion specializes minimum sketches to repeated s=1 observations over a finite universe; the banded filter is a separately derived changed-condition task. No Web clustering or containment implementation claim.')]
DETAILS=[
 dict(number=5,name='count_min',title='Weighted Count-Min and overflow-safe partition merge',
 claims=['Derive and implement nonnegative weighted Count-Min with explicit affine hashes and all-or-nothing arithmetic.','Account for counter/hash storage and update/query/merge work; distinguish deterministic no-underestimate from probabilistic error bounds.','Validate exact cells and queries independently, reject collision/overflow mistakes and defend a changed partition-merge contract.'],
 problem='Implement sk_cm_init, sk_cm_add and sk_cm_query in src/count_min.c. The public contract specifies width, depth, bit-parity hashes, total/cell invariants and error publication. A zero-matrix row is legal and forces collisions: adding key1 weight7 makes key99 estimate7 even though key99 never arrived. Build an independent exact frequency table and a separate cell-grouping oracle. Prove which bound survives arbitrary fixed hashes and which bound needs independently sampled rows.',
 necessity='Exact hash-table counts grow with distinct keys. This bounded summary trades point-query error for fixed storage. A minimum is safe only under its nonnegative invariant; modulo overflow, saturation or undocumented deletion changes that invariant. Replacing a proof with a fixed-seed histogram cannot establish the probability of failure.',
 reasoning='Derive pairwise independence for uniformly random Boolean masks and biases, then the collision-mass expectation and Markov bound with width w and depth d. Name fixed-query/nonadaptive-stream and independent-row premises. Explain why an individual all-zero map is legal despite poor estimates. Derive O(depth*p*32) bit work for the present implementation, actual sizeof storage and merge cost. Identify the difference between overflow refusal and a successful but inaccurate collision estimate.',
 verification='Use exact weighted counts and independently grouped cells for all64 query keys in each corpus. Cover empty, weightzero, maximum uint64 mass, overflow unchanged snapshots, deliberately singular matrices and configuration mismatch. Enumerate the tiny two-bit affine family independently. The count_min anchor and both compiling controls must behave as specified; memory faults are not successful semantic rejections.',
 variant='X-T15-005-DELAYED: at least seven days later, implement sk_cm_merge for independently processed partitions under identical complete active hash configurations. Prove concatenated-stream equivalence, compare all cells and queries with a direct run, and test empty/self/mismatched/overflowing merges with destination snapshots. Give a concrete counterexample to merging different hash maps and to applying the nonnegative minimum argument after signed deletion.'),
 dict(number=6,name='hyperloglog',title='HyperLogLog ranks, numerical corrections and precision folding',
 claims=['Derive and implement32-bit HyperLogLog rank registers, harmonic estimate and explicit numerical failure boundary.','Account for finite hash collisions, numerical approximation and implementation cost separately from ideal-model statistical error.','Validate bit/rank/correction behavior independently and derive an exact state-level precision-fold transfer.'],
 problem='Implement sk_hll_init, sk_hll_add, sk_hll_estimate and sk_hll_merge in src/hyperloglog.c. Inputs are prehashed32-bit words; a domain/version identity labels their interpretation. High p bits select a register and the remaining suffix determines rank. Implement the explicitly disjoint correction ranges in CONTRACTS.md and an original logarithm approximation. Empty state must estimate zero; all-zero suffixes reach rank33-p. Identical input hashes must be idempotent.',
 necessity='Exact distinct counting requires retaining growing identity information. Rank summaries use fixed storage but discard identity and depend on hash behavior. Confusing the index prefix with the rank suffix, using arithmetic mean or forgetting the empty-state correction produces a different estimator. A legal register vector can exceed the real domain of finite-hash correction; refusal must preserve the caller output.',
 reasoning='Write the rank and harmonic-indicator equations, explain initialization and the three correction ranges, and derive a truncation bound for your logarithm approximation without confusing it with floating rounding. State the ideal independent-uniform model behind asymptotic standard error and explain why it is not a deterministic relative-error bound. Include validation scans in the actual operation cost; the supplied teaching API does not promise a constant-time trusted-state update.',
 verification='Build ranks independently from fixed-width bitstrings, including zero, allones, repeated collisions and every supported precision. Compare estimates with an independent logarithm implementation under declared tolerances; force all legal uniform rank vectors to exercise small/intermediate/large/refused cases. Test merge identity mismatch and active-register max equivalence, and demonstrate a collision-heavy stream that defeats a proposed unconditional accuracy claim.',
 variant='X-T15-006-DELAYED: at least seven days later, implement sk_hll_fold from finer to equal/coarser precision. Derive how removed index bits become rank-prefix bits, skip empty source registers, and compare with direct coarse processing for every precision pair. Test hashes with an earlyone in the extra prefix and an allzero extra prefix, failed precision increase, mismatched domain identity and self-fold. Explain why averaging estimates cannot implement the same state union.'),
 dict(number=7,name='minhash',title='Permutation MinHash, exact finite resemblance and candidate bands',
 claims=['Derive and implement finite permutation MinHash with precise empty-set, duplicate and union semantics.','Separate uniform minwise probability from arbitrary hash or correlated row behavior and account for configuration/signature costs.','Validate every subset pair against explicit set/permutation oracles and investigate false-positive/negative banded candidates.'],
 problem='Implement sk_mh_init, sk_mh_add, sk_mh_similarity and sk_mh_merge in src/minhash.c. Each row is an actual finite permutation, validated before publication. The signature stores minimum rank; a separate sentinel represents an empty set. Similarity is an exact fraction of matching positions, with explicit empty policy. Merge computes the signature of union under identical ordered permutations. Do not substitute a weak hash and silently retain a minwise guarantee.',
 necessity='Set resemblance differs from membership, frequency and containment. A bounded signature can estimate resemblance without retaining every element, but cannot reconstruct arbitrary intersections or multiplicity. The explicit small universe permits an exhaustive check of the mathematical probability identity instead of confusing a deterministic seeded sample with a distributional theorem.',
 reasoning='Prove that under a uniform permutation the earliest element of the union belongs to the intersection with Jaccard probability. Handle twoempty and oneempty cases separately. Derive union-by-minimum and construct indistinguishable signatures with different intersections. Distinguish sampling variance, repeated permutations and correlated row order. Give the actual O(k*universe) configuration validation and O(k) update/query work plus storage bounds.',
 verification='Enumerate all120 permutations on fiveelements and every one of the1024 subset pairs. Independently compute set intersection/union, minima and exact matching fractions. Cover malformed permutations, out-of-domain updates, empty/duplicate/idempotent cases, configuration mismatch and unchanged outputs on failure. The ten-row ordered case deliberately lacks independent randomness; report observed candidate errors without asserting a probabilistic calibration.',
 variant='X-T15-007-DELAYED: at least seven days later, implement sk_mh_bands with positive row groups that divide the signature count. Count only completely matching nonempty bands and preserve output on invalid grouping/configuration. Derive candidate probability1-(1-J^r)^b only under independent uniform rows. Exhibit a candidate with J<.5 and a missed pair with J>=.5 from explicit sets and the ordered ten-permutation family; explain why this filter is not a resemblance estimator.')]

def main():
    resources=[]
    for name,rid,title,authors,year,edition,url,locators,limits in SOURCES:
        capture=ROOT/f'build/corpus_sketch_sources/{name}.pdf'
        resources.append(dict(id=rid,title=title,authors=authors,year=year,edition=edition,kind='paper',url=url,access='Public primary author/institution PDF, downloaded and inspected',domains=['T15'],role='primary',reason='Required finite streaming-summary mechanism with explicit mathematical premises',prerequisites=['T04 unsigned arithmetic','T14 elementary probability','T15 invariants'],assignable=True,locators=locators,inspection=dict(date='2026-10-18',method='Read the named actual PDF passages; HLL practical algorithm and MinHash theorem also rendered and visually inspected.',scope='; '.join(locators),limits=limits,download_path=capture.relative_to(ROOT).as_posix(),download_sha256=sha(capture))))
    dump(ROOT/BASE/'resources.json',dict(resources=resources))
    dump(ROOT/BASE/'taxonomy_extensions.json',dict(schema_version=1,kind='required-competency-refinement-of-existing-leaf',source_records=[r['id'] for r in resources],scope_rationale='The accessible corpus names Count-Min, HyperLogLog and MinHash. A hash table or generic randomized-algorithm paragraph does not implement their summaries, merge laws or statistical premises. Preserve all original T15.15 obligations and add these required concrete mechanisms.',objectives=['T15.15.a','T15.15.b','T15.15.c'],exercises=[f'X-T15-{n:03}' for n in (5,6,7)],preservation='No replacement of accepted leaves, hours, original criteria or historical receipts.'))
    for detail,source in zip(DETAILS,resources):
        identifier=f'X-T15-{detail["number"]:03}';objectives=['T15.15.a','T15.15.b','T15.15.c'];rubric=[];coverage=[]
        for index,(objective,claim) in enumerate(zip(objectives,detail['claims']),1):
            rid=identifier+f'-R{index}'
            rubric.append(dict(id=rid,objective=objective,criterion=claim,levels={'0':'Missing mechanism or false semantic/probabilistic claim.','1':'Selected ordinary examples work, but a required premise, boundary or independent check is missing.','2':claim+' D1-D3 support the precise bounded contract.','3':'Independently completes D4 under its changed conditions and defends the altered proof, failure and cost model.'}))
            coverage.append(dict(objective=objective,teaching=['#problem','#reasoning'],implementation=['D1','D2'],boundary=dict(applicable=True,evidence=['D3']),independent_assessment=[rid],assessed_claim=claim,scope='Required streaming-summary refinement; other parent algorithms remain independently required.'))
        meta=dict(schema_version=1,id=identifier,title=detail['title'],project='PR-T15-10',domain='T15',objectives=objectives,families=list('ABCDEFGHIJ'),required=True,prerequisite_exercises=['X-T04-001','X-T15-002'],prerequisite_artifacts=[],profile='C',environment=dict(language='ISO C17 with exact-width integers; HLL requires at least53-bit double; no C23 dependency',target='Hosted Linux numerical and integer experiments; no board claim',toolchain=['GCC','Clang','CMake','Python'],dependencies='Original allocation-free sketch mechanisms. stdio and Python are hosted verification only. No external sketch/hash/math implementation. Cross-target runtime dependencies are not approved by hosted results.'),source_records=[dict(id=source['id'],path=BASE+'/resources.json',passages=source['locators'])],lab=dict(family='corpus_sketch',scaffold='labs/corpus_sketch',tests=['corpus_sketch.'+detail['name']]),assessor_analysis='assessor/corpus_sketch/ANALYSIS.md',status=dict(content='authored',execution='source-bound campaign required',independent_review='not-run',learner_assessment='not-assessed',limitations=['Finite deterministic author evidence is not a production hash-quality proof.','Author execution does not supply unaided or delayed learner evidence.','No physical target observation.'],campaign_receipt='reviews/v5/expansion/corpus_sketch/current_validation.json'),fixtures=['fixtures/corpus_sketch/cases.json'],shared_contracts=['labs/corpus_sketch/CONTRACTS.md'],delayed_transfer=dict(minimum_days=7,variant_ids=[identifier+'-DELAYED']),objective_coverage=coverage,rubric=rubric)
        body=f'''# {identifier}: {detail['title']}

## Prerequisites

Complete the named unsigned arithmetic and algorithm exercises. Bring an invariant notebook, independent test strategy and elementary expectation/probability reasoning. Read the exact registered primary passages and relevant CONTRACTS.md section; the assessor implementation is not a prerequisite.

## Environment

Use the declared C17/profileC contract. Implement only the original component; no third-party sketch or hash library may supply its mechanism. Keep hosted I/O and Python verification separate from the component. C23 mode supplies no automatic bounds, lifetime or statistical guarantee. Pointers, ownership, serialization and hash interpretation remain explicit caller premises.

## Problem

{detail['problem']}

## Necessity

{detail['necessity']}

## Reasoning

{detail['reasoning']}

## Deliverables

- D1: Derivation, invariants, at least two wrong explanations with counterexamples, exact source locators and assumption/enforcement table.
- D2: Completed ordinary C17 provider, explicit build/dependency manifest, state diagrams and storage/operation cost accounting.
- D3: Independent oracle, boundary and adversarial cases, unchanged-output snapshots, two compiling semantic controls and actual GCC/Clang profile records.
- D4: Required delayed mechanism, changed-condition proof, independent comparison and failure/cost analysis.

## Verification

{detail['verification']}

Configure labs/corpus_sketch with CMake, build and run its selected anchor. The default src files must compile then fail SK_CHECK. Supply your external completed source directory through SK_PROVIDER. Record commands, compiler versions, input/source identities and raw outcomes; reject crashes as semantic controls. The public tests are necessary examples, not a complete solution certificate. Use the full procedure in CONTRACTS.md.

## Rubric

Assess each JSON criterion separately against D1-D4. A false probability premise fails the related reasoning criterion even if all public examples pass. Level2 requires a correct bounded implementation and independent evidence; level3 requires unaided changed-condition work, not copying a retained author implementation.

## Hints

1. State exactly what information one stored counter/register/minimum retains.
2. Derive a merge law from that meaning before selecting an arithmetic operation.
3. Separate deterministic state invariants, numerical error and probabilistic assumptions in every result table.

## Variants

{detail['variant']}

## Security

The only targets are original local teaching providers and bounded synthetic inputs. Model adversarial keys/hashes, malformed configurations and arithmetic boundaries; do not probe external services. Demonstrate a minimized failure in a compiled mutant and the corrected behavior under the same input. A passing finite test is not a security proof, cryptographic hash certification or production resilience claim. Compare the repaired invariant and remaining collision/information-loss limitations explicitly.

## Validation

Source-bound author campaigns, separate review, learner performance, elapsed delayed assessment and physical observation are distinct. Learner sources remain intentionally unsolved. Only current campaign and exported-workspace receipts whose guards pass support executed author claims.
'''
        (ROOT/f'curriculum/v5/exercises/{identifier}.md').write_text('```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body,encoding='utf-8',newline='\n')
    print('Authored three additive sketch briefs and inspected source records; no catalog or global inventory write')
if __name__=='__main__':main()
