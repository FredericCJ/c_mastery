"""Required scene/corridor refinements with bounded actual primary inspections."""
import copy
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
BASE='research/v5/expansion/corpus_scene'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def dump(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
SOURCES=[
 ('DIRTY','Game Programming Patterns: Dirty Flag',['Robert Nystrom'],2014,'Online chapter','https://gameprogrammingpatterns.com/dirty-flag.html','dirty.html',['Deferred recalculation and The Pattern: source versus derived state and cached work','Sample Code: unoptimized traversal, dirty propagation and direct-access limitation']),
 ('FUNNEL','Simple Stupid Funnel Algorithm',['Mikko Mononen'],2010,'8March2010 author article','https://digestingduck.blogspot.com/2010/03/simple-stupid-funnel-algorithm.html','funnel.html',['Opening explanation: narrowed rays, crossed side and restart at emitted corner','stringPull and portal assembly: endpoint equality, signed orientation, start/end portals']),
 ('GLTF','glTF2.0 Specification',['Khronos Group'],2017,'2.0 current online specification, selected stable hierarchy clauses','https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html',None,['3.5.2 Nodes and Hierarchy: disjoint strict trees, zero or one parent','3.5.3 Transformations: parent-global times local, TRS order and no local shear'])]
PACKETS=[
 ('X-T14-011','PR-T14-09','T14',['T14.14.b','T14.15.a','T14.15.b'],'Scene-graph transforms, invalidation and reparenting',1,['DIRTY','GLTF'],
  ['Derive the forest, matrix convention, signed64 product bound and descendant invalidation invariant.',
   'Implement typed slots, cached ancestor composition, leaf deletion and staged keep-local/keep-world reparenting.',
   'Reject wrong order and missing descendant invalidation; preserve all cache/output state on overflow and invalid topology.',
   'Transfer to3D shear/reflection and restricted exact inverse; account for cache work, copied staging and finite generation retirement.'],
  ['Change local translation-only data to three-axis reflection/shear and a different parent while preserving local state; compare every world entry to the uncached oracle.',
   'Preserve world under a signed-permutation parent, then use a scaled/singular parent; require exact refusal and unchanged state, not guessed floating inversion.'],
  'Implement a16-slot scene forest. A quarter-turn parent at(10,0,0) and child local(3,0,0) must yield(10,3,0). Edit the parent, query the leaf directly, reparent, remove/reuse, and inject late range overflow. The header and shared contract define every bound and effect.',
  'The local node may be unchanged while every cached world value beneath its parent is stale; multiplication order and identity reuse create independent failures.'),
 ('X-T16-004','PR-T16-08','T16',['T16.13.a','T16.13.b','T16.13.c'],'Convex navigation corridors with request-owned funnel storage',2,['FUNNEL'],
  ['Derive strictly convex polygon admission, shared portal orientation and the funnel invariant; distinguish an ordered corridor from global route selection.',
   'Implement exact-integer funnel routing in actual typed request scratch; publish a copied complete path only after all validation/capacity checks.',
   'Implement independent session identity, cancellation and copied-path lifetime; reject stale tickets and preserve previous control/output on failure.',
   'Transfer rotated/reflected/reversed channels and repeated request reuse; compare independent visibility paths and charge persistent/input/scratch/output regions.'],
  ['Rotate, reflect with repaired winding, scale and reverse the polygon corridor; reconstruct portal ownership and compare shortest length and admitted segments.',
   'Cancel a completed request, overwrite scratch, begin a new generation and exhaust both region budgets; old tickets stay stale while independently copied values survive.'],
  'Given1..8 ordered strictly convex CCW polygons, construct shared portals and the taut point-agent path. The L-channel and1280 changed geometries supply exact workloads. Retain immutable corridor input across requests, use a typed NvScratch per computation, then publish a copied NvSession path under a nonwrapping ticket.',
  'A geometrically valid path can still escape dead scratch or acquire a reused request identity. Corridor lifetime, temporary funnel storage and independently retained output require different region boundaries.')]
def main():
 resources=[]
 for key,title,authors,year,edition,url,payload,locators in SOURCES:
  inspection=dict(date='2026-10-18',method='Actual author HTML downloaded and read; Khronos exact hierarchy/transform clauses read through browser after direct403.',scope='; '.join(locators),limits='Only listed primary passages; no full-book, whole-navigation-library or glTF conformance claim.')
  if payload:
   p=ROOT/'build/scene-sources'/payload;inspection.update(download_path=p.relative_to(ROOT).as_posix(),download_sha256=sha(p))
  resources.append(dict(id='R-CSG-'+key,title=title,authors=authors,year=year,edition=edition,kind='specification' if key=='GLTF' else 'book' if key=='DIRTY' else 'article',url=url,access='Accessible primary passages inspected',domains=['T14','T16'],role='primary',reason='Actual transform/invalidation or portal-routing contract, with explicit original bounded adaptations.',prerequisites=['Tree invariants','Matrix products','Checked arithmetic'],assignable=True,locators=locators,inspection=inspection))
 dump(ROOT/BASE/'resources.json',dict(resources=resources))
 template=json.loads((ROOT/'curriculum/v5/exercises/X-T14-006.md').read_text(encoding='utf8').split('```json\n')[1].split('\n```')[0])
 refs=[]
 for eid,project,domain,objectives,title,case,keys,claims,variants,problem,necessity in PACKETS:
  meta=copy.deepcopy(template);meta.update(id=eid,project=project,domain=domain,title=title,objectives=objectives,prerequisite_exercises=['X-T04-001','X-T03-005'],prerequisite_artifacts=[],lab=dict(family='corpus_scene',scaffold='labs/corpus_scene',tests=[f'corpus_scene.{case}']),fixtures=['fixtures/corpus_scene/cases.json'],shared_contracts=['labs/corpus_scene/CONTRACTS.md'],assessor_analysis='assessor/corpus_scene/ANALYSIS.md')
  meta['environment']['dependencies']='First-principles typed forest, matrix composition, polygon admission, funnel and region ownership. Host independent matrix/visibility oracle only.'
  meta['source_records']=[dict(id=r['id'],path=BASE+'/resources.json',passages=r['locators']) for r in resources if r['id'].removeprefix('R-CSG-') in keys]
  meta['rubric']=[dict(id=eid+f'-R{i+1}',objective=objectives[min(i,len(objectives)-1)],criterion=claim,levels={'0':'Breaks representation, ownership or evidence contract.','1':'Runs the ordinary example without the invariant or changed condition.','2':claim+' Supply concrete source-bound counterexample and verification evidence.','3':'Independently reconstructs both changed conditions at least7days later.'}) for i,claim in enumerate(claims)]
  meta['objective_coverage']=[dict(objective=o,teaching=['#problem','#reasoning'],implementation=['D1','D2'],boundary=dict(applicable=True,evidence=['D3']),independent_assessment=[r['id'] for r in meta['rubric'] if r['objective']==o],assessed_claim='; '.join(r['criterion'] for r in meta['rubric'] if r['objective']==o)) for o in objectives]
  meta['status']['limitations']=['Source-bound receipts distinguish actual execution from authorship.','No rendering, device navigation, physical safety, independent learner or full corpus guarantee.']
  body=f'''# {eid}: {title}

## Prerequisites
Complete checked extent X-T04-001 and typed generation pool X-T03-005 first. Supply those learner invariants and stale-identity counterexamples. For scene work also reconstruct parent/child traversal and3x4 matrix multiplication; for corridor work reconstruct orientation, convexity and T16.13 region-use diagrams. No old assessor implementation is silently imported. All studied operations remain in src/scene.c.

## Environment
Use C17 Profile C under the exact shared contract. Inputs, scratch, output and controls are distinct actual typed objects. GCC/Clang and host Python verify the mechanism; target object/link audits establish only emitted dependency closure.

## Problem
{problem}

## Necessity
{necessity}

## Reasoning
State every pre/postcondition, ownership interval, failure publication and finite bound. Draw the smallest noncommuting or crossing example. Prove which cached information or supporting ray justifies skipping work. Explain what C types and diagnostics establish, what runtime checks enforce, and what live/unmodified/exclusive-control discipline remains external. The shared contract supplies exact statuses, capacities, permitted geometry and counter boundaries.

## Deliverables
D1: hand-derived outputs and mechanism/lifetime proof. D2: complete implementation and structural/ownership traces. D3: independent oracle results, two distinct compiled causal mutants, refusal snapshots and toolchain closure. D4: both actual changed-condition implementations, cost ledger and separately recorded delayed learner evidence. Use the supplied header/tests/fixtures; do not replace the mechanism with a library call or a generic topic essay.

## Verification
Run corpus_scene.{case}, its two semantic controls and the six C17 profiles. The public harness states concrete expected examples; independent Python recomputes all world matrices or visibility-graph paths. Reject missing controls, source changes, compile failures presented as semantic controls, and unsupported physical claims. Equal contracts are required for work comparisons. Test exactly-full and insufficient regions, stale identities and unchanged prior authority after refusal.

## Rubric
Assess R1-R4 independently against the explicit frontmatter criteria. Mere fixture success is level1. Level2 requires explanation, construction, diagnosis and bounded verification; level3 requires independent delayed reconstruction. A reference campaign awards no learner mastery.

## Hints
H1 draw the parent chain or the next two portals and label owners. H2 compute one matrix product or signed area before coding. H3 stage a complete result, then deliberately omit one invalidation/identity/side test and minimize the resulting counterexample.

## Variants
V1: {variants[0]}

V2: {variants[1]}

## Security
The target is the original local array/control implementation using synthetic data only. Bounded adversarial coordinates/capacities and previously issued stale tickets expose wrong-output or ownership violations. Harmless success is one wrong value or admitted stale read; repairs and regressions restore the invariant. This is not an external target, malformed-pointer experiment, physical navigation guarantee or arbitrary code execution claim. See shared security scope for exact limits.

## Validation
Authored, compiled, executed, independently assessed and physically observed are distinct states. Actual current report is reviews/v5/expansion/corpus_scene/current_validation.json. Original Snook2000 and Real-Time Rendering source passages remain uninspected; actual accessible primary substitutes above support only their registered claims. The separate assessor analysis explains the technical solution without leaking it into the learner package. No finite suite proves absolute mastery.
'''
  (ROOT/'curriculum/v5/exercises'/f'{eid}.md').write_text('```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body,encoding='utf8')
  refs.append(dict(exercise=eid,required=True,parent_objectives=objectives,competencies=[dict(id=r['id'],outcome=r['criterion'],source=[s['id'] for s in meta['source_records']]) for r in meta['rubric']]))
 dump(ROOT/BASE/'taxonomy_extensions.json',dict(schema_version=1,kind='required-competency-refinement-of-existing-leaf',scope_rationale='Scene graph is a concrete advanced hierarchical structure. Navigation refines T16 lifetime architecture with actual persistent corridor, typed request scratch and copied session path; geometry is explicitly new required transferable content, not claimed as old allocator coverage.',refinements=refs))
 paths=[BASE+'/resources.json',BASE+'/taxonomy_extensions.json'];dump(ROOT/BASE/'research_gate.json',dict(status='passage-inspected',research_input_sha256={p:sha(ROOT/p) for p in paths},limitations='Author passage inspection only; no published C17/C23 or unavailable original book access invented.'))
 dump(ROOT/BASE/'audit.json',dict(status='confirmed-required-mechanisms-authored',inspected_inputs=['curriculum/modules/T16.md','curriculum/projects/PR-T16-08.md','curriculum/projects/PR-T14-09.md','research/v5/expansion/corpus_followthrough/screening_map.json'],mapping={'scene-graph':'X-T14-011','navmesh-convex-corridor-funnel':'X-T16-004'},reference_gaps=['Snook2000 original chapter not accessed.','Real-Time Rendering3/4 original transform passages not accessed.'],limits='No renderer, mesh generation, global optimal corridor selector, finite-radius clearance or arbitrary3D projection implementation is implied.'))
 print('Authored scene and navigation lifetime refinements')
if __name__=='__main__':main()
