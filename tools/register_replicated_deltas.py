"""Preserve canonical T76.05 durable causal-delta successor integration."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ANCHORS=['replicated.'+a for a in ['delta_sets','delta_wire','delta_durable','delta_protocol']]
LOCATOR='Section2 system model; section6 Definitions4-6 and Proposition2; section6.1 Algorithm2 and durable-counter/full-state-fallback rationale'
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def dump(p,x):(ROOT/p).write_text(json.dumps(x,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
def register():
    family=read('labs/replicated/family.json');family['tests']=list(dict.fromkeys(family['tests']+ANCHORS))
    for row in family['projects']+family['exercises']:
        if row['id'] in ['PR-T76-05','X-T76-005']:row['tests']=list(dict.fromkeys(row['tests']+ANCHORS))
    family['campaign']['deltas_producer']='tools/run_replicated_deltas.py';family['campaign']['deltas_report']='research/v5/modern/replicated_deltas_validation.json';dump('labs/replicated/family.json',family)
    p=ROOT/'labs/replicated/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/deltas/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    p=ROOT/'curriculum/v5/exercises/X-T76-005.md';body=p.read_text(encoding='utf8');before,rest=body.split('```json\n',1);raw,after=rest.split('\n```',1);meta=json.loads(raw)
    meta['lab']['tests']=list(dict.fromkeys(meta['lab']['tests']+ANCHORS));meta['shared_contracts']=list(dict.fromkeys(meta['shared_contracts']+['labs/replicated/deltas/DELTAS.md']));meta['fixtures']=list(dict.fromkeys(meta['fixtures']+['fixtures/replicated/deltas.json']))
    for row in meta['source_records']:
        if row['id']=='R-RP-DELTA':row['passages']=list(dict.fromkeys(row['passages']+[LOCATOR]))
    h='\n## Required durable causal delta intervals\n';after=after.split(h,1)[0].rstrip()+h+'''

Implement [DELTAS.md](../../../labs/replicated/deltas/DELTAS.md): three actual
RPSet-based replicas, retained exact tombstones, bounded interval history,
durable transition/transport identity, commit-before-acknowledgment and
full-state fallback. The actual page-store and first-principles hash imports
are explicit. Four anchors and ten semantic controls accompany 222 commit
cuts and the independent 3,104-transition/9,312-snapshot causal oracle. Include
last-sequence/epoch and stale-authority refusals. The typed local transport is
not a real network or Byzantine protocol. Preserve all original ordinary,
independent and X-T76-005-DELAYED obligations; author campaigns supply support,
not a learner's unaided result or physical durability evidence.
'''
    p.write_text(before+'```json\n'+json.dumps(meta,indent=2,ensure_ascii=False)+'\n```'+after,encoding='utf8')
    p=ROOT/meta['assessor_analysis'];body=p.read_text(encoding='utf8');h='\n## Durable causal delta analysis\n';p.write_text(body.split(h,1)[0].rstrip()+h+'\nSee [ANALYSIS.md](../deltas/ANALYSIS.md) for the two-block abstraction, durable prefix/session proof, uncertainty and unsafe tombstone-deletion counterexample.\n',encoding='utf8')
    dump('fixtures/replicated/deltas.json',dict(exercise='X-T76-005',namespace=9,replicas=3,values=[0,1,2,3],tags_per_writer=8,retained_delta_capacity=4,snapshot_bytes=128,histories=32,transition_rows=3104,snapshot_comparisons=9312,causal_base_checks=413,commit_cut_cases=222,corruption_bit_cases=1024,finite_boundaries=['sequence UINT64_MAX-1 to UINT64_MAX then refusal','epoch UINT64_MAX-1 to UINT64_MAX then refusal','subsumed body at sequence exhaustion can be acknowledged','new body at sequence exhaustion leaves ACK unchanged','old sender packet after new-epoch full handshake is stale'],limitations=['Counts specify required experiments; execution requires the matching source-bound report.','Boundary injection does not execute 2^64 operations.','Typed local messages, identified noncorrupt peers, finite tombstone retention and imported flush durability are explicit premises.']))
if __name__=='__main__':register()
