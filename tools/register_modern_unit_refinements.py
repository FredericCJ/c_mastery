"""Attach already-authored cross-topic assessments to their owning learner units.

This changes neither exercise criteria nor implementation evidence. Historical
unit identities/minutes and project hours remain descriptive baseline records.
The later exercise routes retain their own prerequisite and assessment gates.
"""
import argparse
import copy
import json
from pathlib import Path
import sys

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief

REFINEMENTS={
 'object_runtime':dict(unit='LAB-OR-12',exercise='X-T74-013',
   prerequisites='First study PR-T74-03 and the original X-T74-004 cache contract. The required deferred X-T74-013 integration is assessed only after X-T74-003, X-T74-004 and X-T74-008 artifacts exist; it does not introduce a backwards PR-T74-08 -> PR-T74-04 project prerequisite.',
   steps=[
    'Required later integration X-T74-013: derive ordered shape keys/attributes and descriptor generations; conserve one descriptor reference for each live receiver, including an empty receiver. Keep field values separate from immutable layout.',
    'Stage replacement fields and acquire the replacement shape before dropping the old shape. Demonstrate deletion, readonly rejection, independent descriptor retirement and acyclic prototype admission using the actual struct orx_heap contract.',
    'At the later X-T74-013 assessment, retain temporary receiver/holder roots through a mutating accessor, then validate weak-cache epoch/generations. Do not award these criteria for the original object_runtime.cache anchor alone.'],
   checks=[
    'For X-T74-013 R1-R3, compare the 3600 independent deletion/shadowing/readonly lookups, descriptor-reference counts and generation retirement; reject the readonly and cycle-admission mutants with the stated harmless local counterexamples.',
    'For X-T74-013 R4-R6, retain two same-layout holders, switch only the receiver prototype, and reject the missing-epoch control. An accessor that installs key1=99, unlinks, collects and then fails preserves output777 while its admitted side effects remain; temporary roots return to their prior counts.'],
   reasoning='Explain why shape and object generations retire independently; prototype reachability is a strong-edge property, while a weak cache does not keep either object or shape alive. Accessor failure cannot roll back already committed callback side effects.',
   deliverable='Submit X-T74-013 D1-D4, all six exact criteria and its delayed accessor variant as the required later integration dossier. Preserve original LAB-OR-12 evidence separately; no current learner pass or elapsed delayed assessment is inferred.',
   variant='After X-T74-008, assess X-T74-013 with an accessor that consumes its owned roots, changes the prototype and returns failure; require the exact output, root-count and persistent-side-effect arguments in the separate assessor analysis.'),
 'replicated':dict(unit='LAB-RP-14',exercise='X-T76-010',
   prerequisites='Retain the original X-T76-007 hashing/Merkle artifact and PR-T14-04 collision/tombstone-table knowledge. Complete X-T76-010 through its distinct corpus_ring workspace and explicit prior struct rph_sha provider; a hash-table lookup is not a ring placement operation.',
   steps=[
    'Required X-T76-010 transfer: derive token identity (owner,ordinal), ordered position collisions and clockwise successor lookup. Prove insertion monotonicity only while all preexisting tokens remain fixed.',
    'Implement adjacent-view migration plans and distinct physical replica selection in the separate corpus_ring exercise. Stage every output before publication and retain the exact view/epoch and digest-byte-order premises.',
    'Use the actual prior SHA component for byte-key routing, then compare ordered interval lookup with the independent clockwise-distance oracle. Explain why placement, availability, membership authority and durable anti-entropy remain different contracts.'],
   checks=[
    'Old tokens (10,1,0),(30,2,0), adding (20,3,0),(40,3,1) moves exactly keys11..20 and31..40 among65536 keys. Reseeding an old token invalidates this monotonicity premise; equal positions use owner/ordinal ties.',
    'With repeated virtual nodes of owner1, requesting three replicas returns distinct physical owners[1,2,3]. A down mask, all-coincident tokens, stale epoch and undersized migration output must produce the exact declared result or leave outputs unchanged.',
    'Run the distinct X-T76-010 corpus_ring.lookup/membership/replicas/routing anchors, causal controls and independent oracle through that exercise route; replicated.merkle alone does not exercise these criteria.'],
   reasoning='Hash-table collision handling from T14.07 transfers to a different representation: ring collision ties and fixed-token placement. Derive bounded chosen-token load capture and show why monotonicity does not imply balanced load, authenticated membership or a completed data handoff.',
   deliverable='Submit X-T76-010 D1-D4 and R1-R4 plus the changed-view delayed dossier. Preserve the original Merkle proof/transport deliverables; ring planning supplements rather than replaces them.',
   variant='Assess X-T76-010-DELAYED after its minimum delay: remove owner1 from epoch7, derive the five moved keys and degraded replica refusal, then falsify stale-view and capacity publication claims using the supplied original analysis.'),
}


def refine(metadata,family):
    spec=REFINEMENTS[family]
    brief,_=parse_brief(ROOT/'curriculum/v5/exercises'/(spec['exercise']+'.md'))
    result=copy.deepcopy(metadata)
    units=[u for u in result['units'] if u['id']==spec['unit']]
    assert len(units)==1 and units[0]['project']==brief['project']
    unit=units[0]
    def extend(key,values):
        for value in values:
            if value not in unit[key]:unit[key].append(copy.deepcopy(value))
    extend('objectives',brief['objectives'])
    for row in brief['rubric']:
        existing=[r for r in unit['rubric'] if r['id']==row['id']]
        assert not existing or existing==[row],row['id']
    extend('rubric',brief['rubric'])
    extend('steps',[spec['prerequisites'],*spec['steps']])
    extend('checks',spec['checks'])
    extend('reasoning',[spec['reasoning']])
    extend('deliverables',[spec['deliverable']])
    extend('variants',[spec['variant']])
    for source in brief['source_records']:
        for locator in source['passages']:
            row=dict(resource_id=source['id'],locator=locator,
                     purpose='Exact inspected source for required '+brief['id']+' cross-topic assessment; original source limits remain.')
            if not any(r['resource_id']==row['resource_id'] and r['locator']==locator for r in unit['references']):unit['references'].append(row)
    stage=dict(exercise=brief['id'],prerequisite_exercises=brief['prerequisite_exercises'],
               objectives=brief['objectives'],criteria=[r['id'] for r in brief['rubric']],
               family=brief['lab']['family'],tests=brief['lab']['tests'],
               assessor_analysis=brief['assessor_analysis'],prerequisite_policy=spec['prerequisites'],
               accounting='Required additive exercise scope; original unit minutes/project hours are preserved historical/descriptive values, not a ceiling.',
               assessment='Use the separate exercise prerequisite and rubric gates; original unit test passage cannot discharge these later criteria.')
    unit.setdefault('required_exercise_assessments',[])
    if stage not in unit['required_exercise_assessments']:unit['required_exercise_assessments'].append(stage)
    return result


def register_family(family):
    path=ROOT/'labs'/family/'family.json'
    old=json.loads(path.read_text(encoding='utf8'));new=refine(old,family)
    assert [(u['id'],u['minutes']) for u in old['units']]==[(u['id'],u['minutes']) for u in new['units']]
    assert refine(new,family)==new,'Refinement must be idempotent'
    path.write_text(json.dumps(new,indent=2)+'\n',encoding='utf8')
    return REFINEMENTS[family]['unit']


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('families',nargs='*',choices=list(REFINEMENTS))
    args=parser.parse_args()
    for family in args.families or REFINEMENTS:
        print(family+': '+register_family(family))
