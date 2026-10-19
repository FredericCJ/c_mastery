"""Bind exact original T67.05 criteria and changed conditions to observed support."""
import json
from common import ROOT,dump,read_json,sha
from check_adt_relocating_pool import REPORT,DIAGNOSTICS,check
READINESS='research/v5/modern/adt_relocating_pool_readiness.json'
def main():
    if not (ROOT/REPORT).exists():return
    d,q=read_json(ROOT/REPORT),read_json(ROOT/DIAGNOSTICS);check(d,q)
    brief='curriculum/v5/exercises/X-T67-005.md';m=json.loads((ROOT/brief).read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0])
    teaching=[brief,'labs/adt/CONTRACTS.md','labs/adt/relocating_pool/POOL.md','assessor/adt/relocating_pool/ANALYSIS.md']
    implementation=['assessor/adt/adt.c','assessor/adt/relocating_pool/relocating_pool.c','labs/adt/relocating_pool/relocating_pool.h','labs/adt/relocating_pool/tests.c','labs/adt/relocating_pool/probes/category.c','labs/adt/relocating_pool/probes/category.rs','tools/verify_adt_relocating_pool.py']
    claims=[
      'Actual GCC/Clang distinct-category refusals and copy/public-reconstruction positives separate nominal category from ownership and runtime identity. Rust1.93 PhantomData named-marker rejection/positive construction and zero marker size are observed without claiming generative per-runtime-pool branding.',
      'Actual retained ADPool implements fixed-address owner/index/generation validation and UINT_MAX retirement; its independent2657-history19844-transition model and separate owner/generation controls execute. The changed pool uses a nonreused external uint64 identity, MAX-once issuer and four FREE/LIVE/RETIRED slots; all-retired capacity and exact ordinary generation increments are tested.',
      'Eight GCC/Clang debug/release/ASan+UBSan/TSan profiles execute five anchors and independently reject ten compiled semantic controls. The successor3968-history30848-transition oracle is distinct from the retained model. Five compiling learner anchors reject. The first attempt exposed early harness return before all joins; its failed report is retained, and the final source joins every created reader before semantic checks.',
      'Actual fallible block relocation preserves handles and values, shell relocation initializes a fresh mutex and transfers the block while leaving the old shell closed. The MAX-1/failed-compaction variant joins3000 old-shell reads then restarts3000 on the new shell. Three competing readers perform30000 copied gets while1000 incarnations/100 compactions/close occur; join precedes destroy. A defined atomic handshake observes999 after checking old generation1, refuting generation-only lifetime protection. Thirteen cost/enforcement rows include paired successful-read samples, build/rebuild, sizes/stack/dependencies and explicit platform limits.'
    ]
    inputs={**d['input_sha256'],**q['input_sha256']};criteria=[];updates=[]
    for r,claim in zip(m['rubric'],claims):criteria.append({**r,'author_readiness':'ready-bounded','demonstration':claim,'teaching':teaching,'implementation':implementation,'validation':[REPORT,DIAGNOSTICS],'independent_learner':'not-assessed','external_review':'not-run','physical_observation':'not-applicable-host-contract'})
    for identity in ['X-T67-005','X-T67-005-DELAYED','T67-05-TRANSFER','T67-05-INVESTIGATION']+[r['id'] for r in m['rubric']]:
        claim=next((r['demonstration'] for r in criteria if r['id']==identity),claims[3] if identity.endswith(('DELAYED','TRANSFER')) else ' '.join(claims))
        updates.append(dict(id=identity,dimensions={'teaching':dict(state='complete',evidence=teaching),'implementation':dict(state='complete',evidence=implementation),'author_validation':dict(state='passed',evidence=[REPORT,DIAGNOSTICS,READINESS]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed',evidence=[]),'physical_observation':dict(state='not-applicable-host-contract',evidence=[teaching[2]])},closure_rationale=claim+' Exact bounded author support; unaided delayed learner and non-author assessment remain unobserved.'))
    path='research/v5/modern/ad_mp_gap_ledger.json';g=read_json(ROOT/path)
    for row in g['gaps']:
        if row['id'] in ['T67-05-TRANSFER','T67-05-INVESTIGATION']:row.update(status='author-support-implemented-and-source-validated; independent-learner-not-assessed',author_evidence=[READINESS,REPORT,DIAGNOSTICS],remaining_external=['Unaided delayed learner attempt and per-objective assessment','Non-author review; no physical or durable-issuer evidence'])
    dump(ROOT/path,g)
    for p in teaching+implementation+[REPORT,DIAGNOSTICS,'tools/build_adt_relocating_pool_readiness.py','tools/check_adt_relocating_pool.py']:inputs[p]=sha(ROOT/p)
    dump(ROOT/READINESS,dict(schema_version=1,domain='T67',criteria=criteria,variants=[dict(id='X-T67-005-DELAYED',author_support='implemented-and-author-validated',evidence=teaching+implementation+[REPORT,DIAGNOSTICS],independent_learner='not-assessed')],current_reports={p:sha(ROOT/p) for p in [REPORT,DIAGNOSTICS]},input_sha256=inputs,limits=['Copied reads use a POSIX mutex; raw payload borrows, lock-free progress and durable identity issuance are not supplied.','Current official POSIX origin inaccessible; identified2017 reprint and glibc2.42 primary lock manual have explicit source bounds.','No published C17/C23 inspection, all-interleaving proof, physical execution, independent learner result or non-author review inferred.']))
    inputs[READINESS]=sha(ROOT/READINESS);dump(ROOT/'research/v5/modern/adt_relocating_pool_completion_dispositions.json',dict(schema_version=1,updates=updates,current_reports=[REPORT,DIAGNOSTICS],input_sha256=inputs));print(json.dumps(dict(passed=True,criteria=4,updates=8)))
if __name__=='__main__':main()
