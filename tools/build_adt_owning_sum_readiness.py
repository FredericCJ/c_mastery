"""Exact T67.01 author-support closure; preserve independent learner obligations."""
import json
from common import ROOT,dump,read_json,sha
from check_adt_owning_sum import REPORT,DIAGNOSTICS,check
READINESS='research/v5/modern/adt_owning_sum_readiness.json'

def main():
    if not (ROOT/REPORT).exists():return
    d,q=read_json(ROOT/REPORT),read_json(ROOT/DIAGNOSTICS);check(d,q)
    brief='curriculum/v5/exercises/X-T67-001.md';m=json.loads((ROOT/brief).read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0])
    teaching=[brief,'labs/adt/CONTRACTS.md','labs/adt/owning_sum/OWNING_SUM.md','assessor/adt/owning_sum/ANALYSIS.md']
    implementation=['assessor/adt/adt.c','assessor/adt/owning_sum/owning_sum.c','labs/adt/owning_sum/owning_sum.h','labs/adt/owning_sum/tests.c','labs/adt/owning_sum/test_support.h','tools/verify_adt_owning_sum.py','labs/adt/owning_sum/probes/evolution.c','labs/adt/owning_sum/probes/evolution.rs']
    claims=[
        'The assessor derives1+2^32+2^64 plus the finite byte-sequence sum. Separate active-member selection, value semantics and release authority are implemented. Actual GCC/Clang default/missing-enumerator controls show diagnostics depend on flags; actual Rust exhaustive/wildcard probes distinguish language checking from a logical default-path omission.',
        'Actual retained pair elimination promotes before addition and preserves scalar output on wrong tag. Every old operation has an explicit TEXT migration rule; borrowed source bytes are copied before mutation and a source-policy clone replaces a destination-policy owner only after success. Text length, embedded zero and empty-text-versus-EMPTY are separate states.',
        'Six native profiles execute the two actual retained sum mutants plus six owning controls, all normal semantic rejections. Enum-size/runtime and compiler branch/stack/dependency observations are source-bound; fourteen diagnostic observations include before/after positive clients and E0004. Independent10512 histories/56384 transitions compare values and owner counts.',
        'Fallible clone/drop callbacks allocate actual storage and inject both preallocation and postallocation-cleanup failure. Old destination survives, including an input suffix borrowing its payload; self-copy/move call no callback, moves empty the source, policy transfer drops the old context once, and repeated drop is harmless. Changed clients that fail diagnostics and clients preserving a default path are both executed.'
    ]
    criteria=[];updates=[];inputs={**d['input_sha256'],**q['input_sha256']}
    for r,claim in zip(m['rubric'],claims):criteria.append({**r,'author_readiness':'ready-bounded','demonstration':claim,'teaching':teaching,'implementation':implementation,'validation':[REPORT,DIAGNOSTICS],'independent_learner':'not-assessed','external_review':'not-run','physical_observation':'not-applicable-host-contract'})
    for identity in ['X-T67-001','X-T67-001-DELAYED','T67-01-TRANSFER','T67-01-INVESTIGATION']+[r['id'] for r in m['rubric']]:
        claim=next((r['demonstration'] for r in criteria if r['id']==identity),claims[3] if identity.endswith(('DELAYED','TRANSFER')) else ' '.join(claims))
        updates.append(dict(id=identity,dimensions={'teaching':dict(state='complete',evidence=teaching),'implementation':dict(state='complete',evidence=implementation),'author_validation':dict(state='passed',evidence=[REPORT,DIAGNOSTICS,READINESS]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed',evidence=[]),'physical_observation':dict(state='not-applicable-host-contract',evidence=[teaching[2]])},closure_rationale=claim+' This is author support for the stated finite callback contract, not acceptance of independent learner work.'))
    ledger='research/v5/modern/ad_mp_gap_ledger.json';g=read_json(ROOT/ledger)
    for row in g['gaps']:
        if row['id'] in ['T67-01-TRANSFER','T67-01-INVESTIGATION']:row.update(status='author-support-implemented-and-source-validated; independent-learner-not-assessed',author_evidence=[READINESS,REPORT,DIAGNOSTICS],remaining_external=['Unaided delayed learner result and objective-specific assessment','Non-author review; no physical claim'])
    dump(ROOT/ledger,g)
    # The shared ledger is intentionally not pinned as an executable dependency:
    # another ADT mechanism may independently advance its own original rows.
    for path in teaching+implementation+[REPORT,DIAGNOSTICS,'tools/build_adt_owning_sum_readiness.py','tools/check_adt_owning_sum.py']:inputs[path]=sha(ROOT/path)
    value=dict(schema_version=1,domain='T67',criteria=criteria,variants=[dict(id='X-T67-001-DELAYED',author_support='implemented-and-author-validated',evidence=teaching+implementation+[REPORT,DIAGNOSTICS],independent_learner='not-assessed')],current_reports={p:sha(ROOT/p) for p in [REPORT,DIAGNOSTICS]},input_sha256=inputs,limits=['Published C17/C23 access gaps remain; no inspected draft is relabeled.','No unaided learner, non-author reviewer or physical evidence. Callback contract violations are not sandboxed by C.'])
    dump(ROOT/READINESS,value);inputs[READINESS]=sha(ROOT/READINESS)
    dump(ROOT/'research/v5/modern/adt_owning_sum_completion_dispositions.json',dict(schema_version=1,updates=updates,current_reports=[REPORT,DIAGNOSTICS],input_sha256=inputs))
    print(json.dumps(dict(passed=True,criteria=4,updates=8)))
if __name__=='__main__':main()
