"""Bind T67.02 signed rational requirements to exact author observations."""
import json
from common import ROOT,dump,read_json,sha
from check_adt_rational_units import REPORT,DIAGNOSTICS,check
READINESS='research/v5/modern/adt_rational_units_readiness.json'
def main():
    if not (ROOT/REPORT).exists():return
    d,q=read_json(ROOT/REPORT),read_json(ROOT/DIAGNOSTICS);check(d,q)
    brief='curriculum/v5/exercises/X-T67-002.md';m=json.loads((ROOT/brief).read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0])
    teaching=[brief,'labs/adt/CONTRACTS.md','labs/adt/rational_units/RATIONAL.md','assessor/adt/rational_units/ANALYSIS.md']
    implementation=['assessor/adt/adt.c','assessor/adt/rational_units/rational_units.c','labs/adt/rational_units/rational_units.h','labs/adt/rational_units/tests.c','tools/verify_adt_rational_units.py']+['labs/adt/rational_units/probes/'+s for s in ['nominal.c','typedefs.c','duration.cpp']]
    claims=[
        'Actual GCC/Clang positive/wrong-type probes distinguish nominal wrappers from identical typedef aliases. Explicit field extraction and reconstruction compile and demonstrate the enforcement boundary. The original safe seconds domain is checked through the unchanged actual ad_milliseconds provider.',
        'The complete quotient/remainder argument bounds every product and sum without a type wider than64bits. Positive and negative limits differ; signed INT64_MIN is handled through unsigned magnitude and explicit final constant. Inspected duration ratio formulas are compared only on admitted actual C++ examples, never by executing signed overflow.',
        'Six profiles execute the original wrong-scale/missing-range controls and four successor remainder/precision/minimum/direction defects with normal semantic assertions. Original quotient boundaries, null/zero ratio admission and unchanged value+flag outputs are checked; independent13900 exact Python cases include10178 successful,741 invalid and2981 range results.',
        'Signed microseconds and rational tick periods implement toward-zero conversion with explicit inexact flag, including unreduced ratios, INT64_MIN and a fitting result whose naive intermediate product exceeds64bits. Seven paired100000-operation raw/wrapper samples per profile have identical checksum33334; actual stack/branch/dependency evidence qualifies timing and ABI claims.'
    ]
    inputs={**d['input_sha256'],**q['input_sha256']};criteria=[];updates=[]
    for r,claim in zip(m['rubric'],claims):criteria.append({**r,'author_readiness':'ready-bounded','demonstration':claim,'teaching':teaching,'implementation':implementation,'validation':[REPORT,DIAGNOSTICS],'independent_learner':'not-assessed','external_review':'not-run','physical_observation':'not-applicable-host-contract'})
    for identity in ['X-T67-002','X-T67-002-DELAYED','T67-02-TRANSFER','T67-02-INVESTIGATION']+[r['id'] for r in m['rubric']]:
        claim=next((r['demonstration'] for r in criteria if r['id']==identity),claims[3] if identity.endswith(('DELAYED','TRANSFER')) else ' '.join(claims))
        updates.append(dict(id=identity,dimensions={'teaching':dict(state='complete',evidence=teaching),'implementation':dict(state='complete',evidence=implementation),'author_validation':dict(state='passed',evidence=[REPORT,DIAGNOSTICS,READINESS]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed',evidence=[]),'physical_observation':dict(state='not-applicable-host-contract',evidence=[teaching[2]])},closure_rationale=claim+' This is exact bounded author support, not independent learner or physical-clock evidence.'))
    path='research/v5/modern/ad_mp_gap_ledger.json';g=read_json(ROOT/path)
    for row in g['gaps']:
        if row['id'] in ['T67-02-TRANSFER','T67-02-INVESTIGATION']:row.update(status='author-support-implemented-and-source-validated; independent-learner-not-assessed',author_evidence=[READINESS,REPORT,DIAGNOSTICS],remaining_external=['Unaided delayed learner attempt and per-objective assessment','Non-author review; physical timing not observed'])
    dump(ROOT/path,g)
    for p in teaching+implementation+[REPORT,DIAGNOSTICS,'tools/build_adt_rational_units_readiness.py','tools/check_adt_rational_units.py']:inputs[p]=sha(ROOT/p)
    dump(ROOT/READINESS,dict(schema_version=1,domain='T67',criteria=criteria,variants=[dict(id='X-T67-002-DELAYED',author_support='implemented-and-author-validated',evidence=teaching+implementation+[REPORT,DIAGNOSTICS],independent_learner='not-assessed')],current_reports={p:sha(ROOT/p) for p in [REPORT,DIAGNOSTICS]},input_sha256=inputs,limits=['Published C17/C23 remain access gaps; N1570 is explicitly a C11 draft.','No physical clocks, arbitrary workload cost, independent learner result or non-author review inferred.']))
    inputs[READINESS]=sha(ROOT/READINESS);dump(ROOT/'research/v5/modern/adt_rational_units_completion_dispositions.json',dict(schema_version=1,updates=updates,current_reports=[REPORT,DIAGNOSTICS],input_sha256=inputs));print(json.dumps(dict(passed=True,criteria=4,updates=8)))
if __name__=='__main__':main()
