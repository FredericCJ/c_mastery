"""Map exact T67.03 flag obligations to current, bounded author observations."""
import json
from common import ROOT,dump,read_json,sha
from check_adt_flags_negotiation import REPORT,EXHAUSTIVE,check
READINESS='research/v5/modern/adt_flags_negotiation_readiness.json'
def main():
    if not (ROOT/REPORT).exists():return
    d,q=read_json(ROOT/REPORT),read_json(ROOT/EXHAUSTIVE);check(d,q)
    brief='curriculum/v5/exercises/X-T67-003.md';m=json.loads((ROOT/brief).read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0])
    teaching=[brief,'labs/adt/CONTRACTS.md','labs/adt/flags_negotiation/NEGOTIATION.md','assessor/adt/flags_negotiation/ANALYSIS.md']
    implementation=['assessor/adt/adt.c','assessor/adt/flags_negotiation/flags_negotiation.c','labs/adt/flags_negotiation/flags_negotiation.h','labs/adt/flags_negotiation/tests.c','labs/adt/flags_negotiation/exhaustive.c','tools/verify_adt_flags_negotiation.py']
    claims=[
        'Set/sum cardinality and per-bit reasoning distinguish eight original states from exclusive variants. Runtime revalidation rejects forged operands, while versioned negotiation preserves unknown advisory data without treating it as support. A local counter witnesses only unsupported feature admission by capability-union control.',
        'Actual original constructor is reused by validated union/intersection and version1 capabilities. All64 operand pairs and512 associativity triples are executed; separate draft clause analysis covers promotions including the wider-int model. Failed negotiation and constructors preserve every result field, with no ownership or callback state.',
        'Actual GCC and Clang separate-TU release programs each call the original constructor for all4294967296 masks:8 accepted,4294967288 unchanged refusals,checksum28. Both original controls compile and reject normally at raw8/raw0. Six general profiles also reject all six original/extended controls and compare283808 independent set-valued cases per profile.',
        'Required/advisory offers are negotiated across version1 mask7 and version2 mask63, with intersection authority, required-union admission and exact advisory preservation. Actual forwarding retains advisory32 through an old peer, activates it only after both support it, and refuses making it required before support exists. Seven100000-operation samples per profile retain exact checksum350000 and measured ABI/stack/assembly bounds.'
    ]
    inputs={**d['input_sha256'],**q['input_sha256']};criteria=[];updates=[]
    for r,claim in zip(m['rubric'],claims):criteria.append({**r,'author_readiness':'ready-bounded','demonstration':claim,'teaching':teaching,'implementation':implementation,'validation':[REPORT,EXHAUSTIVE],'independent_learner':'not-assessed','external_review':'not-run','physical_observation':'not-applicable-host-contract'})
    for identity in ['X-T67-003','X-T67-003-DELAYED','T67-03-TRANSFER','T67-03-INVESTIGATION']+[r['id'] for r in m['rubric']]:
        claim=next((r['demonstration'] for r in criteria if r['id']==identity),claims[3] if identity.endswith(('DELAYED','TRANSFER')) else ' '.join(claims))
        updates.append(dict(id=identity,dimensions={'teaching':dict(state='complete',evidence=teaching),'implementation':dict(state='complete',evidence=implementation),'author_validation':dict(state='passed',evidence=[REPORT,EXHAUSTIVE,READINESS]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed',evidence=[]),'physical_observation':dict(state='not-applicable-host-contract',evidence=[teaching[2]])},closure_rationale=claim+' This is bounded author support, not independent learner or external protocol security evidence.'))
    path='research/v5/modern/ad_mp_gap_ledger.json';g=read_json(ROOT/path)
    for row in g['gaps']:
        if row['id'] in ['T67-03-TRANSFER','T67-03-INVESTIGATION']:row.update(status='author-support-implemented-and-source-validated; independent-learner-not-assessed',author_evidence=[READINESS,REPORT,EXHAUSTIVE],remaining_external=['Unaided delayed learner attempt and per-objective assessment','Non-author review; no physical or external service evidence'])
    dump(ROOT/path,g)
    for p in teaching+implementation+[REPORT,EXHAUSTIVE,'tools/build_adt_flags_negotiation_readiness.py','tools/check_adt_flags_negotiation.py']:inputs[p]=sha(ROOT/p)
    dump(ROOT/READINESS,dict(schema_version=1,domain='T67',criteria=criteria,variants=[dict(id='X-T67-003-DELAYED',author_support='implemented-and-author-validated',evidence=teaching+implementation+[REPORT,EXHAUSTIVE],independent_learner='not-assessed')],current_reports={p:sha(ROOT/p) for p in [REPORT,EXHAUSTIVE]},input_sha256=inputs,limits=['Only original finite raw-mask constructor was exhaustively enumerated; negotiation has a separate finite model and proof obligation.','No published C17/C23 inspection, physical execution, external peer authentication, independent learner result or non-author review inferred.']))
    inputs[READINESS]=sha(ROOT/READINESS);dump(ROOT/'research/v5/modern/adt_flags_negotiation_completion_dispositions.json',dict(schema_version=1,updates=updates,current_reports=[REPORT,EXHAUSTIVE],input_sha256=inputs));print(json.dumps(dict(passed=True,criteria=4,updates=8)))
if __name__=='__main__':main()
