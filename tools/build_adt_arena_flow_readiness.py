"""Exact original arena and cyclic-analysis obligations; no inferred learner pass."""
import json
from common import ROOT,dump,read_json,sha
from check_adt_arena_flow import REPORT,COST,check
READINESS='research/v5/modern/adt_arena_flow_readiness.json'

def main():
    if not (ROOT/REPORT).exists():return
    d,q=read_json(ROOT/REPORT),read_json(ROOT/COST);check(d,q)
    brief='curriculum/v5/exercises/X-T67-007.md';m=json.loads((ROOT/brief).read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);teaching=[brief,'labs/adt/CONTRACTS.md','labs/adt/arena_flow/FLOW.md','assessor/adt/arena_flow/ANALYSIS.md'];implementation=['assessor/adt/adt.c','assessor/adt/arena_flow/arena_flow.c','labs/adt/arena_flow/arena_flow.h','labs/adt/arena_flow/tests.c','tools/verify_adt_arena_flow.py']
    claims=[
      'Actual retained ad_eval validates all copied nodes before an owned arena certificate publishes. Prior child indices establish acyclicity; an unmodified inline certificate permits unchecked internal traversal, while typed indices alone do not establish range or identity. Original input mutation cannot invalidate the copy; public certificate mutation remains an explicit caller violation.',
      'Index induction establishes cached value equivalence and representable additions after whole validation. Construction is O(n), roots O(1), fixed scratch is observed through compiler stack output, and actual recursive expansion of a16-node shared-child graph visits65535 nodes. Seven samples compare10000 retained validations against construction plus10000 cached roots under the stated immutable-input premise; clean/unchanged build and hosted helper observations remain separate from freestanding claims.',
      'Six profiles independently compare9117 whole-arena cases,37123 synchronous-equation flow cases and22066 exhaustive Boolean cases. Empty/maximum/over-capacity, every root, shared children, unreachable malformed/overflow nodes and original forward-reference/overflow controls are present. Eight compiled semantic controls per profile fail ordinary assertions, and four compiling learner anchors reject. Compiler dependency closures and current source/report hashes bind these bounded author observations.',
      'Actual copied cyclic sessions resume one-pop budgets to the least fixed point; the reverse-ring changed variant mutates its original input and reaches independently derived masks241,227,199,143,31,62,124,248. The finite monotone n+8E bound is justified, BUDGET leaves result unchanged while retaining progress, and complete Boolean search separately proves self-NOT unsatisfiable. Mutual-NOT oscillation still has fixed assignments, falsifying nonconvergence-as-nonexistence. All thirteen cost/enforcement rows are addressed without claiming arbitrary-language analysis soundness.'
    ]
    inputs={**d['input_sha256'],**q['input_sha256']};criteria=[];updates=[]
    for r,claim in zip(m['rubric'],claims):criteria.append({**r,'author_readiness':'ready-bounded','demonstration':claim,'teaching':teaching,'implementation':implementation,'validation':[REPORT,COST],'independent_learner':'not-assessed','external_review':'not-run','physical_observation':'not-applicable-host-contract'})
    for identity in ['X-T67-007','X-T67-007-DELAYED','T67-07-TRANSFER','T67-07-INVESTIGATION']+[r['id'] for r in m['rubric']]:
        claim=next((r['demonstration'] for r in criteria if r['id']==identity),claims[3] if identity.endswith(('DELAYED','TRANSFER')) else ' '.join(claims))
        updates.append(dict(id=identity,dimensions={'teaching':dict(state='complete',evidence=teaching),'implementation':dict(state='complete',evidence=implementation),'author_validation':dict(state='passed',evidence=[REPORT,COST,READINESS]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed',evidence=[]),'physical_observation':dict(state='not-applicable-host-contract',evidence=[teaching[2]])},closure_rationale=claim+' Bounded author support; no unaided delayed learner result or non-author assessment inferred.'))
    path='research/v5/modern/ad_mp_gap_ledger.json';g=read_json(ROOT/path)
    for row in g['gaps']:
        if row['id'] in ['T67-07-TRANSFER','T67-07-INVESTIGATION']:row.update(status='author-support-implemented-and-source-validated; independent-learner-not-assessed',author_evidence=[READINESS,REPORT,COST],remaining_external=['Unaided delayed learner attempt and per-objective assessment','Non-author review; arbitrary-language analysis soundness not established','Exact published C17/C23 text remains inaccessible'])
    dump(ROOT/path,g)
    for p in teaching+implementation+[REPORT,COST,'tools/build_adt_arena_flow_readiness.py','tools/check_adt_arena_flow.py']:inputs[p]=sha(ROOT/p)
    dump(ROOT/READINESS,dict(schema_version=1,domain='T67',criteria=criteria,variants=[dict(id='X-T67-007-DELAYED',author_support='implemented-and-author-validated',evidence=teaching+implementation+[REPORT,COST],independent_learner='not-assessed')],current_reports={p:sha(ROOT/p) for p in [REPORT,COST]},input_sha256=inputs,limits=['Finite monotone copied fact model is distinct from the arbitrary Boolean search and unbounded mathematical counterexample.','Hosted compiler helpers are measured; no strict-freestanding, arbitrary-language soundness or physical-target claim.','No published C17/C23 inspection, independent learner result or non-author review inferred.']))
    inputs[READINESS]=sha(ROOT/READINESS);dump(ROOT/'research/v5/modern/adt_arena_flow_completion_dispositions.json',dict(schema_version=1,updates=updates,current_reports=[REPORT,COST],input_sha256=inputs));print(json.dumps(dict(passed=True,criteria=4,updates=8)))
if __name__=='__main__':main()
