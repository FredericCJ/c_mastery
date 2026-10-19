"""Preserve T76.07 inclusion-proof and framed repair integration."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ANCHORS=['replicated.'+a for a in ['merkle_proof','merkle_wire','merkle_repair','merkle_cost']]
LOCATOR='Sections2.1.1-2.1.3.2 raw tree domain/split, recursive inclusion path and fn/sn exact-path verifier'

def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def dump(p,x):(ROOT/p).write_text(json.dumps(x,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
def register():
    family=read('labs/replicated/family.json');family['tests']=list(dict.fromkeys(family['tests']+ANCHORS))
    for row in family['projects']+family['exercises']:
        if row['id'] in ['PR-T76-07','X-T76-007']:row['tests']=list(dict.fromkeys(row['tests']+ANCHORS))
    family['campaign']['merkle_repair_producer']='tools/run_replicated_merkle_repair.py';family['campaign']['merkle_repair_report']='research/v5/modern/replicated_merkle_repair_validation.json';dump('labs/replicated/family.json',family)
    p=ROOT/'labs/replicated/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/merkle_repair/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    p=ROOT/'curriculum/v5/exercises/X-T76-007.md';body=p.read_text(encoding='utf8');before,rest=body.split('```json\n',1);raw,after=rest.split('\n```',1);meta=json.loads(raw)
    meta['lab']['tests']=list(dict.fromkeys(meta['lab']['tests']+ANCHORS));meta['shared_contracts']=list(dict.fromkeys(meta['shared_contracts']+['labs/replicated/merkle_repair/MERKLE.md']));meta['fixtures']=list(dict.fromkeys(meta['fixtures']+['fixtures/replicated/merkle_repair.json']))
    row=next(r for r in meta['source_records'] if r['id']=='R-RP-MERKLE');row['passages']=list(dict.fromkeys(row['passages']+[LOCATOR]))
    h='\n## Required inclusion proofs and framed repair\n';after=after.split(h,1)[0].rstrip()+h+'''

Implement [MERKLE.md](../../../labs/replicated/merkle_repair/MERKLE.md):
arbitrary bounded leaf bytes, exact inclusion paths, application-domain/count
commitments, canonical query/reply frames through the actual PR-T24-03 parser,
and staged selective repair. Compare selective traversal, complete traversal and
direct full-leaf transfer under the same final-root publication contract.
Four anchors and ten semantic controls accompany 2,311 independently calculated
root/path rows and 2,090 repair rows. A trusted root is an external premise;
hash agreement does not authenticate its source or guarantee availability.
Preserve R1–R4, the streaming/HMAC work and X-T76-007-DELAYED. Author campaigns
do not establish independent learner mastery, physical durability or a live
network experiment.
'''
    p.write_text(before+'```json\n'+json.dumps(meta,indent=2,ensure_ascii=False)+'\n```'+after,encoding='utf8')
    p=ROOT/meta['assessor_analysis'];body=p.read_text(encoding='utf8');h='\n## Inclusion proof and repair analysis\n';p.write_text(body.split(h,1)[0].rstrip()+h+'\nSee [ANALYSIS.md](../merkle_repair/ANALYSIS.md) for proof shape, authenticated publication, actual parser integration and equivalent transfer-cost reasoning.\n',encoding='utf8')
    dump('fixtures/replicated/merkle_repair.json',dict(exercise='X-T76-007',max_leaves=16,max_leaf_bytes=32,max_proof_depth=4,request_bytes=27,response_bytes=108,
        proof_rows=2311,independently_verified_paths=2284,repair_rows=2090,repair_status_counts={'0':1698,'5':301,'3':91},
        changed_leaf_cost=dict(selective_requests=5,all_node_requests=31,direct_leaf_requests=16,selective_client_hashes=122,all_node_client_hashes=64,direct_leaf_client_hashes=48),
        required_malformed_cases=['reordered siblings','missing or extra sibling','wrong index/count/domain','depth beyond native capacity','empty proof request','stale nonce','nonzero reserved/padding bytes','truncated frame','missing node','corrupt child or leaf'],
        limits=['Counts specify required experiments; execution requires the matching source-bound report.','Trusted commitment and finite synchronous local callbacks are premises.','No collision-resistance proof, origin authentication, physical durability, deployed CT service or unaided learner result is inferred.']))
if __name__=='__main__':register()
