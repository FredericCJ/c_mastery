"""Canonical authoring source for four required corpus-discovered mechanisms."""
import hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4]
HERE=pathlib.Path(__file__).resolve().parent
def dump(path,value):
 path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
SOURCE=[
 ('R-CINDEX-FENWICK94','A new data structure for cumulative frequency tables',['Peter M. Fenwick'],1994,'https://static.aminer.org/pdf/PDF/001/073/976/a_new_data_structure_for_cumulative_frequency_tables.pdf',['Principles and implementation, printed pp329-333','Scaling and performance, printed pp334-336'],['T14'],'fenwick','Original 1994 article; third-party mirror. One-based adaptation is independently derived.'),
 ('R-CINDEX-FENWICK-ERRATA94','Corrections: A new data structure for cumulative frequency tables',['Peter M. Fenwick'],1994,'https://onlinelibrary.wiley.com/doi/epdf/10.1002/spe.4380240706',['Complete correction note, printed p677'],['T14'],'fenwick_correction','Publisher browser PDF read; private direct capture returned HTTP403. Corrections to Figures8 and9 apply to the original zero-origin algorithms.'),
 ('R-CINDEX-SEGMENT','AC Library segment tree documentation',['AtCoder Library authors'],2026,'https://atcoder.github.io/ac-library/production/document_en/segtree.html',['Monoid requirements, prod and all_prod','max_right and min_left predicate restrictions'],['T14'],'segment','Official C++ API contract read as a comparison. No provider implementation is linked or used.'),
 ('R-CINDEX-LAZY','AC Library lazy segment tree documentation',['AtCoder Library authors'],2026,'https://atcoder.github.io/ac-library/production/document_en/lazysegtree.html',['Mapping laws and composition(f,g)=f after g','apply and prod contracts and oracle costs'],['T14'],'lazy','Official C++ API contract; the C polynomial aggregate and failure model are original.'),
 ('R-CINDEX-INTERVAL05','Introduction to Algorithms lecture11: augmenting data structures',['Erik Demaine','Charles E. Leiserson'],2005,'https://www.cs.virginia.edu/~robins/cs4102/slides/Cormen%20et%20al%20Slides/PDF/11-Augmenting%20Data%20Structures.pdf',['Slides9-13: augmentation method, closed intervals and subtree maximum','Slides14-25: rotations, interval search and pruning proof'],['T14'],'interval','MIT course slides mirrored by University of Virginia. Source uses red-black trees and closed endpoints; this exercise derives AVL and half-open changes.'),
 ('R-CINDEX-SUFFIX90','Suffix arrays: a new method for on-line string searches',['Udi Manber','Gene Myers'],1990,'https://niplav.site/doc/cs/suffarr/suffix_arrays_a_new_method_for_on_line_string_searches_manber_myers_1990.pdf',['Sections2-3, printed pp320-323: search bounds and rank doubling','Section4, printed pp323-324: adjacent LCP and range minimum relation'],['T15'],'suffix','1990 conference paper, not the 1993 journal article. Third-party mirror. Plain binary search does not inherit the accelerated search bound.')]
access=json.loads((HERE/'source_access.json').read_text(encoding='utf-8'))['sources'];by={x['id']:x for x in access}
resources=[]
for id,title,authors,year,url,locators,domains,key,limit in SOURCE:
 r=dict(id=id,title=title,authors=authors,edition='Selected primary passage; version identified in inspection',year=year,kind='paper' if key in ('fenwick','fenwick_correction','suffix') else 'documentation',url=url,access='public',domains=domains,role='reference',reason='Derive and falsify the concrete index mechanism and its changed assumptions.',prerequisites=['C arrays, integer bounds and loop invariants'],locators=locators,assignable=True,inspection=dict(status='passage-inspected',date='2026-10-18',method='Direct browser primary text plus local PDF extraction where captured',scope='; '.join(locators),limits=limit))
 if by[key].get('sha256'):r['inspection']['sha256']=by[key]['sha256']
 resources.append(r)
dump(HERE/'resources.json',dict(resources=resources))
CASES=[
 dict(id='X-T14-003',title='Fenwick frequencies: rank boundaries and exact rescaling',project='PR-T14-09',domain='T14',topic='T14.14',case='fenwick',refs=SOURCE[:2],
 problem='Implement the five Fenwick operations in index.h for a synthetic adaptive-symbol frequency table. Begin with [1,0,3], submit each prefix and each rank selection, then apply delta=-2 at index2 and reject delta=INT64_MIN at index0 without state change. The component must support every n from0 to64 and total UINT32_MAX. Implement both rescaling modes; no arithmetic coder is required or implicitly provided. The exact one-based representation, output semantics and examples are in the shared contract.',
 necessity='Cumulative counts are an advanced-structure omission hidden by the old choose-one project. Correct prefix sums do not establish correct inverse selection. Zero counts and rank exactly on a partial sum distinguish strict from non-strict comparisons. Rounding aggregate nodes can disagree with rounding individual frequencies; the inspected original also has a published correction.',
 reasoning='Derive the lowbit partition and why update ancestors cover exactly the changed point. For select, maintain the skipped-prefix sum and prove the next skipped block stays <= the requested zero-based rank. Prove total admission bounds every affected cell and safely handles INT64_MIN. Derive linear build and inverse-build rescaling; compare their work with n independent point queries. Explain why nonnegative counts are essential for selection monotonicity.',
 verification='Enumerate all ternary vectors of length0..5, every prefix and rank, then signed updates, invalid indexes and both rescaling modes against a plain vector. Add [UINT32_MAX,0], all-zero tables, exact-total rank rejection and maximum-capacity inputs. Snapshot rejected initialization/update/rescale. Kill the rank-comparison and direct-cell-rescale mutants with distinct minimized traces.',
 criteria=['Derive and implement prefix/update/select and both rescaling semantics from the one-based frequency representation.','Prove nonnegative frequency, total, rank-search and failure-preservation invariants; account for build/rescale scratch and exact work.','Diagnose zero-frequency and rounding counterexamples, compare the published corrections, and independently transfer from floor to ceil rescaling.'],
 hints=['Draw the covered interval for bit indexes1..8.','The rank is a zero-based count of items that can be skipped, including zero-width blocks.','Reverse the linear parent-accumulation construction before rounding point values.'],
 variant='X-T14-003-DELAYED: replace floor rescaling with ceil rescaling so every previously positive symbol remains representable. Derive the new total bound, reconstruct point frequencies without n logarithmic prefix queries, and show why shifting every tree cell is incorrect. Implement both policies behind the supplied mode parameter and compare identical workloads. Also analyze the separate signed-frequency variant: prefix sums still exist, but binary rank selection is invalid without monotonicity.',
 analysis='The lowbit cells partition a prefix without overlap. Selection accepts a candidate block when its sum is <= the residual rank; using < returns the wrong zero-count boundary. For linear rescale, copy cells, walk indexes downward subtracting each child aggregate from its parent, round the recovered points, then rebuild upward. Since rounded nonnegative points never exceed their old values, total remains bounded. [1,1] already distinguishes rounding cells from rounding points. INT64_MIN must be compared against -old, not negated. Level3 requires independently deriving the ceil policy and rejecting rank-selection equivalence for signed values.'),
 dict(id='X-T14-004',title='Ordered segment aggregation with affine lazy maps',project='PR-T14-09',domain='T14',topic='T14.14',case='segment',refs=SOURCE[2:4],
 problem='Implement a fixed-capacity range index over residues modulo65521. Return all four fields of the ordered polynomial aggregate, and apply affine maps to half-open ranges. Use [1,2,3] and the exact examples in the shared contract as hand calculations. Then compose x->2x+1 followed by x->3x+4 on overlapping proper subranges; include assignment a=0. Queries must not mutate the object. No external segment-tree implementation or hashing library may perform this mechanism.',
 necessity='A range-sum example hides reversal because addition commutes. This aggregate exposes order errors, while affine tags expose wrong temporal composition. Assignment cannot be encoded by treating zero as an absent-tag marker. Algebraic laws, not a generic tree heading, justify lazy propagation.',
 reasoning='Prove identity and associativity by interpreting aggregates as sequences. Derive mapping over concatenation and newer-after-older affine composition. State exactly which stored aggregate already includes which pending tag; derive read-only query carry order. Bound intermediate uint64 products and node indexes. Show why a constant-sized algebraic operation is required for the quoted logarithmic operation cost. Explain why equal polynomial residues do not prove equal text.',
 verification='Compare every subrange of all ternary vectors length0..5 after identity, affine, assignment and overlapping updates with a flat residue vector. Include n0/n64, singleton and empty ranges, invalid coefficient/range preservation and read-only query snapshots. Verify power/geometric fields independently as well as hash. Kill reversed-join and reversed-tag-composition mutants; retain different witnesses.',
 criteria=['Construct the ordered monoid and implement range query plus affine updates including assignment and empty intervals.','Derive lazy composition and ownership/failure invariants, read-only query behavior and operation/resource bounds under constant-cost residue arithmetic.','Falsify commutativity and tag-order mistakes, distinguish a hash collision from equality, and transfer to overwrite mixed with affine updates.'],
 hints=['Represent one aggregate as its exact underlying sequence before doing algebra.','Write f(g(x)) symbolically; the coefficient of the old additive term exposes ordering.','A read-only query can carry ancestor mappings instead of pushing them into child nodes.'],
 variant='X-T14-004-DELAYED: admit a=0 assignment interleaved with older affine updates and later proper-subrange queries. Implement and derive the tag law without a special zero-as-no-tag convention. Supply a counterexample where sum-only tests pass reversed traversal. In the independent transfer analysis, replace residue addition with exact int32 sums and identify the new overflow/admission obligation; a modular result cannot be silently reused.',
 analysis='The empty tuple is (0,1,0,0). The right length shifts the left polynomial, so join direction matters. New(a,b) after old(c,d) yields (ac,ad+b); the wrong (ac,cb+d) can survive updates with b or d zero. A parent aggregate already reflects its own tag while children do not; query carries only ancestors, then composes the current tag before descending. a=0 must annihilate prior value dependence. Product bounds stay below 2^32 for residues but uint64 is used explicitly; integer promotions must not create signed overflow. Reference query is read-only. The finite oracle is not a proof of algebraic associativity; require the sequence derivation.'),
 dict(id='X-T14-005',title='Interval search with rotation-safe maximum endpoints',project='PR-T14-09',domain='T14',topic='T14.14',case='interval',refs=[SOURCE[4]],
 problem='Build a64-node typed AVL interval tree keyed by (low,id), with subtree maximum endpoint. Implement insert, removal, one-overlap search and sorted all-overlap enumeration with transactional capacity failure. Start with [1,3),[3,5), nested[-2,9), and duplicate-low intervals distinguished by id. Delete a two-child node whose replacement changes an ancestor maximum, then reuse freed slots. A returned value is copied; there are no borrowed-node handles.',
 necessity='A balanced search tree alone does not justify pruning. Stale maximum metadata can make search falsely report absence even when rotations preserve key ordering. Closed versus half-open endpoint conventions change both overlap and pruning inequalities. An all-results API adds an output-ownership and partial-publication failure boundary absent from find-one.',
 reasoning='Prove key ordering, AVL height/balance, exact subtree maximum and one-to-one partition of live/free nodes. Derive both pruning directions before choosing a branch. Show the bottom-up repair order after single/double rotations and successor deletion. State key uniqueness precisely: id alone is not unique. Derive O(log n) mutation/find and O(n) worst-case enumeration plus scratch, rather than importing a stronger bound from a different algorithm.',
 verification='Compare deterministic operation histories with an unsorted dictionary keyed by (low,id). After each insertion/deletion independently walk the tree to recompute heights, maximum and node reachability. Exercise all64slots, duplicate while full, missing removal, smallest/largest int32 endpoints and short output buffers. For both endpoint modes, enumerate queries touching stored endpoints. Kill omitted-child-maximum and closed-comparison-in-half-open-mode mutants.',
 criteria=['Implement the actual typed-node AVL interval index and complete mutation/find/all contracts without delegating balancing or searching.','Prove augmentation, pruning, free-list, deletion and failure atomicity, and distinguish find-one from enumeration resource bounds.','Diagnose stale metadata and touching-endpoint failures and independently transfer to closed intervals including point intervals.'],
 hints=['First write overlap as two inequalities for the chosen endpoint convention.','A rotation changes which descendants belong to each of its two roots.','Stage enumeration until capacity admission is known; publishing a prefix is not the stated failure contract.'],
 variant='X-T14-005-DELAYED: switch to closed intervals, admit low==high point intervals, and return every overlap in sorted key order. Re-derive pruning equality, not just the local overlap predicate. Demonstrate a touching-point case that changes result and a failed short-buffer call that preserves both count and array. Compare repeated find/delete enumeration with the supplied read-only traversal under equal output contracts.',
 analysis='Search-left is justified only when left.max reaches the query start (>= for closed, > for half-open). If that branch yields no intersection, key ordering and the witness reaching the start exclude the right branch in the standard find-one proof. Rotations repair the demoted root before the promoted root. Two-child deletion copies the successor value then repairs the removal path and all ancestors; count decreases once. The all traversal may visit O(n) nodes even when output is small; the implementation does not claim optimal output-sensitive enumeration. Free-list allocation has no fallible callback after admission. Endpoint extrema require comparisons, never subtraction.'),
 dict(id='X-T15-003',title='Byte suffix ranks, LCP ranges and disjoint repeated spans',project='PR-T15-10',domain='T15',topic='T15.15',case='suffix',refs=[SOURCE[5]],
 problem='Implement a copied byte-text suffix index for length0..64, including the empty suffix. Build ranks by stable integer-key doubling, then adjacent LCP. Provide exact pattern intervals, LCE and longest repeated spans with overlap permitted or forbidden. Reproduce the banana order/LCP/ranges in the contract, then use embedded0/255 and all-equal texts. Never use a zero byte as an implicit terminator or materialize n complete suffix copies in the component.',
 necessity='The retained specialized-algorithm project currently supplies run-length encoding evidence; it does not implement suffix indexing. Rank-pair stability, empty-suffix ordering and prefix-equal upper bounds are independent mechanisms. The longest adjacent LCP solves overlapping repetition but not all nonoverlap policies without further reasoning.',
 reasoning='Derive the rank equivalence for length2h from two lengthh ranks and justify stable secondary-then-primary sorting. Prove the permutation/sentinel invariant and adjacent-LCP scan bound. Derive LCE as an adjacent-minimum query, and explain the cost of the actual implementation. For disjoint repeats prove the length bound min(LCP,abs(offset difference)); state tie breaking. Contrast plain pattern search with the inspected accelerated algorithm without claiming its bound.',
 verification='Enumerate all texts over bytes{0,1,255} through length6 and all patterns through length3, plus length64 adversaries. Compare order with independently sorted copied byte suffixes and LCP with direct comparisons. Compare all-offset LCE and longest spans with exhaustive text-position pairs. Test absent/empty patterns and output preservation. Kill lost-secondary-rank and incorrect-upper-bound mutants separately.',
 criteria=['Implement rank doubling, adjacent LCP and exact byte-pattern range with owned text and the empty suffix.','Derive representation, sentinel, LCE and complexity arguments and account for scratch without borrowing source input.','Diagnose repeated-prefix/rank failures and independently derive and implement longest nonoverlapping repeated spans with deterministic ties.'],
 hints=['A suffix rank describes a prefix-equivalence class, not just its first byte.','An equal prefix belongs below the upper bound but not below the lower bound.','For two starts, nonoverlap adds a distance bound to their common-prefix bound.'],
 variant='X-T15-003-DELAYED: forbid overlap between repeated spans and choose lexicographically smallest (start1,start2) among maximum-length results. Implement both modes and give a repeated-character counterexample to blindly returning the longest adjacent LCP. Then analyze replacing copied text with a borrowed mutable span: show an actual mutation that invalidates ranks and state the ownership repair. No logarithmic or linear-time claim is required for the bounded pair-scan transfer.',
 analysis='Byte ranks begin at1 and the empty/out-of-text rank at0. Stable sorting by the second key and then first key produces pair order; new ranks must compare both keys. The adjacent LCP decrement proof uses suffixes one character shorter and at most linear total successful extensions. Between ranks x<y the common prefix is the minimum lcp[x+1..y]. Plain binary search compares up to pattern length at each level and is O(m log n). Nonoverlap clips pair length by start distance; maximizing only adjacent LCP without this constraint is wrong (aaaa). Pair scan with running LCP minima is O(n^2) and uses deterministic text-coordinate tie breaking. The copied-text ownership eliminates postbuild mutation of the input as a state change.')]
public=['labs/corpus_index/'+p for p in ['index.h','tests.c','driver.c','benchmark.c','reject_mutant.py','CMakeLists.txt','dependency-contract.md']]
for c in CASES:
 id=c['id'];objectives=['T14.14.b','T14.15.b','T14.15.a'] if c['domain']=='T14' else ['T15.15.a','T15.15.b','T15.15.c'];rubric=[];coverage=[]
 for i,(objective,criterion) in enumerate(zip(objectives,c['criteria']),1):
  rid=id+'-R'+str(i);rubric.append(dict(id=rid,objective=objective,criterion=criterion,levels={'0':'Missing or semantically incorrect implementation/argument.','1':'One example passes; the named invariant, rejection or changed-condition argument is absent.','2':criterion+' Supply the specified independent model, boundary witnesses and resource argument.','3':'Independently derive the named delayed change and explain which proof, implementation and cost assumptions must be revisited.'}));coverage.append(dict(objective=objective,teaching=['#problem','#necessity','#reasoning'],implementation=['D1','D2'],boundary=dict(applicable=True,evidence=['D3']),independent_assessment=[rid],assessed_claim=criterion,scope='Named required mechanism within the existing advanced/specialized objective; no blanket closure of its other mechanisms.'))
 meta=dict(schema_version=1,id=id,title=c['title'],project=c['project'],domain=c['domain'],objectives=objectives,families=['A','B','C','D','F','H','I','J'],required=True,prerequisite_exercises=[],prerequisite_artifacts=[],profile='C',environment=dict(language='C17; exact published standard remains separately identified as a source gap',target='Hosted Linux verification, fixed-capacity portable core; target audit separately reported',toolchain=['GCC','Clang','CMake','Python3'],dependencies='Original profile C index implementation; host-only libc and Python verification'),source_records=[dict(id=s[0],path='research/v5/expansion/corpus_followthrough/resources.json',passages=s[5]) for s in c['refs']],lab=dict(family='corpus_index',scaffold='labs/corpus_index',tests=['corpus_index.'+c['case']]),assessor_analysis='assessor/corpus_index/analyses/'+id+'.md',status=dict(content='authored',execution='see-source-bound-campaign',independent_review='not-run',limitations=['Author implementation and execution are distinct from independent learner assessment.','Source readings are the exact registered passages, not whole-book inspection.','Physical observations and universal correctness are not claimed.'],campaign_receipt='reviews/v5/expansion/corpus_followthrough/corpus_index_campaign.json',learner_assessment='not-assessed'),fixtures=public+['fixtures/corpus_index/cases.json'],shared_contracts=['labs/corpus_index/CONTRACTS.md'],delayed_transfer=dict(minimum_days=7,variant_ids=[id+'-DELAYED']),objective_coverage=coverage,rubric=rubric)
 body=f'''# {id}: {c['title']}

## Prerequisites

Read {c['project']} and the applicable T14/T15 module teaching. Bring your own array/bounds tests and invariant notebook; no unlisted accepted artifact import is required. Know unsigned arithmetic, half-open spans, typed object lifetime and asymptotic work. Interval work additionally requires AVL rotations; suffix work requires stable counting sort. Diagnose those prerequisites before attempting the integrated mechanism.

## Environment

The explicit shared contract is [corpus index contract1](../../../labs/corpus_index/CONTRACTS.md). It supplies the API, representation, ownership, null/disjointness premises, status precedence, execution profile, target restrictions and evidence limits. Use the supplied compiling learner scaffold; author solutions remain separate. Record actual compiler/data-model assumptions. ISO C type checking does not establish the semantic invariants or pointer preconditions.

## Problem

{c['problem']}

## Necessity

{c['necessity']}

## Reasoning

{c['reasoning']} Include one plausible incorrect explanation and its smallest counterexample. Separate mathematical proof, ISO C guarantees, compiler diagnostics, dynamic checks and caller discipline. Fixed bounds make experiments tractable; they do not excuse a false general complexity claim.

## Deliverables

- D1: Derivation, ownership/admission proof and source comparison with exact inspected locators; distinguish the source algorithm from this C adaptation.
- D2: Original implementation, reproducible strict builds, dependency/stack/storage/work ledger and generated-code inspection for any target claim.
- D3: Independently represented oracle, complete experiment inputs, expected/observed outputs, failure snapshots and two distinct killed semantic mutants.
- D4: Named delayed variant source, new proof/counterexample and resource comparison under equivalent correctness contracts; retain its study and assessment identities.

## Verification

{c['verification']}

Configure `cmake -S labs/corpus_index -B <fresh-build>`, build, then run `ctest --test-dir <fresh-build> -R '^corpus_index.{c['case']}$' --output-on-failure`. The unsolved stub must compile and fail with CI_ASSERT. Public tests are necessary, not sufficient: independently design D3 and submit D1/D4. The assessor additionally runs different flat Python models and compiled defect controls. Compare GCC/Clang O0/O3 and sanitizers; record unavailable target evidence rather than fabricating it.

## Rubric

Apply all three objective-specific JSON criteria. Each must reach2 for ordinary bounded acceptance and3 for the changed-condition argument. A passing output with a false ownership, algebra or complexity argument fails that criterion. No finite passing collection proves absolute mastery. The same original objective can support several mechanisms only because each has its own explicit rubric here.

## Hints

1. {c['hints'][0]}
2. {c['hints'][1]}
3. {c['hints'][2]}

## Variants

{c['variant']}

Take the named delayed assessment at least7days after the source-bound study event. Submit the changed input/contract before coding, and explain transfer without copying the assessor implementation. This is required work; historical project hours remain descriptive metadata.

## Security

Use only original local curriculum code and synthetic byte/number inputs in an unprivileged process. Bound generators and subprocess time. This is an adversarial correctness/resource exercise; no exploitability or external service claim is implied by a failed check. A polynomial residue is not an authentication tag. Null/lifetime-invalid addresses are not executed to test an impossible portable pointer validator.

## Validation

Authored status and the source-bound campaign receipt distinguish implemented, executed and unexecuted obligations. Read the actual receipt before claiming a profile passed. External review, independent learner performance and physical observation remain separate; none is inferred from author tests. All changed-condition code and exact witnesses are required even when a public anchor passes.
'''
 (ROOT/'curriculum/v5/exercises'/f'{id}.md').write_text('```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body,encoding='utf-8')
 p=ROOT/meta['assessor_analysis'];p.parent.mkdir(parents=True,exist_ok=True);p.write_text(f'''# {id} assessor analysis

This file is withheld from learner exports. The reference C files and independent
Python oracle in this directory are author evidence, not evidence of learner mastery.

{c['analysis']}

## Objective-specific assessment

{chr(10).join('- '+r['id']+': '+r['criterion'] for r in rubric)}

Require a preimplementation prediction, minimized mutant witness and derivation,
not a recital of this analysis. Ask the learner to change the stated premise in
the named delayed variant after at least7days; score each criterion separately.
Wrong invariants fail despite coincidentally correct outputs. Record exact source,
compiler, independent inputs and evidence limits. No human-independent review is
claimed for this author material.

## Withheld discriminators

Use an unfamiliar ordering of equal/touching/zero-width logical states where
the chosen contract permits them, ask for the first invalid assumption before
running tests, and independently reconstruct expected values from the flat model.
Retain the exact named delayed variant and require its new resource argument.
The public fixtures and the oracle are finite falsification tools, not exhaustive
claims for arbitrary C implementations or arbitrary integer/text capacity.
''',encoding='utf-8')
dump(ROOT/'fixtures/corpus_index/cases.json',dict(schema_version=1,cases=[dict(id=c['id'],anchor='corpus_index.'+c['case'],delayed=c['id']+'-DELAYED',mechanism=c['title'],mutants=[i*2+1,i*2+2]) for i,c in enumerate(CASES)],limits='Synthetic local teaching fixtures; expected results are derived independently in tests and assessor oracle.'))
dump(ROOT/'labs/corpus_index/family.json',dict(version=1,family='corpus_index',title='Corpus-discovered exact index mechanisms',build_system='cmake',assessor_root='assessor/corpus_index',projects=[],units=[],modules=['T14','T15'],tests=['corpus_index.'+c['case'] for c in CASES],exercises=[dict(id=c['id'],project=c['project'],tests=['corpus_index.'+c['case']],capabilities=['linux-c17'],scope=c['title']) for c in CASES],contract='labs/corpus_index/CONTRACTS.md',fixture='fixtures/corpus_index/cases.json',campaign=dict(producer='tools/run_corpus_index.py',profiles=['gcc-debug','gcc-release','gcc-asan','clang-debug','clang-release','clang-asan'],assertion_marker='CI_ASSERT:',mutants_per_anchor=2,fixture_oracle='assessor/corpus_index/oracle.py',oracle_driver='corpus-index-driver'),remaining_evidence='Independent learner reasoning/transfer, external review and any physical claims remain separate.'))
dump(HERE/'taxonomy_refinement.json',dict(status='required-additive-mechanisms',baseline_text_unchanged=True,rows=[dict(exercise=c['id'],objectives=['T14.14.b','T14.15.b','T14.15.a'] if c['domain']=='T14' else ['T15.15.a','T15.15.b','T15.15.c'],omission=c['necessity'],rationale='The existing selected advanced structure and verification/specialized algorithm objectives admit this mechanism. T14.14.a specifically names B-trees/skip lists/probabilistic filters and T14.14.c concerns persistence/probability; neither is credited by these exact indexes. Headings and choose-one work were insufficient evidence; explicit facets are added without replacing other mechanisms.') for c in CASES]))
