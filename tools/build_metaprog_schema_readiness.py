"""Bind original T68.06 criteria to source-current schema evolution evidence."""
import json
from common import ROOT,dump,read_json,sha
from check_metaprog_schema import REPORT,COST,check
READINESS='research/v5/modern/metaprog_schema_readiness.json'

def main():
    d,q=read_json(ROOT/REPORT),read_json(ROOT/COST);check(d,q)
    brief='curriculum/v5/exercises/X-T68-006.md';m=json.loads((ROOT/brief).read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);teaching=[brief,'labs/metaprog/CONTRACTS.md','labs/metaprog/schema/SCHEMA.md','assessor/metaprog/schema/ANALYSIS.md'];implementation=['assessor/metaprog/metaprog.c','assessor/metaprog/schema/schema.c','assessor/metaprog/schema/schema_fields.h','labs/metaprog/schema/include/schema.h','labs/metaprog/schema/schema_stamp.py','labs/metaprog/schema/tests/tests.c','tools/verify_metaprog_schema.py','labs/api_depth/architecture/arch.h','assessor/api_depth/architecture/core.c']
    claims=[
      'One trusted field registry derives native declarations, names/offsets/sizes, typed access, descriptors, validation, presence-aware equality, explicit tagged wire and ordered visitors. Actual codec.records ar_encode/ar_decode version2 handles the optional signed field. Retained six-octet registry producer and MOD11/12 controls are actually executed; no native padding is serialized.',
      'Inspected N1570 layout, substitution, stringification, token-concatenation and offsetof passages distinguish C11 draft rules from unavailable published C17/C23. Actual Rust2021 macro_rules plus derived PartialEq includes a new field, with no C ABI/hygiene or arbitrary-reflection equivalence. Complete-input hash words and observed linker symbol significance are separate build enforcement, not ISO language introspection or authentication.',
      'Six profiles pass4anchors and8compiled ordinary semantic controls;4compiling stubs reject. Every profile matches9811 independent encode/decode/visitor cases, including1390accepted and40callback-stop outcomes. Seven10000-encode samples, real object/stack/helper costs and clean/unchanged build observations retain exact contracts. GCC/Clang consistently packed native layouts change offsets while preserving independently specified wire bytes.',
      'Required quality field transfer adds tag4,width2,since3,range0..15 and exercises generated decode/equality/validation/visitor plus version2-to3 migration and refusal to downgrade a present field. An old handwritten equality still compiles and demonstrably misses quality. Actual stale full-input check, four-word compile assertion and old-object link guards reject; regenerated consumers execute. Generated compiler header bytes and recipe are explicitly verified and archived rather than bypassing dependency closure. Delayed independent learner evidence remains unassessed.'
    ]
    criteria=[{**r,'author_readiness':'ready-bounded','demonstration':claim,'teaching':teaching,'implementation':implementation,'validation':[REPORT,COST],'independent_learner':'not-assessed','external_review':'not-run','physical_observation':'not-applicable-hosted-schema'} for r,claim in zip(m['rubric'],claims)]
    updates=[]
    for identity in ['X-T68-006','X-T68-006-DELAYED','T68-06-TRANSFER','T68-06-INVESTIGATION']+[r['id'] for r in m['rubric']]:
        claim=next((r['demonstration'] for r in criteria if r['id']==identity),claims[3] if identity.endswith(('DELAYED','TRANSFER')) else ' '.join(claims))
        updates.append(dict(id=identity,dimensions={'teaching':dict(state='complete',evidence=teaching),'implementation':dict(state='complete',evidence=implementation),'author_validation':dict(state='passed',evidence=[REPORT,COST,READINESS]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed',evidence=[]),'physical_observation':dict(state='not-applicable-hosted-schema',evidence=[teaching[2]])},closure_rationale=claim+' Bounded author support only.'))
    g=read_json(ROOT/'research/v5/modern/ad_mp_gap_ledger.json')
    for row in g['gaps']:
        if row['id'] in ['T68-06-TRANSFER','T68-06-INVESTIGATION']:row.update(status='author-support-implemented-and-source-validated; independent-learner-not-assessed',author_evidence=[READINESS,REPORT,COST],remaining_external=['Unaided delayed learner attempt and per-objective assessment','Non-author assessment','Exact published C17/C23 access gaps remain'])
    dump(ROOT/'research/v5/modern/ad_mp_gap_ledger.json',g)
    inputs={**d['input_sha256'],**q['input_sha256']}
    for p in teaching+implementation+[REPORT,COST,'tools/build_metaprog_schema_readiness.py','tools/check_metaprog_schema.py']:inputs[p]=sha(ROOT/p)
    dump(ROOT/READINESS,dict(schema_version=1,domain='T68',criteria=criteria,variants=[dict(id='X-T68-006-DELAYED',author_support='implemented-and-author-validated',evidence=teaching+implementation+[REPORT,COST],independent_learner='not-assessed')],current_reports={p:sha(ROOT/p) for p in [REPORT,COST]},input_sha256=inputs,limits=['Trusted declared schema constraints and live disjoint externally serialized native objects are premises.','Changed packed objects are consistently recompiled; stale ABI object is never executed.','Hosted compiler copy/zero helpers are observed, not a strict freestanding claim.','Fingerprint detects accidental staleness, not malicious build control or authentication.','Published C17/C23 inaccessible; independent learner and human review unperformed.']))
    inputs[READINESS]=sha(ROOT/READINESS);dump(ROOT/'research/v5/modern/metaprog_schema_completion_dispositions.json',dict(schema_version=1,updates=updates,current_reports=[REPORT,COST],input_sha256=inputs));print(json.dumps(dict(passed=True,criteria=4,updates=8)))
if __name__=='__main__':main()
