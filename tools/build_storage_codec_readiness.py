"""Bind exact original storage codec tasks and variants to observed support."""
from common import ROOT,dump,read_json,sha
from check_storage_codec import REPORT,EXPORT,check,check_export
BASE='research/v5/expansion/legacy_storage_followthrough'
def main():
    d,e=read_json(ROOT/REPORT),read_json(ROOT/EXPORT);check(d);check_export(e)
    project='curriculum/v5/expansion/legacy_assessments/PR-T47-01.json'
    teaching=[project,'labs/storage/FORMAT.md','labs/storage_followthrough/codec/CONTRACTS.md','assessor/storage_followthrough/codec/ANALYSIS.md']
    implementation=['assessor/storage/store.c','assessor/storage_followthrough/codec/codec.c','labs/storage_followthrough/codec/tests.c','tools/verify_storage_codec.py']
    claims={
      'a':'Independent Python encodes the complete empty and seven-entry maximum CST2 leaf, including every zero/CRC octet; actual retained C encoder matches byte-for-byte and unaligned decode preserves semantic fields. Native struct padding is not serialized.',
      'a-DELAYED':'Original CST3L leaf version and exact field table are implemented, with actual CST2 decoder/encoder adapters, canonical round trips for counts0..7, full-width generation, unknown-version refusal and internal-node migration rejection. No whole-image migration is inferred.',
      'b':'Actual retained decoder rejects independently CRC-resealed magic/generation/page/count/key/value/next/child mutations with unchanged node output before references are followed. Tests separate checksum consistency from structural admission.',
      'b-DELAYED':'The original variable-offset decoder admits subtraction-bounded extents, rejects all positive overlap, accepts reordered disjoint placement and empty one-past values, validates strict key order before staged publication, and supports byte/output overlap. Independent Python sets exhaust1024 selected offset/length combinations; compiled omitted-overlap and early-publication controls are rejected.',
      'c':'An established nonempty actual store is reopened after wrong checkpoint magic, CRC-resealed leaf identity corruption, partial/error read and a torn older checkpoint. All failure images remain byte-identical with zero writes; only the valid newest checkpoint permits successful preserved-data reopen.',
      'c-DELAYED':'A fresh-image factory owns candidate disposal and output publication. Actual store_format/open execute every38 write/flush failure cuts and every5 verification-read failures; each rejects unchanged output and discards exactly once, then retries a different fresh identity. Acquisition failure retains no candidate. No established image is supplied and no physical durability is claimed.'}
    criteria=[];updates=[]
    for suffix,claim in claims.items():
        identity='AX-PR-T47-01-T47.01.'+suffix
        dims=dict(teaching=dict(state='complete',evidence=teaching),implementation=dict(state='complete',evidence=implementation),author_validation=dict(state='passed',evidence=[REPORT,EXPORT,BASE+'/codec_readiness.json']),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-applicable',evidence=['labs/storage_followthrough/codec/CONTRACTS.md']))
        criteria.append(dict(id=identity,author_readiness='ready-bounded',demonstration=claim,teaching=teaching,implementation=implementation,validation=[REPORT,EXPORT],external_review='not-run',independent_learner='not-assessed',physical_observation='not-applicable'))
        updates.append(dict(id=identity,dimensions=dims,closure_rationale=claim+' Bounded author support only; delayed learner administration remains unobserved.'))
    inputs={**d['input_sha256'],**e['input_sha256']}
    for p in teaching+implementation+[REPORT,EXPORT,BASE+'/codec_resources.json',BASE+'/codec_research_gate.json','tools/check_storage_codec.py','tools/register_storage_codec.py','tools/build_storage_codec_readiness.py','tools/check_replicated_replay_depth.py']:inputs[p]=sha(ROOT/p)
    ready=BASE+'/codec_readiness.json';dump(ROOT/ready,dict(schema_version=1,criteria=criteria,input_sha256=inputs,current_reports={p:sha(ROOT/p) for p in [REPORT,EXPORT]},limits=['Original fixed CST2 engine remains unchanged; changed component is leaf-only.','Selected1024 offset/length cases and finite corruption/failure cuts do not prove every file or device.','No learner assessment, human-independent review, physical persistence or strict-freestanding claim.']))
    inputs[ready]=sha(ROOT/ready);dump(ROOT/BASE/'codec_completion_dispositions.json',dict(schema_version=1,updates=updates,current_reports=[REPORT,EXPORT],input_sha256=inputs));print(dict(passed=True,updates=6))
if __name__=='__main__':main()
