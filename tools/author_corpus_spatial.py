"""Canonical spatial refinement, primary passage register and five complete exercises."""
import hashlib
import json
import pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
BASE='research/v5/expansion/corpus_spatial'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def dump(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')

SOURCES=[
 ('BENTLEY','Multidimensional Binary Search Trees Used for Associative Searching',['Jon Louis Bentley'],1975,'Communications of the ACM18(9),509-517','https://cs.wmich.edu/gupta/teaching/cs6310/lectureNotes_cs6310/kdtree-bentley.pdf','bentley.pdf',['Section2 printed510: discriminator, equal-key ordering and subtree bounds','Section4 printed512-514: region pruning and nearest-neighbor assumptions']),
 ('PBRT','Physically Based Rendering: Bounding Volume Hierarchies',['Matt Pharr','Wenzel Jakob','Greg Humphreys'],2023,'Fourth edition section7.3','https://pbr-book.org/4ed/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies',None,['Section7.3 introduction: primitive subdivision, conservative bounds and2n-1 nodes','Section7.3.1 construction and equal-count partition: child unions, centroids and leaf admission']),
 ('QUAD','The Skip Quadtree: A Simple Dynamic Data Structure for Multidimensional Data',['David Eppstein','Michael T. Goodrich','Jonathan Z. Sun'],2005,'arXiv cs/0507049v1,19July2005','https://arxiv.org/pdf/cs/0507049','skip-quadtree.pdf',['Pages1-2 introduction: region quad/octrees, arbitrary depth and dimensionality','Page3 section2: half-open square ownership and standard versus compressed quadtree']),
 ('GUTTMAN','R-Trees: A Dynamic Index Structure for Spatial Searching',['Antonin Guttman'],1984,'ACM SIGMOD1984,47-57','https://www.cs.princeton.edu/courses/archive/fall08/cos597B/papers/rtrees.pdf','guttman.pdf',['Printed48-50 sections2-3.3: occupancy, overlapping search, ChooseLeaf and split propagation','Printed50-52 sections3.3-3.5.2: CondenseTree, updates and quadratic split algorithms']),
 ('HASH','Optimized Spatial Hashing for Collision Detection of Deformable Objects',['Matthias Teschner','Bruno Heidelberger','Matthias Mueller','Danat Pomeranets','Markus Gross'],2003,'VMV2003 selected sections3-4.1','https://cgl.ethz.ch/Downloads/Publications/Papers/2003/Tes03/Tes03.pdf','teschner.pdf',['PDFpage3 section3: floor discretization, all overlapped cells and final geometry test','PDFpage3 section4.1: finite hash indexing and collisions']),
 ('PARTITION','Game Programming Patterns: Spatial Partition',['Robert Nystrom'],2014,'Online Spatial Partition chapter','https://gameprogrammingpatterns.com/spatial-partition.html','partition.html',['Intent and motivation: nearby-object queries and all-pairs baseline','Neighbor-cell boundary discussion and Design Decisions: flat/hierarchical, movement and duplicate collections'])
]
PACKETS=[
 (6,'k-d tree: exact nearest queries with weighted ties','KD: partition, lower bounds and deterministic ties',['BENTLEY'],
  'Build a median point tree with axis cycling, closed subtree boxes and deterministic ID ties. Implement range and weighted nearest queries plus staged insertion, move and deletion. Use the supplied five-point duplicate fixture and32-item mutation workloads.',
  'A scalar ordering does not establish multidimensional pruning. Equal lower bounds and metric changes expose plausible false-negative optimizations.',
  [('Partition and lower-bound proof','Derive coordinate/ID partition, exact subtree bounds and positive weighted distance bound.'),('Construction and updates','Implement build/query/nearest and staged rebuild mutation; distinguish balance from dynamic logarithmic update.'),('Tie and refusal falsification','Reject equal-bound pruning and unweighted-metric mutants; preserve every refused output.'),('Changed metric and costs','Reconstruct three-dimensional weights and compare visits against an independent linear scan without a universal speed claim.')],
  ['V1: change2D Euclidean to3D weighted squared distance with weights1..4; rederive the bound and reproduce the exact changed winner.', 'V2: compare duplicate/collinear clouds and full-capacity move/delete/reinsert; predict node visits and storage refusal before executing.']),
 (7,'Bounding-volume hierarchy with conservative refit',['BVH: conservative enclosure, refit and rebuilding'][0],['PBRT'],
  'Construct a binary leaf-owned box hierarchy, then move ID0 to(1024,1024) using topology-preserving refit. Compare range/nearest answers and work against rebuilding the same objects, including face contact and zero-volume boxes.',
  'Bounding boxes may overlap and grow after movement. A stale ancestor can hide a valid leaf even while every leaf remains locally correct.',
  [('Conservative hierarchy proof','Derive2n-1 nodes, exact child unions and closed-boundary pruning.'),('Build and refit','Implement median-center build, copied ownership and bottom-up refit without altering child links.'),('Contact and stale ancestor diagnosis','Kill contact-exclusion and skipped-refit mutants; retain minimal moved-box evidence.'),('Changed geometry and costs','Compare refit with rebuild in3D and distinguish box candidates from physical shape intersection.')],
  ['V1: move separated boxes into an overlapping cluster, refit without changing topology, then rebuild and compare exact answers plus visits.', 'V2: transfer to3D degenerate slabs and nearest point-to-box distance; supply a bounding-box false-contact counterexample.']),
 (8,'Point-region quadtree and octree with finite duplicate buckets','Quad and octree: finite subdivision and duplicate buckets',['QUAD'],
  'Implement sparse midpoint subdivision over[-16,16)^d, preserving IDs of coincident points in unit cells. Query closed ranges, inject node-budget exhaustion midway and update by staged rebuilding. The changed implementation runs in3D.',
  'A point count alone does not bound continuous subdivision depth. Boundary ownership and coincident points must be resolved before promising termination.',
  [('Ownership and termination','Derive half-open child ownership and depth5 for the chosen integer lattice.'),('Sparse construction and updates','Implement quad and octree children, unit duplicate buckets and transactional rebuild.'),('Boundary and capacity diagnosis','Kill omitted-low-child and dropped-duplicate controls; test-16,15 and rejected16.'),('Dimensional transfer','Reconstruct eight-child ownership, sparse space use and exact exhausted budget with independent queries.')],
  ['V1: change dimension2 to3 with midpoint-plane points and coincident IDs; derive child bits and depth independent of n.', 'V2: reduce node budget to one below the measured requirement after a valid old index; prove old queries survive refusal and explain why continuous floating coordinates need a new contract.']),
 (9,'R-tree split propagation and deletion condensation','R-tree: overlapping branches, quadratic split and deletion repair',['GUTTMAN'],
  'Implement M4/m2 R-tree insertion, quadratic split, ancestor bounds, root growth and removal with orphan reinsertion/root collapse. Reinsert moved boxes and search all overlapping branches. Exercise32 IDs through removal to empty and3D updates.',
  'Spatial pages have overlapping regions, so scalar B-tree single-path lookup and adjacency assumptions are unsound. Underflow repair must preserve every orphaned object.',
  [('Occupancy and search proof','Derive minimum/maximum entries, equal leaf depth, exact bounds and all-branch search.'),('Split and deletion construction','Implement recursive split propagation and staged flatten/reinsert condensation, documenting its difference from original level-preserving reinsertion.'),('Lost branch and orphan diagnosis','Kill first-branch-only and dropped-orphan mutants; refuse capacity without publishing a partial split.'),('Changed dimensional updates','Use3D overlapping boxes, root collapse and volume ties; account for worst-case traversal and reinsertion costs.')],
  ['V1: delete enough entries to collapse the root, then reinsert changed3D boxes; verify equal leaf depth after each step.', 'V2: use zero-volume and mutually overlapping boxes with an exactly insufficient node budget; compare every result to a scan and explain why page balance does not guarantee narrow spatial search.']),
 (10,'Signed spatial hash grid with exact candidate ownership','Spatial hash grid and the broader partition pattern',['HASH','PARTITION'],
  'Map each closed box to every touched width1..16 cell, hash full signed coordinates, deduplicate all overlap candidates and apply the final box test. Compare occupied-reference queries against BVH for the same input; update atomically through bounded scratch.',
  'Truncation differs from floor for negative coordinates. Hash identity, cell identity, repeated pair identity and true overlap are four different claims.',
  [('Cell and candidate proof','Derive signed floor, closed endpoint mapping, full-cell equality and deduplicated pair completeness.'),('Construction and ownership','Implement bounded reference lists and complete preflight for every replicated box; preserve old index on exhaustion.'),('Negative coordinates and duplicates','Kill truncation and repeated-pair controls; demonstrate same-cell disjoint boxes and collisions without false contact.'),('Partition comparison and transfer','Change width and dimension, compare replication/visits with BVH under identical geometry and explain large-object resource refusal.')],
  ['V1: change width2 to1 and4 across negative cell boundaries in3D; predict reference counts and unique candidate pairs.', 'V2: admit a large closed box under coarse cells, refuse it under the256-reference limit with finer cells, and compare unchanged queries with a leaf-owned BVH.'])
]

def main():
    resources=[]
    for key,title,authors,year,edition,url,payload,locators in SOURCES:
        inspection=dict(date='2026-10-18',method='Actual primary PDF/HTML passage reading; PBRT read through browser response after direct download403.',scope='; '.join(locators),limits='Only exact listed passages; original bounded adaptations are not whole-book or whole-paper implementations.')
        if payload:
            p=ROOT/'build/spatial-sources'/payload
            inspection.update(download_path=p.relative_to(ROOT).as_posix(),download_sha256=sha(p))
        resources.append(dict(id='R-CSP-'+key,title=title,authors=authors,year=year,edition=edition,kind='book' if key in ['PBRT','PARTITION'] else 'paper',url=url,access='Accessible primary passages inspected',domains=['T14'],role='primary',reason='Precise geometric representation and query/update assumptions for required spatial refinement.',prerequisites=['Tree invariants','Checked arithmetic'],assignable=True,locators=locators,inspection=inspection))
    dump(ROOT/BASE/'resources.json',dict(resources=resources))
    refinements=[]
    for offset,title,heading,keys,problem,why,criteria,variants in PACKETS:
        eid=f'X-T14-{offset:03}';kind=offset-5;objectives=['T14.14.b','T14.15.a','T14.15.b']
        rubric=[]
        for i,(name,claim) in enumerate(criteria,1):
            objective=['T14.15.a','T14.14.b','T14.15.b','T14.14.b'][i-1]
            rubric.append(dict(id=eid+f'-R{i}',objective=objective,criterion=name+': '+claim,levels={'0':'Violates the geometric/storage property or claims unsupported completeness.','1':'Passes one example without the proof, causal control or costs.','2':claim+' Supply source-bound evidence and stated limitations.','3':'Independently reconstruct and falsify the changed conditions after the recorded delay.'}))
        source_records=[dict(id=r['id'],path=BASE+'/resources.json',passages=r['locators']) for r in resources if r['id'][6:] in keys]
        assert len(source_records)==len(keys)
        meta=dict(schema_version=1,id=eid,title=title,project='PR-T14-09',domain='T14',objectives=objectives,families=list('ABCDFGHIJ'),required=True,prerequisite_exercises=['X-T04-001'],prerequisite_artifacts=[],profile='C',environment=dict(language='ISO C17; exact published C17/C23 source access remains separately limited',target='Hosted Linux, fixed-capacity portable core; Cortex-M4 object/link audit separate',toolchain=['GCC','Clang','CMake','Python3','Arm GCC'],dependencies='Original first-principles geometry, sorting, nodes and update algorithms; host oracle and diagnostics are verification-only'),source_records=source_records,lab=dict(family='corpus_spatial',scaffold='labs/corpus_spatial',tests=[f'corpus_spatial.{kind}']),fixtures=['fixtures/corpus_spatial/cases.json'],shared_contracts=['labs/corpus_spatial/CONTRACTS.md'],assessor_analysis='assessor/corpus_spatial/ANALYSIS.md',status=dict(content='authored',execution='see-source-bound-campaign',independent_review='not-run',learner_assessment='not-assessed',limitations=['Current source-bound campaign determines actual execution; authoring alone is not validation.','No physical geometry, independent learner or universal complexity claim.']),rubric=rubric,objective_coverage=[dict(objective=o,teaching=['#problem','#reasoning'],implementation=['D1','D2'],boundary=dict(applicable=True,evidence=['D3']),independent_assessment=[r['id'] for r in rubric if r['objective']==o],assessed_claim='; '.join(r['criterion'] for r in rubric if r['objective']==o)) for o in objectives],delayed_transfer=dict(minimum_days=7,variant_ids=['V1','V2']))
        body=f'''# {eid}: {title}

## Prerequisites
Complete X-T04-001 checked arithmetic and bring your PR-T14 tree/hash and invariants dossier. Supply your own typed-array sorting and traversal; no algorithm library or hidden assessor dependency is permitted. Existing accepted IDs/hours and PR-T14-09 historical elective status remain unchanged; this expansion is required.

## Environment
Use C17 Profile C and the exact public spatial.h API. Native verification may use host diagnostics/Python. Strict Cortex-M4 object/link auditing is separately labeled and cannot establish board execution. The shared contract defines live/disjoint typed storage, external serialization and fixed coordinate/ID bounds.

## Problem
{problem} The supplied header, tests and fixtures define concrete inputs/outputs; implement the `{heading}` section of the shared contract in src/spatial.c. Other anchors are distinct mechanisms and do not substitute for this one.

## Necessity
{why}

## Reasoning
Derive the representation invariant, every admitted mutation's pre/postconditions, unchanged refusal publication and geometric pruning proof. Explain why copied IDs do not grant pointer stability. State complexity in n/nodes/references and separately the fixed32-object experiment. Identify exactly which properties C enforces, which checks run at runtime and which valid-control/lifetime premises require caller discipline.

## Deliverables
D1: hand prediction and invariant/lower-bound or candidate-completeness proof. D2: actual construction/update/query code and structural traces. D3: independent oracle results, two compiled causal controls, boundaries/refusal snapshots and source/toolchain closure. D4: both changed-condition variants, costs and delayed independent evidence. The shared contract gives the required workload and observable results; the separate assessor analysis explains the technical solution.

## Verification
Run `corpus_spatial.{kind}` and its two compiled mutation controls under all six C17 profiles. Compare actual query output with the independent scalar Python oracle. Verify the representation after every removal, insertion and move, including full capacity and deletion to empty. Target audits must report helper closure and code/stack evidence without calling it execution. A failed compilation, sanitizer termination or heading mention is not a passing causal control.

## Rubric
Assess each of R1-R4 separately: proof, implementation, falsification, then transfer/cost. Passing the ordinary fixture alone is level1. Level3 requires independently reconstructed changed assumptions at least7days later; no author reference campaign awards that level.

## Hints
H1 draw a smallest boundary/overlap case and its owner. H2 inspect what conservative information a skipped subtree/cell actually certifies. H3 stage the entire mutation before committing, then remove one invariant repair to find the first divergent query.

## Variants
{variants[0]}

{variants[1]}

## Security
Use only original synthetic coordinates in the local harness. Adversarial layouts exercise resource exhaustion, duplicate identities and false-negative queries; they are not an external-service exploit or authority claim. Do not execute malformed pointers or forge internal object graphs; those violate the explicitly admitted API domain.

## Validation
Authored reference, compiling learner stubs, independent oracle and causal controls are separate from actually executed receipts. Consult reviews/v5/expansion/corpus_spatial/current_validation.json and its current source pins. Source gaps include uninspected Finkel/Bentley1974 and Real-Time Rendering2018 passages; the listed accessible primary substitutes are bounded and accurately identified. Finite evidence does not prove absolute mastery or application-level geometry safety.
'''
        (ROOT/'curriculum/v5/exercises'/f'{eid}.md').write_text('```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body,encoding='utf8')
        refinements.append(dict(exercise=eid,required=True,parent_objectives=objectives,competencies=[dict(id=r['id'],outcome=r['criterion'],source=[s['id'] for s in source_records]) for r in rubric]))
    dump(ROOT/BASE/'taxonomy_extensions.json',dict(schema_version=1,kind='required-competency-refinement-of-existing-leaf',scope_rationale='Five distinct spatial mechanisms absent from retained implementation; quad/oct dimensional transfer shares one exercise, and spatial-partition is assessed by explicit grid/hierarchy comparison rather than a duplicate sixth implementation.',refinements=refinements))
    gatepaths=[BASE+'/resources.json',BASE+'/taxonomy_extensions.json']
    dump(ROOT/BASE/'research_gate.json',dict(status='passage-inspected',research_input_sha256={p:sha(ROOT/p) for p in gatepaths},limitations='Author bounded passage inspection only; execution and independent assessment are separate.'))
    dump(ROOT/BASE/'audit.json',dict(status='confirmed-missing-mechanisms-authored',actual_sources_inspected=['curriculum/modules/T14.md','curriculum/projects/PR-T14-09.md','assessor/algorithms/algo.c','research/v5/expansion/corpus_followthrough/screening_map.json'],search_scope='Exact mechanism tokens in current labs/assessor C headers and sources; no implementations found. Existing scalar trees/hash tables do not establish spatial-query coverage.',mapping={'k-d-tree':'X-T14-006','bounding-volume-hierarchy':'X-T14-007','quadtree-octree':'X-T14-008','r-tree':'X-T14-009','uniform-spatial-hash-grid':'X-T14-010','spatial-partition':'X-T14-010 plus hierarchy comparison across006-009'},reference_gaps=['Finkel/Bentley1974 original paper was not accessed; metadata is not inspection.','Samet1984 primary PDF URL returned404; no source-inspection credit.','Real-Time Rendering2018 original source not inspected; actual PBRT4ed section7.3 is the bounded primary substitute.'],limits='No exhaustion of all research structures or arbitrary application requirements is inferred.'))
    print('Authored five spatial mechanism exercises and explicit grouped-topic audit')
if __name__=='__main__':main()
