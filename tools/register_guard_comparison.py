"""Register bounded actual upstream inspection and a reasoning companion."""
import json
from common import ROOT, dump, read_json, sha

BASE = 'research/v5/expansion/guard_comparison'
PUBLIC = 'labs/guard_comparison/CONTRACTS.md'
PRIVATE = 'assessor/guard_comparison/ANALYSIS.md'

def generate():
    access = read_json(ROOT/'build/guard-comparison-primary/access.json')
    resources = []
    for name, rid, locators in [
        ('guard.rs', 'R-GUARD-CROSSBEAM-0918', ['Guard representation and pinning/nested guards', 'defer/defer_unchecked/defer_destroy safety and progress', 'flush, repin, repin_after, collector, Drop and unprotected']),
        ('atomic.rs', 'R-GUARD-ATOMIC-0918', ['Atomic::load lines339–357 and its Shared guard lifetime', 'Owned::into_shared lines1094–1109; raw conversion and Shared representation boundary'])]:
        capture = next(x for x in access['captures'] if x['path'].endswith('/'+name))
        assert capture['status'] == 200 and sha(ROOT/capture['path']) == capture['sha256']
        resources.append(dict(id=rid, title='Crossbeam epoch '+name+' guard interface', authors=['Crossbeam contributors'],
            edition='crossbeam-epoch-0.9.18 tagged upstream source', year=2024, url=capture['url'], kind='source', role='primary',
            access='Actual named source text inspected via primary upstream repository', domains=['T55'], assignable=True,
            reason='Precisely bounded lifetime, unsafe precondition and reclamation comparison',
            prerequisites=['C lifetime and epoch retirement invariants'], locators=locators,
            inspection=dict(date='2026-10-18', method='Actual source passage read; normal authorized HTTPS capture', scope=locators,
                capture=capture, limits='No whole collector audit, Rust execution or published ISO text inspection')))
    dump(ROOT/BASE/'resources.json', dict(schema_version=1, resources=resources))
    record = dict(id='SE-EPOCH-ENFORCEMENT-01', exercise='X-T55-003', domain='T55', objectives=['T55.07.a','T55.07.b','T55.07.c'],
        required=True, family=['A','D','F','I','J'], contract=PUBLIC, assessor=PRIVATE,
        work='Five exact logical histories and two delayed changed-condition analyses; no actual dangling access',
        completion='Evaluate each objective independently using the explicit 0–3 assessor matrix; source-bound author reasoning is not a learner result',
        status='authored-and-source-inspected; independent performance pending')
    child = dict(id='MBR-SOURCE-GUARD', parent='MBR-SOURCE', required=True,
        work='Resolve inaccessible Crossbeam Guard details against an identified tagged source',
        implemented='Guard and selected Atomic lifetime passages actually inspected; bounded comparison and assessor histories authored',
        limits='Exact published C17 remains open under original parent; no entire collector verification')
    dump(ROOT/BASE/'remaining_work.json', dict(schema_version=1, required_work=[record,child],
        external_evidence=['Non-author review', 'Actual independent ordinary and delayed learner attempts']))
    ledger = read_json(ROOT/'curriculum/v5/expansion/completion_ledger.json')
    register = BASE+'/remaining_work.json'
    if register not in ledger['required_gap_registers']:
        ledger['required_gap_registers'].append(register)
        dump(ROOT/'curriculum/v5/expansion/completion_ledger.json', ledger)
    evidence=[PUBLIC,PRIVATE,BASE+'/resources.json']
    dimensions=dict(teaching=dict(state='complete-bounded-source-comparison',evidence=evidence),
        implementation=dict(state='complete-reasoning-only-package',evidence=[PUBLIC,PRIVATE]),
        author_validation=dict(state='source-inspected-and-worked-analysis;not-executable-experiment',evidence=evidence),
        external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),
        physical_observation=dict(state='not-applicable-to-reasoning-companion',evidence=[]))
    paths=[PUBLIC,PRIVATE,BASE+'/resources.json',register,'tools/register_guard_comparison.py','tools/check_guard_comparison.py']
    paths += [x['path'] for x in access['captures']]
    dump(ROOT/BASE/'completion_dispositions.json',dict(schema_version=1,
        source_sha256={p:sha(ROOT/p) for p in paths},updates=[dict(id=i,dimensions=dimensions,
        closure_rationale='Named upstream passages resolve Guard-detail access; explicit histories assess enforcement and ownership without claiming implementation equivalence.',
        required_environment='C17 reasoning and identified Crossbeam0.9.18 source; no target runtime dependency',
        validation_method='Inspect named tagged source and independently derive each logical history; evaluate objective-specific rubric',
        evidence_limits=['No Rust compilation, collector execution, published C17 or independent learner claim']) for i in [record['id'],child['id']]]))
    print('Registered bounded guard source child and reasoning companion')

if __name__ == '__main__': generate()
