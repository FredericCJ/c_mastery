"""Preserve canonical T76.06 Unicode sequence/context successor integration."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ANCHORS=['replicated.'+a for a in ['seq_tree','seq_snapshot','ot_context','ot_delivery']]
LOCATORS={'R-RP-CRDT':'Section3.5.1 and Specification19 RGA timestamp/tombstone/downstream premises; section4.1 stability and fixed-membership liveness','R-RP-OT':'Sections4.1-4.4 transformation necessity, state-vector/request-log context and quiescence; sections5-6 insert/delete transforms and mixed-priority counterexample','R-RP-UTF8':'Section3 scalar ranges, octet construction and surrogate exclusion'}
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def dump(p,x):(ROOT/p).write_text(json.dumps(x,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
def register():
    family=read('labs/replicated/family.json');family['tests']=list(dict.fromkeys(family['tests']+ANCHORS))
    for row in family['projects']+family['exercises']:
        if row['id'] in ['PR-T76-06','X-T76-006']:row['tests']=list(dict.fromkeys(row['tests']+ANCHORS))
    family['campaign']['sequence_depth_producer']='tools/run_replicated_sequence_depth.py';family['campaign']['sequence_depth_report']='research/v5/modern/replicated_sequence_depth_validation.json';dump('labs/replicated/family.json',family)
    p=ROOT/'labs/replicated/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/sequence_depth/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    p=ROOT/'curriculum/v5/exercises/X-T76-006.md';body=p.read_text(encoding='utf8');before,rest=body.split('```json\n',1);raw,after=rest.split('\n```',1);meta=json.loads(raw)
    meta['lab']['tests']=list(dict.fromkeys(meta['lab']['tests']+ANCHORS));meta['shared_contracts']=list(dict.fromkeys(meta['shared_contracts']+['labs/replicated/sequence_depth/SEQUENCE.md']));meta['fixtures']=list(dict.fromkeys(meta['fixtures']+['fixtures/replicated/sequence_depth.json']))
    for ident,loc in LOCATORS.items():
        row=next((r for r in meta['source_records'] if r['id']==ident),None)
        if row:row['passages']=list(dict.fromkeys(row['passages']+[loc]))
        else:meta['source_records'].append(dict(id=ident,path='research/v5/modern/replicated_resources.json',passages=[loc]))
    h='\n## Required Unicode sequence and exact-context protocol\n';after=after.split(h,1)[0].rstrip()+h+'''

Implement [SEQUENCE.md](../../../labs/replicated/sequence_depth/SEQUENCE.md):
early-delete queue versus placeholder admission, scalar-safe UTF-8, canonical
snapshots, fenced three-replica compaction and retained-log server/client OT
with exact prefix vectors. Four anchors and ten semantic controls accompany
9,312 tagged-tree and15,456 marker-token context transitions. The concrete
three-operation axb/abx counterexample distinguishes an original operation from
its transformed coordinate context. This bounded central prefix protocol has
explicit responsiveness, membership, durability and failover limitations; it
does not claim a general decentralized OT theorem. Preserve all original
ordinary, independent and X-T76-006-DELAYED assessment obligations.
'''
    p.write_text(before+'```json\n'+json.dumps(meta,indent=2,ensure_ascii=False)+'\n```'+after,encoding='utf8')
    p=ROOT/meta['assessor_analysis'];body=p.read_text(encoding='utf8');h='\n## Unicode sequence and context analysis\n';p.write_text(body.split(h,1)[0].rstrip()+h+'\nSee [ANALYSIS.md](../sequence_depth/ANALYSIS.md) for retained dependencies, compaction authority and exact-prefix transformed-log reasoning.\n',encoding='utf8')
    dump('fixtures/replicated/sequence_depth.json',dict(exercise='X-T76-006',replicas=3,node_capacity=16,early_delete_capacity=16,ot_log_capacity=16,document_scalar_capacity=32,snapshot_bytes=368,tree_histories=96,tree_rows=9312,ot_histories=96,ot_rows=15456,corruption_bit_cases=2944,counterexample=dict(retained_context='axb',blind_original_context='abx'),limitations=['Counts specify required experiments; execution requires the matching source-bound report.','Fixed membership, trusted central prefix sequencer, one pending local request and a genuinely drained compaction barrier are explicit premises.','No general decentralized OT proof, physical durability, live network or unaided learner result is inferred.']))
if __name__=='__main__':register()
