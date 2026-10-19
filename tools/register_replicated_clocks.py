"""Preserve canonical T76.03 clock/restart successor integration."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ANCHORS=['replicated.'+a for a in ['clock_identity','clock_store','clock_policy']]
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def dump(p,x):(ROOT/p).write_text(json.dumps(x,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
def register():
    family=read('labs/replicated/family.json');family['tests']=list(dict.fromkeys(family['tests']+ANCHORS))
    for row in family['projects']+family['exercises']:
        if row['id'] in ['PR-T76-03','X-T76-003']:row['tests']=list(dict.fromkeys(row['tests']+ANCHORS))
    family['campaign']['clocks_producer']='tools/run_replicated_clocks.py';family['campaign']['clocks_report']='research/v5/modern/replicated_clocks_validation.json';dump('labs/replicated/family.json',family)
    p=ROOT/'labs/replicated/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/clocks/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    p=ROOT/'curriculum/v5/exercises/X-T76-003.md';body=p.read_text(encoding='utf8');before,rest=body.split('```json\n',1);raw,after=rest.split('\n```',1);meta=json.loads(raw)
    meta['lab']['tests']=list(dict.fromkeys(meta['lab']['tests']+ANCHORS));meta['shared_contracts']=list(dict.fromkeys(meta['shared_contracts']+['labs/replicated/clocks/CLOCKS.md']));meta['fixtures']=list(dict.fromkeys(meta['fixtures']+['fixtures/replicated/clocks.json']))
    h='\n## Actual identity and durable clock successor\n';after=after.split(h,1)[0].rstrip()+h+'''

Implement the retained-incarnation vectors, bounded wall/overflow policy and
actual page-store publication in [CLOCKS.md](../../../labs/replicated/clocks/CLOCKS.md).
Three anchors and six compiled controls accompany a graph-reachability oracle,
independent canonical-byte checks and148 event/identity persistence cuts.
The contract separates externally provisioned unique identities, causal history,
runtime drift admission, finite counter refusal and physical-clock assumptions.
Use the actual retained clock arithmetic and imported storage implementation;
keep all original rubric and delayed unaided obligations.
''';p.write_text(before+'```json\n'+json.dumps(meta,indent=2,ensure_ascii=False)+'\n```'+after,encoding='utf8')
    p=ROOT/meta['assessor_analysis'];body=p.read_text(encoding='utf8');h='\n## Identity and durable clock analysis\n';p.write_text(body.split(h,1)[0].rstrip()+h+'\nSee [ANALYSIS.md](../clocks/ANALYSIS.md) for retained identity, graph-oracle, conditional skew and durable-publication arguments.\n',encoding='utf8')
    dump('fixtures/replicated/clocks.json',dict(exercise='X-T76-003',namespace=9,identities=[10,20,30],capacity=6,wire_bytes=160,
         causal_model=dict(histories=128,events_per_history=48,pair_comparisons=144384,wall_offset_min=-2,wall_offset_max=2,maximum_ahead=4),
         persistence_model=dict(operations=['event','replacement'],callback_boundaries=37,modes=['discard-pending','persist-all-pending'],cases=148),
         limitations=['Counts specify the experiment; actual source-bound execution is recorded separately.','Global identity provisioning, authenticated causal input and physical clock bounds are explicit external premises.']))
if __name__=='__main__':register()
