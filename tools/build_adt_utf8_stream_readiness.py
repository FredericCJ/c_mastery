"""Bind original T67.04 objectives to exact incremental/owner observations."""
import json
from common import ROOT,dump,read_json,sha
from check_adt_utf8_stream import REPORT,DIAGNOSTICS,check
READINESS='research/v5/modern/adt_utf8_stream_readiness.json'
def main():
    if not (ROOT/REPORT).exists():return
    d,q=read_json(ROOT/REPORT),read_json(ROOT/DIAGNOSTICS);check(d,q)
    brief='curriculum/v5/exercises/X-T67-004.md';m=json.loads((ROOT/brief).read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0])
    teaching=[brief,'labs/adt/CONTRACTS.md','labs/adt/unicode_stream/UTF8.md','labs/adt/owning_sum/OWNING_SUM.md','assessor/adt/unicode_stream/ANALYSIS.md']
    implementation=['assessor/adt/adt.c','assessor/adt/owning_sum/owning_sum.c','assessor/adt/unicode_stream/unicode_stream.c','labs/adt/unicode_stream/unicode_stream.h','labs/adt/unicode_stream/tests.c','labs/adt/unicode_stream/probes/borrow.rs','tools/verify_adt_utf8_stream.py']
    claims=[
        'Inspected RFC3629 scalar/byte constraints are derived separately from backing lifetime and higher-level text claims. Actual live C alias mutation invalidates borrowed encoding without dangling access; composed/decomposed valid examples have distinct scalar counts. Rust positive last-use/owned-copy probes and E0506 live-borrow rejection establish only their declared language boundary.',
        'The original complete validator is actually imported. Incremental state retains at most three copied octets, counts only completed scalars and transactionally publishes a whole chunk; invalid suffix, impossible prefix, null/length admission, finalization and cancellation have explicit unchanged-state or clearing semantics. Owner composition imports actual owning_sum clone-before-drop with exact pre/post-allocation failure preservation.',
        'Six profiles compare233265 cases with independent Python strict codecs and all17652 possible scalar prefixes derived from1112064 encodings. Every0/1/2-byte string has all two-chunk splits, with named boundary partitions and second-byte domains. Eight compiled normal assertion controls reject original overlong/surrogate and successor state/owner defects; observed ED A0 error-timing difference is documented instead of misreported as equivalent codec semantics.',
        'All boundary splits, copied-pending mutation, truncated finalization, cancellation/restart, complete65535-byte limit, copied-owner source mutation, self-borrowing replacement and clone failures are implemented and tested. Four compiling learner anchors reject; seven10000-stream measurements/profile retain checksum30000 and actual fixed pending storage/stack/dependency observations. No asynchronous cancellation, normalization, universal cost or learner mastery claim is made.'
    ]
    inputs={**d['input_sha256'],**q['input_sha256']};criteria=[];updates=[]
    for r,claim in zip(m['rubric'],claims):criteria.append({**r,'author_readiness':'ready-bounded','demonstration':claim,'teaching':teaching,'implementation':implementation,'validation':[REPORT,DIAGNOSTICS],'independent_learner':'not-assessed','external_review':'not-run','physical_observation':'not-applicable-host-contract'})
    for identity in ['X-T67-004','X-T67-004-DELAYED','T67-04-TRANSFER','T67-04-INVESTIGATION']+[r['id'] for r in m['rubric']]:
        claim=next((r['demonstration'] for r in criteria if r['id']==identity),claims[3] if identity.endswith(('DELAYED','TRANSFER')) else ' '.join(claims))
        updates.append(dict(id=identity,dimensions={'teaching':dict(state='complete',evidence=teaching),'implementation':dict(state='complete',evidence=implementation),'author_validation':dict(state='passed',evidence=[REPORT,DIAGNOSTICS,READINESS]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed',evidence=[]),'physical_observation':dict(state='not-applicable-host-contract',evidence=[teaching[2]])},closure_rationale=claim+' This is exact bounded author support; delayed independent learner and non-author review remain unobserved.'))
    path='research/v5/modern/ad_mp_gap_ledger.json';g=read_json(ROOT/path)
    for row in g['gaps']:
        if row['id'] in ['T67-04-TRANSFER','T67-04-INVESTIGATION']:row.update(status='author-support-implemented-and-source-validated; independent-learner-not-assessed',author_evidence=[READINESS,REPORT,DIAGNOSTICS],remaining_external=['Unaided delayed learner attempt and per-objective assessment','Non-author review; no physical or external text-service evidence'])
    dump(ROOT/path,g)
    for p in teaching+implementation+[REPORT,DIAGNOSTICS,'tools/build_adt_utf8_stream_readiness.py','tools/check_adt_utf8_stream.py']:inputs[p]=sha(ROOT/p)
    dump(ROOT/READINESS,dict(schema_version=1,domain='T67',criteria=criteria,variants=[dict(id='X-T67-004-DELAYED',author_support='implemented-and-author-validated',evidence=teaching+implementation+[REPORT,DIAGNOSTICS],independent_learner='not-assessed')],current_reports={p:sha(ROOT/p) for p in [REPORT,DIAGNOSTICS]},input_sha256=inputs,limits=['Whole-chunk refusal and early impossible-prefix rejection are explicit policies, not claims that all incremental libraries behave identically.','No normalization/grapheme correctness, asynchronous cancellation, published C17/C23 inspection, physical execution, independent learner result or non-author review inferred.']))
    inputs[READINESS]=sha(ROOT/READINESS);dump(ROOT/'research/v5/modern/adt_utf8_stream_completion_dispositions.json',dict(schema_version=1,updates=updates,current_reports=[REPORT,DIAGNOSTICS],input_sha256=inputs));print(json.dumps(dict(passed=True,criteria=4,updates=8)))
if __name__=='__main__':main()
