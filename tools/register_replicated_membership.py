"""Preserve canonical T76.04 membership/authentication successor integration."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ANCHORS=['replicated.'+a for a in ['membership_join','membership_wire','membership_auth','membership_replay']]
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def dump(p,x):(ROOT/p).write_text(json.dumps(x,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
def register():
    family=read('labs/replicated/family.json');family['tests']=list(dict.fromkeys(family['tests']+ANCHORS))
    for row in family['projects']+family['exercises']:
        if row['id'] in ['PR-T76-04','X-T76-004']:row['tests']=list(dict.fromkeys(row['tests']+ANCHORS))
    family['campaign']['membership_producer']='tools/run_replicated_membership.py';family['campaign']['membership_report']='research/v5/modern/replicated_membership_validation.json';dump('labs/replicated/family.json',family)
    p=ROOT/'labs/replicated/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/membership/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    p=ROOT/'curriculum/v5/exercises/X-T76-004.md';body=p.read_text(encoding='utf8');before,rest=body.split('```json\n',1);raw,after=rest.split('\n```',1);meta=json.loads(raw)
    meta['lab']['tests']=list(dict.fromkeys(meta['lab']['tests']+ANCHORS));meta['shared_contracts']=list(dict.fromkeys(meta['shared_contracts']+['labs/replicated/membership/MEMBERSHIP.md']));meta['fixtures']=list(dict.fromkeys(meta['fixtures']+['fixtures/replicated/membership.json']))
    h='\n## Required retained membership and authenticated admission\n';after=after.split(h,1)[0].rstrip()+h+"""

Implement [MEMBERSHIP.md](../../../labs/replicated/membership/MEMBERSHIP.md):
retained identity-tagged PN components, canonical checked snapshots, exact
writer-bound HMAC admission and explicit cap/revocation policy. Four anchors
and nine controls accompany the independent12,416-row dictionary/byte/MAC
oracle. The preparatory HMAC slice is required before this adapter without
requiring completion of the later Merkle exercise. Preserve all original
register, ordinary-law and delayed unaided obligations.
""";p.write_text(before+'```json\n'+json.dumps(meta,indent=2,ensure_ascii=False)+'\n```'+after,encoding='utf8')
    for ident in ['R-RP-HMAC','R-RP-HMAC-VECTORS']:
        record=next(r for r in read('research/v5/modern/replicated_resources.json')['resources'] if r['id']==ident)
        meta['source_records']=[r for r in meta['source_records'] if r['id']!=ident]+[dict(id=ident,path='research/v5/modern/replicated_resources.json',passages=record['locators'])]
    p.write_text(before+'```json\n'+json.dumps(meta,indent=2,ensure_ascii=False)+'\n```'+after,encoding='utf8')
    p=ROOT/meta['assessor_analysis'];body=p.read_text(encoding='utf8');h='\n## Membership and authorization analysis\n';p.write_text(body.split(h,1)[0].rstrip()+h+'\nSee [ANALYSIS.md](../membership/ANALYSIS.md) for the retained-map join, canonical snapshot, writer-bound admission and valid-but-excessive counterexample.\n',encoding='utf8')
    dump('fixtures/replicated/membership.json',dict(exercise='X-T76-004',namespace=9,identities=[10,20,30],capacity=6,snapshot_bytes=160,packet_bytes=64,
         component_max=4294967295,policy_limit=100,histories=128,transition_rows=12416,small_state_law_cases=4368,corruption_bit_cases=1280,
         limitations=['Counts describe required experiments; actual source-bound execution is separate.','No global membership consensus, physical durability or production cryptographic certification is inferred.']))
if __name__=='__main__':register()
