"""Resolve the already-read finite84 descriptions against concrete mechanisms.

No new corpus discovery. Metadata identity is not primary-passage inspection.
"""
import hashlib,json,pathlib,re
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def brief(i):return json.loads(re.search(r'```json\s*(.*?)```',(ROOT/'curriculum/v5/exercises'/f'{i}.md').read_text(encoding='utf8'),re.S)[1])
rows={}
def new(element,ids,folder,mechanism,limits):
 ids=ids.split();metas=[brief(i) for i in ids];packet='research/v5/expansion/'+folder+'/completion_dispositions.json'
 assert (ROOT/packet).is_file(),packet
 evidence={packet};rubrics=[];passages=[]
 for i,m in zip(ids,metas):
  evidence|={'curriculum/v5/exercises/'+i+'.md',m['assessor_analysis'],*m['shared_contracts'],*m.get('fixtures',[])}
  evidence|={p.relative_to(ROOT).as_posix() for p in (ROOT/'assessor'/m['lab']['family']).rglob('*.c')}
  rubrics += [r['id'] for r in m['rubric']];passages+=m['source_records']
 rows[element]=dict(status='bounded-mechanism-implemented',origin='required concrete companion or executed successor',exercises=ids,criteria=rubrics,mechanism=mechanism,limits=limits,evidence=sorted(evidence),primary_passage_records=passages,inspection_provenance='Concrete mechanism/contract and provider evidence reviewed; exact primary-inspection scope is inherited only from the named author source records, not from the discovery title.')

NEW=[
('compression','X-T15-004','corpus_codec','Original bounded canonical Huffman and LZ match/copy construction, malformed decode, capacity and transfer checks.','Not every compression family or production format.'),
('error-correcting-codes','X-T15-008','corpus_ecc','Systematic parity/syndrome correction with explicit error model and uncorrectable/miscorrection boundaries.','Finite code/channel assumptions; no universal correction or physical-channel claim.'),
('fenwick-tree','X-T14-003','corpus_followthrough','Binary indexed partial sums, updates and changed range-query obligations.','Bounded integer/storage model and stated overflow rules.'),
('segment-tree','X-T14-004','corpus_followthrough','Segment aggregation/update invariants and independent range oracle.','Specified algebra and updates, not every lazy operation.'),
('interval-tree','X-T14-005','corpus_followthrough','Augmented ordered interval tree with overlap pruning, boundaries and update repair.','Specified interval conventions, not interval arithmetic.'),
('suffix-array','X-T15-003','corpus_followthrough','Suffix ordering and pattern interval construction checked independently.','Suffix array is not a suffix tree or FM index; those are comparison leads, not falsely claimed implementations.'),
('count-min-sketch','X-T15-005','corpus_sketch','Count-Min row minima, collision overestimate and update/merge contracts.','Assumed hash/error model; no universal error bound from samples.'),
('hyperloglog','X-T15-006','corpus_sketch','Register rank, estimate and union with explicit finite hash domain.','Estimator assumptions and small/large-range limits retained.'),
('minhash','X-T15-007','corpus_sketch','Set-signature minima and resemblance with collision/empty-set policies.','Finite signatures and stated sampling model.'),
('t-digest','X-T15-009','corpus_tdigest','Normalized k2 centroid admission, ordered merge and two-level recompression.','Observed maximum rank error0.625 and merge/arithmetic order differences remain; no universal accuracy claim.'),
('bitmap-index','X-T14-012','corpus_query','Predicate bitmaps, live-row masking, Boolean queries and atomic rebuild on replacement.','At most32 copied rows; full rebuild cost, no persistent transaction.'),
('inverted-index','X-T14-013','corpus_query','Term postings and positions, phrase intersection and update consistency.','Bounded token vocabulary; not a general search service.'),
('secondary-index','X-T14-012 X-T14-013','corpus_query','Actual qi_build/build_terms map non-primary term attributes to stable row IDs; qi_replace stages base row and all derived masks/postings before publication. Stale-index mutant and scan oracle verify every write.','RAM strong publication and full reindex write amplification are explicit; no durable or concurrent transaction claim.'),
('zone-map','X-T14-014','corpus_query','Conservative min/max summaries, safe pruning and lazy-update false positives.','Summary may admit extra candidates; exact predicate still runs.'),
('k-d-tree','X-T14-006','corpus_spatial','Axis split and nearest-neighbor lower-bound search.','Finite dimension/domain and tie policy, not universal best index.'),
('bounding-volume-hierarchy','X-T14-007','corpus_spatial','Conservative enclosing volumes and branch pruning with refit.','Explicit geometry and finite workload.'),
('quadtree-octree','X-T14-008','corpus_spatial','Point-region subdivision and executed 2D-to-3D changed dimension.','Finite cell/domain/depth rules; no arbitrary geometry engine.'),
('r-tree','X-T14-009','corpus_spatial','Overlapping bounding-box grouping, split and multi-branch search.','Bounded fanout and exact query contract.'),
('uniform-spatial-hash-grid','X-T14-010','corpus_spatial','Signed cell coordinates, neighbor stencil and duplicate suppression.','Hash collisions are candidates, not geometric equality.'),
('spatial-partition','X-T14-006 X-T14-007 X-T14-008 X-T14-009 X-T14-010','corpus_spatial','Explicit comparison of grid/cell, axis and enclosing-volume partitions under equivalent query results.','Broad description resolved by assessed concrete alternatives, not a duplicate sixth implementation.'),
('scene-graph','X-T14-011','corpus_scene','Hierarchical transforms, cached world state and changed-parent invalidation.','Original finite scene, not a renderer.'),
('navigation-mesh','X-T16-004','corpus_scene','Polygon adjacency, portal path and bounded geometry transfer.','Selected finite geometry/agent assumptions, not arbitrary game navigation.'),
('consistent-hashing','X-T76-010','corpus_ring','Actual ordered token ring, virtual nodes and measured membership remapping.','Ring implementation is distinct from retained weighted/rendezvous routing; no production cluster.'),
('skip-list','X-T14-015','corpus_storage','Multi-level ordered links, seeded promotion, allocation rollback and changed promotion probability.','Expected cost depends on explicit random model; adversarial/worst-case remains linear.'),
('pseudorandom-number-generator','X-T14-015','corpus_storage','Actual state recurrence/full-period argument, unbiased bounded rejection with finite fuel and affine jump-ahead composed into promotion.','Not cryptographic or independent random samples; finite period and rollback are explicit.'),
('copy-on-write-b-tree','X-T14-016','corpus_storage','Immutable published pages, copied update path, split propagation, retained roots and all-root reclamation.','Bounded RAM pages; tombstone/vacuum policy, not durable disk or structural deletion.'),
('b-tree','X-T14-016','corpus_storage','Actual multi-key page ordering/occupancy, split propagation and lookup, with explicit path-copy ownership.','This concrete B-tree plus retained B+tree contrasts internal records with leaf-only storage; not every B-tree variant.'),
('lsm-tree','X-T15-017','corpus_storage','Memtable flush, sorted runs, sequence winners, partial/full compaction and tombstone retention.','Logical bounded storage model; not real disk durability or an entire production engine.'),
('erase-remove','X-T14-017','corpus_erase','Stable retained prefix/logical end versus physical erase; fallible owning-tail publication and actual C++ lifetime comparison.','Original stronger staged contract differs from unspecified C++ tail and owning move destruction timing.'),
('arbitrary-precision-arithmetic','X-T11-002','corpus_limbs','Runtime count/capacity limbs, add/sub/multiply/division, signed truncation and fallible owning growth.','No fixed implementation limb maximum; physical memory and caller capacity still bound execution.'),
('interval-arithmetic','X-T17-003','corpus_interval','Outward enclosure and dependency overestimation with independent rational oracle.','Specified finite arithmetic operations and environment, not arbitrary real-expression exactness.'),
('money','X-T17-004','corpus_money','Exact scale/currency, rounding and allocation with conserved totals.','Specified currency/rounding policies, not financial advice or every monetary convention.'),
('regular-expressions','X-T15-010','corpus_parsing','Explicit regular-language automaton construction/matching and malformed-pattern handling.','Original grammar, not all PCRE/backreference features.'),
('lr-parser','X-T15-011','corpus_parsing','SLR item/table generation, shifts/reductions and conflict diagnosis.','Selected grammar/table family, not every LR variant.'),
('pratt-parser','X-T15-012','corpus_parsing','Binding-power expression parsing and associativity transfer.','Selected operators and typed AST.'),
('scannerless-parsing','X-T15-013','corpus_parsing','Grammar-level character consumption and lexical/grammar ambiguity comparison.','Original finite language, not general ambiguity resolution.'),
('panic-mode-error-recovery','X-T15-014','corpus_text','Explicit synchronization, diagnostic spans and resumed independent statement parsing.','Declared recovery points can discard input; not arbitrary syntax repair.'),
('pretty-printer','X-T15-015','corpus_text','Document algebra, groups/nesting/line alternatives and width-aware rendering.','Declared greedy layout and bounded work, not globally optimal formatting.'),
('template-engine','X-T15-016','corpus_text','Restricted bytecode, scoped list/current values, conditionals/includes and contextual escaping.','No arbitrary code execution or universal output context.'),
('shunting-yard-algorithm','X-T15-018','corpus_shunting','Operator stack and output order with precedence/associativity and malformed-token checks.','Concrete algorithm distinct from retained recursive descent and Pratt.'),
('event-driven-parsing','X-T15-019','corpus_events','Actual streaming nested START/CHARACTERS/END/DOCUMENT callbacks, pending event and separate consumer.','Restricted XML-derived ASCII grammar, not XML/SAX conformance or DOM equivalence.'),
('maximal-munch','X-T15-020','corpus_munch','Last accepting automaton state and rollback to longest accepted token boundary.','Selected regular rules and explicit lookahead/work model.'),
('semantic-model','X-T15-021','corpus_model','Separate persistent semantic state, names/types and failure-atomic statement application.','Selected original language; syntax tree alone is not semantic state.'),
('read-eval-print-loop','X-T15-021','corpus_model','Actual repeated statement read/evaluate/print with persistent definitions and failure survival.','Original bounded interpreter, not a general shell.'),
('escape-sequence','X-T15-022','corpus_escape','Paired textual encoding/inverse, once-only grammar and actual template/events numeric-versus-named adapter.','Specific text context, not browser/script sanitizer, Unicode codec or generic XML.'),
('lock-free-stack-treiber','X-T55-004','corpus_treiber','Actual CAS head Treiber push/pop, protected lifetime and finite participant/node reclamation protocol.','Four participants/eight nodes; no live-node reinsertion, arbitrary-history proof or board claim.')]
for args in NEW:new(*args)

def retained(element,objectives,folder,files,mechanism,limits,exercises=''):
 evidence=set(files.split());criteria=[];ids=exercises.split()
 packet=('research/v5/modern/'+folder+'_completion_dispositions.json' if folder in ['dod','coroutines','replicated'] else 'research/v5/expansion/'+folder+'/completion_dispositions.json')
 evidence.add(packet)
 for i in ids:
  m=brief(i);evidence|={'curriculum/v5/exercises/'+i+'.md',m['assessor_analysis'],*m['shared_contracts']};criteria += [r['id'] for r in m['rubric']]
 if objectives:
  wanted=set(objectives.split())
  for p in (ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T*.json'):
   d=read(p)
   matches=[c['id'] for c in d['criteria'] if c['objective'] in wanted]
   if matches:criteria+=matches;evidence.add(p.relative_to(ROOT).as_posix())
  assert criteria,(element,wanted)
 rows[element]=dict(status='bounded-mechanism-implemented',origin='retained concrete implementation and assessed transfer',exercises=ids,criteria=sorted(set(criteria)),mechanism=mechanism,limits=limits,evidence=sorted(evidence),primary_passage_records=[],inspection_provenance='Actual retained source/contracts/tests and exact objective bindings inspected in this followthrough; no new claim to have read every bibliography item. Existing source-register limits remain.')

C='assessor/containers_depth/';L='labs/containers_depth/CONTRACTS.md'
CONTAINERS=[
('dynamic-array','T14.03.a T14.03.b T14.03.c','sequence.c','Doubling reserve/shrink/insert, unchanged allocation failure, invalidation and copied resource rollback.','Expected amortized copies are separated from per-operation latency.'),
('fixed-capacity-container','T14.01.b T14.02.a','sequence.c','Actual typed fixed-capacity array, zero capacity, logical size and admission without out-of-range pointers.','Capacity is explicit, not automatic dynamic growth.'),
('linked-list','T14.04.a T14.04.b','ordered.c transfer.c','Singly/doubly/circular intrusive links, sentinel and detach/iteration ownership.','Live owner/node premises; no arbitrary pointer corruption defense.'),
('embedded-pointer','T14.04.c','ordered.c transfer.c','Task with two embedded intrusive links: independent memberships and complete detach before destruction.','Embedded links are not owning wrapper nodes; lifetime is caller discipline.'),
('stack','T14.05.b','transfer.c','Actual back/back deque operations implement LIFO, contrasted with other end combinations and movement costs.','One actual representation realizes the abstract stack; no duplicate function-name quota.'),
('queue-fifo','T14.05.a T14.05.c','transfer.c','Actual typed linked queue and count-ring, FIFO refill/refusal and partial-versus-strong admission.','Serialized queue; concurrent protocol is separately assessed.'),
('deque','T14.05.b','transfer.c','Both ends of typed bounded deque with LIFO/FIFO histories and representation cost comparison.','Specified fixed capacity, not a concurrent work-stealing deque.'),
('hash-table','T14.07.a T14.07.b T14.07.c','indexed.c','Actual chaining and bounded linear probing, tombstones/rebuild, copied keys and equality/hash consistency.','Collision-heavy linear behavior remains; no universal constant-time claim.'),
('binary-heap-priority-queue','T14.08.a T14.08.b T14.08.c','ordered.c','Bottom-up/repeated heap construction, stable ties, update handles and swap-map repair.','Finite generation retirement and child-index admission are explicit.'),
('binary-search-tree','T14.09.a T14.09.b T14.09.c T14.10.b','ordered.c','Actual unbalanced BST transplants/traversal plus full AVL repair and multi-ancestor deletion.','No claim of an unimplemented full red-black repair implementation.'),
('trie','T14.11.a T14.11.b T14.11.c','indexed.c','Actual byte trie with empty/binary keys, copied edges, terminal deletion/pruning and cursor invalidation.','Compressed-radix split remains explicit analysis, not claimed compiled trie variant.'),
('bitmap-bitset','T14.12.a','indexed.c','Finite-universe union/intersection/difference/complement and full-word shift boundary.','Not the separate multi-predicate bitmap-index mechanism.'),
('sparse-set','T14.12.b T14.12.c','indexed.c','Dense/sparse reverse map and generation clear with actual wrap maintenance.','Membership tags do not establish external stable identity.'),
('disjoint-set-union-find','T14.13.a','indexed.c','Union by size/path compression and bounded rollback log with unchanged log exhaustion.','Declared rollback cost and finite node domain.'),
('bloom-filter','T14.14.b T14.14.c','indexed.c','Actual approximate membership, false-positive enumeration and counting deletion with exact key ledger.','Deletion ledger cost and hash/distribution assumptions retained; not zero-error membership.')]
for element,objectives,files,mechanism,limits in CONTAINERS:
 retained(element,objectives,'legacy_containers',' '.join([L,*[C+f for f in files.split()],'research/v5/expansion/legacy_containers/bindings.py']),mechanism,limits)

for element,ids,files,mechanism,limits in [
 ('gap-buffer','X-T70-008','assessor/dod/dod_text.c labs/dod/tests/text_ext.c','Actual gap movement/edit ownership and243 scripts/1215 prefixes.','Bounded text storage and scalar edits.'),
 ('piece-table','X-T70-008','assessor/dod/dod_text.c labs/dod/tests/text_ext.c','Actual original/add buffers, staged pieces and compaction.','Explicit backing lifetime and finite piece capacity.'),
 ('rope','X-T70-008','assessor/dod/dod_text.c labs/dod/tests/text_ext.c','Persistent text DAG, structural sharing, edits and metrics.','Selected byte/scalar domain and retained-root budget.'),
 ('zipper','X-T70-006','assessor/dod/dod_persistent.c labs/dod/tests/persistent_ext.c','Actual focused path/context reconstruction across8 paths and4 allocation cuts.','Persistent focus is not arbitrary mutable iterator equivalence.'),
 ('persistent-data-structure','X-T70-006','assessor/dod/dod_persistent.c labs/dod/tests/persistent_ext.c','Path-copy updates preserve old roots and resource budgets.','Persistent in-memory versioning is distinct from crash durability.'),
 ('hash-array-mapped-trie-hamt','X-T70-007','assessor/dod/dod_persistent.c labs/dod/tests/persistent_ext.c','Actual hash-fragment trie, collisions, path copying and root retention.','No automatic equivalence with ordered COW B-tree.'),
 ('slot-map','X-T70-004','assessor/dod/dod.c','Typed stable slots plus dense iteration and finite owner/generation identity.','Live stable owner and generation retirement; not unforgeable capabilities.')]:
 retained(element,'','dod',files+' tools/verify_dod_extensions.py',mechanism,limits,ids)
retained('conflict-free-replicated-data-type-crdt','','replicated','assessor/replicated/replicated.c','Actual counter/register joins, observed-remove sets and sequence examples with algebraic and membership limits.','A finite selection of CRDT mechanisms, not every datatype or a universal convergence proof.','X-T76-004 X-T76-005 X-T76-006')
retained('merkle-tree','','replicated','assessor/replicated/replicated.c','Domain-separated leaves/internal nodes, deterministic tree shape and differing-subtree repair.','Merkle identity is not authenticated authorization by itself.','X-T76-007')
retained('lock-free-fifo-queue-michael-scott','T26.04.a','legacy_advanced_reclamation','labs/advanced_concurrency_depth/reclamation/CONTRACTS.md assessor/advanced_concurrency_depth/reclamation/reclamation.c','Actual index-node Michael-Scott head/tail protocol with hazard protection, registration and bounded reclamation.','Not Treiber stack; progress and retention premises are explicit and no arbitrary-history proof is inferred.')
retained('checked-arithmetic','T04.03.a','legacy_integer_kernel','assessor/integer_kernel/integer.c','Admission before signed arithmetic, synthetic range and unchanged failure.','Checks prove only the declared operation/range, not surrounding pointer validity.')
retained('saturation-arithmetic','T04.03.a','legacy_integer_kernel','assessor/integer_kernel/integer.c','Actual checked arithmetic followed by explicit endpoint clipping and status.','Clipped result is not exact mathematical arithmetic.')
retained('epsilon-ulp-float-comparison','T04.10.c','legacy_float_contract','assessor/float_contract/float_contract.c','Absolute/relative policy and selected binary64 ULP comparison with NaN/signed-zero cases.','Tolerance is a declared problem contract, not universal equality.')
retained('fixed-point-arithmetic','T17.03.a T17.03.b T17.03.c','legacy_numerics','assessor/numerical_depth/fixed.c research/v5/expansion/legacy_numerics/bindings.py','Actual widened Q7/Q11 multiply/accumulate, ties-away and saturation.','Rounding-each and rounding-final differ; unclipped quantization bounds are scoped.')
retained('kahan-summation','T17.02.a T17.02.b','legacy_numerics','assessor/numerical_depth/sum.c research/v5/expansion/legacy_numerics/bindings.py','Actual Kahan, Neumaier and fixed pairwise comparisons against represented-input exact references.','No universal accuracy or arithmetic-order equivalence claim.')
retained('serial-number-arithmetic','T41.06.a T41.06.b','legacy_network_tcp','assessor/network_followthrough/tcp/tcp.c labs/network_followthrough/tcp/CONTRACTS.md','Actual modular sequence ordering and half-range ambiguity under bounded windows.','Not unrestricted total order on all serial values.')
for element,objectives,mechanism,limits in [
 ('abstract-syntax-tree','T15.14.b T15.14.c','Actual typed precedence tree, distinct lexical/syntax/semantic failures and unchanged node-admission failure.','A syntax tree is not a persistent semantic environment.'),
 ('lexer','T15.14.a','Actual token kinds/values/source offsets and copied-spelling lifetime alternative.','General longest-token rollback is assessed separately in X-T15-020.'),
 ('recursive-descent-parser','T15.14.b','Actual recursive precedence parsing with explicit depth/node bounds and right-associative transfer.','The separate Pratt/SLR/shunting mechanisms are not inferred from this parser.'),
 ('tree-walking-interpreter','T15.14.c','Actual recursive typed-AST evaluation with checked arithmetic and pure name callback.','Persistent REPL/environment is assessed separately in X-T15-021.')]:
 retained(element,objectives,'legacy_algorithms','assessor/algorithm_depth/parser.c research/v5/expansion/legacy_algorithms/bindings.py',mechanism,limits)
retained('symbol-table','T46.05.a T46.05.b','legacy_vm_compiler','assessor/runtime_followthrough/compiler/compiler.c labs/runtime_followthrough/compiler/CONTRACTS.md','Actual name/scope resolution, declaration/assignment shadowing and reclaimed lexical slots.','Selected language and finite storage; not inferred from a generic hash table.')
retained('parser-combinator','','coroutines','assessor/coroutines/coroutines.c assessor/coroutines/followthrough/parser_follow.c labs/coroutines/followthrough/CONTRACTS.md','Actual sequence/choice/repetition with rollback, progress, cut and farthest diagnostics.','Selected combinator graph/budget, not arbitrary effectful parser composition.','X-T72-006')
retained('packrat-parsing','','coroutines','assessor/coroutines/coroutines.c assessor/coroutines/followthrough/parser_follow.c labs/coroutines/followthrough/CONTRACTS.md','Actual success/failure memoization keyed by input/environment identity and direct-left-recursion seed growth.','Direct selected recursion is not arbitrary mutual left-recursion support.','X-T72-007')

screen=read(HERE/'screening_map.json');expected={r['id'] for r in screen['elements'] if r['metadata_description_read']}
assert len(expected)==84 and set(rows)==expected,(expected-set(rows),set(rows)-expected)
for row in rows.values():
 for path in row['evidence']:assert (ROOT/path).is_file(),path
 row['evidence_sha256']={n:sha(ROOT/n) for n in row['evidence']}
 row['dimensions']=dict(teaching='specified',implementation='actual bounded mechanism',author_validation='provider-specific evidence; not rerun by this mapping',external_review='not established by this mapping',independent_learner='not established by this mapping',physical_observation='not established by this mapping')
out=dict(schema_version=1,fixed_description_count=84,scope='Exact previously read84 design descriptions;999 other elements remain metadata routing only. No whole-library or whole-book passage-inspection claim.',input_metadata_sha256=screen['inputs'],resolutions=[dict(id=k,**v) for k,v in sorted(rows.items())],limitations=['Mechanism reuse is justified by representation/operation/invariant/transfer evidence, not a heading or title.','Discovery named_in leads are not the same as actually inspected primary references; provider records retain exact gaps and substitutes.','Finite descriptions bound this discovery screen, not the entire curriculum objective map or user corpus scope; remaining assigned sources and genuine new evidence retain the global gap-ledger process.'])
(HERE/'mechanism_resolutions.json').write_text(json.dumps(out,indent=2)+'\n',encoding='utf8')
print('PASS84 concrete bounded mechanism resolutions')
