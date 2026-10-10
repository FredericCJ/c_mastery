"""Explicit partial mechanism mapping, not an objective acceptance shortcut."""
import json
from pathlib import Path
root=Path(__file__).resolve().parents[2]
rows=[
('PR-T14-01',['ring'],'Bounded caller-owned ring only; array/view interfaces and other capacities require project evidence.'),
('PR-T14-04',['hash'],'Fixed 16-slot tombstone table; dynamic resizing and generic ownership remain project work.'),
('PR-T14-05',['sort','avl'],'Heap repair and fixed-pool ordered AVL tree; complete heap container API remains project work.'),
('PR-T14-06',['avl'],'Insertion/deletion/rotations/reuse with independent structural checks; mathematical height proof and transfer remain project work.'),
('PR-T14-08',['graph'],'Union-find and bounded matrix graph; alternate graph representations and ownership remain project work.'),
('PR-T15-01',['sort'],'Unsigned lower bound and stable insertion/merge; the remaining comparison-sort suite is not implemented here.'),
('PR-T15-04',['match'],'Length-delimited naive matcher only; alternative algorithm and cost argument remain project work.'),
('PR-T15-05',['graph'],'BFS and checked nonnegative Dijkstra; DFS, DAG ordering and other path algorithms remain project work.'),
('PR-T16-01',['lifetime'],'Aligned byte offsets only; effective type and typed-object design require project reasoning.'),
('PR-T16-02',['lifetime'],'Mark/rewind byte arena; cross-arena and stale mark misuse remain caller preconditions.'),
('PR-T16-03',['lifetime'],'Typed int32 pool with generation handles and retirement; generalized typed ownership remains project work.'),
('PR-T16-09',['lifetime'],'Capacity, alignment, huge request and stale handles only; not general allocator adversarial coverage.'),
('PR-T17-01',['numeric'],'Bounded compensated sum fixtures; conditioning and error-bound derivation remain project work.'),
('PR-T17-02',['numeric'],'Bounded Q7 MAC and exhaustive single-product rounding; no generalized scale conversion API.'),
('PR-T36-01',['ring','sort'],'Ring sequence and sorted permutation evidence; other representation/cost ledgers remain project work.'),
('PR-T36-02',['x86_64','thumb'],'Original lower-bound kernels and checked boundaries plus ABI probes; written invariants and address/cost certificates remain required.'),
('PR-T36-03',['sort','x86_64','thumb'],'Original stable tagged insertion in both ISAs; C counters distinguish shifts/placements, instruction work needs learner disassembly evidence.'),
('PR-T36-04',['sort','x86_64','thumb'],'Original sift-down in both ISAs; reused C facade checks heap premise, no new heap container supplied.'),
('PR-T36-05',['avl'],'Structural mutation checks and fixed-pool AVL only; path cost certificates remain learner evidence.'),
('PR-T36-06',['hash','match'],'Tombstone and byte-span mechanisms; callback/context abstraction design remains learner work.'),
('PR-T36-07',['graph'],'Bounded matrix frontier and checked distances; generic frontier/allocator adapters remain learner work.'),
('PR-T37-01',['models'],'Exhaustive Boolean four-bit addition, carry and signed overflow; broader ALU controls remain project work.'),
('PR-T37-05',['models'],'Declared direct-map/shadow model only; not measurements of actual host or MCU caches.'),
('PR-T37-07',['x86_64','thumb'],'Scalar function ABI only; reset and exception frames are not accepted by these tests.'),
('PR-T37-08',['sort'],'Correct kernel prerequisite plus separately invoked nine-sample hosted CSV; cycles and calibrated native comparison remain project work.'),
('PR-T43-01',['sort'],'Declared comparisons/moves counters for reused kernels; complete cost contract and proof remain learner work.'),
('PR-T43-05',['models'],'Fixed cache trace only; store traffic and device I/O evidence belong to their actual adapters.'),
('PR-T43-08',['sort'],'Reusable correct workload and optional timing CSV; paired interventions, placement and statistical interpretation remain learner work.'),
]
stage_tests={
 'PR-T14-05':['heap'], 'PR-T14-08':['sets'],
 'PR-T16-01':['arena'], 'PR-T16-02':['arena'], 'PR-T16-03':['pool'],
 'PR-T17-01':['sum'], 'PR-T17-02':['q7'],
 'PR-T36-02':['x86_64_lower','thumb_lower'],
 'PR-T36-03':['sort','x86_64_insert','thumb_insert'],
 'PR-T36-04':['heap','x86_64_sift','thumb_sift'],
 'PR-T37-01':['alu'], 'PR-T37-05':['cache'], 'PR-T43-05':['cache']}
rows=[(pid,stage_tests.get(pid,tests),scope) for pid,tests,scope in rows]
rows=[(pid,tests,
 'Heap repair only; the complete heap container API and ordered tree remain project work.' if pid=='PR-T14-05' else
 'Bounded union-find only; graph storage, alternate representations and ownership remain project work.' if pid=='PR-T14-08' else scope)
 for pid,tests,scope in rows]
family={'schema_version':1,'id':'algorithms','modules':['T14','T15','T16','T17','T36','T37','T43'],
 'language':'C17 plus original SysV x86-64 and Thumb assembly','build_system':'cmake',
 'default':'Compiling learner stubs; semantic tests fail','assessor_option':'CMASTERY_ASSESSOR_ROOT',
 'assessor_directory':'assessor/algorithms','acceptance_scope':'Finite mechanism evidence, never automatic objective or full-project acceptance.',
 'projects':[{'id':pid,'tests':['algorithms.'+t for t in tests],
 'capabilities':['hosted-c17']+(['linux-x86_64','arm-none-eabi-gcc','qemu-cortex-m4'] if any(t.startswith('thumb') for t in tests) else []),
 'scope':scope}for pid,tests,scope in rows],
 'tests':['algorithms.'+t for t in ['ring','hash','avl','sets','graph','sort','heap','match','arena','pool','lifetime','sum','q7','numeric','alu','cache','models','x86_64','x86_64_lower','x86_64_insert','x86_64_sift','thumb','thumb_lower','thumb_insert','thumb_sift']],
 'negative_controls':['algorithms.mutation_'+x for x in ['tombstone','stability','generation','asm_SIGNED','asm_UNSTABLE','asm_ABI','thumb_SIGNED','thumb_UNSTABLE','thumb_ABI']],
 'optional_measurement':'algorithms_measure emits raw wall/process CPU observations without timing thresholds',
 'not_covered':['Dynamic containers and advanced structures','General-purpose allocator families','Matrix/filter/ODE sequence','Pipeline and instruction decoder models','Physical MCU, DMA, PMU/cgroup access and calibrated cycle claims']}
(root/'labs/algorithms/family.json').write_text(json.dumps(family,indent=2)+'\n',encoding='utf-8')
