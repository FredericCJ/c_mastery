"""Register the concrete T76.02 successor without creating execution evidence."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ANCHORS=['replicated.'+x for x in ['outbox_receive','outbox_ack','outbox_transport','outbox_faults','outbox_native']]
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def dump(p,x):(ROOT/p).write_text(json.dumps(x,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
def register():
    family=read('labs/replicated/family.json')
    family['tests']=list(dict.fromkeys(family['tests']+ANCHORS))
    for row in family['projects']+family['exercises']:
        if row['id'] in ['PR-T76-02','X-T76-002']:row['tests']=list(dict.fromkeys(row['tests']+ANCHORS))
    family['dependencies']=list(dict.fromkeys(family['dependencies']+['hosted_systems']))
    imp=dict(family='hosted_systems',artifact='ipc.local',producer='PR-T24-02',adapter_producer='PR-T24-03',
        learner_source='labs/hosted_systems/stream_depth/stream.c',assessor_source='assessor/hosted_systems/stream_depth/stream.c',
        include='labs/hosted_systems/stream_depth',contract='labs/hosted_systems/stream_depth/CONTRACTS.md',
        scope='Explicit later stream/framing producer adapter plus original local socketpair harness; no assertion that the historical IPC exercise supplied a general reactor.')
    family['source_imports']=[r for r in family['source_imports'] if r['artifact']!='ipc.local']+[imp]
    family['campaign']['outbox_producer']='tools/run_replicated_outbox.py'
    family['campaign']['outbox_report']='research/v5/modern/replicated_outbox_validation.json'
    dump('labs/replicated/family.json',family)
    path=ROOT/'labs/replicated/CMakeLists.txt';body=path.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/outbox/CMakeLists.txt")'
    if line not in body:path.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    path=ROOT/'curriculum/v5/exercises/X-T76-002.md';body=path.read_text(encoding='utf8');before,rest=body.split('```json\n',1);raw,after=rest.split('\n```',1);meta=json.loads(raw)
    meta['lab']['tests']=list(dict.fromkeys(meta['lab']['tests']+ANCHORS))
    meta['shared_contracts']=list(dict.fromkeys(meta['shared_contracts']+['labs/replicated/outbox/OUTBOX.md']))
    meta['fixtures']=list(dict.fromkeys(meta['fixtures']+['fixtures/replicated/outbox.json']))
    heading='\n## Actual durable inbox/outbox successor\n'
    after=after.split(heading,1)[0].rstrip()+heading+'''

Implement the concrete three-store protocol and exact local framing in
[OUTBOX.md](../../../labs/replicated/outbox/OUTBOX.md). It imports the actual
durable-log, page-store, projection, codec and stream producers. Five anchors,
ten compiled semantic controls, an independent list oracle,148 persistent cut
cases and actual local sender/receiver process terminations distinguish stored
authority from reply progress. The original separate-effect specimen produces
a harmless duplicate counter; trusted reply correlation is not authentication.
Retain all original reasoning, cost review and delayed unaided obligations.
'''
    path.write_text(before+'```json\n'+json.dumps(meta,indent=2,ensure_ascii=False)+'\n```'+after,encoding='utf8')
    path=ROOT/meta['assessor_analysis'];body=path.read_text(encoding='utf8');heading='\n## Durable inbox/outbox analysis\n'
    path.write_text(body.split(heading,1)[0].rstrip()+heading+'\nSee [ANALYSIS.md](../outbox/ANALYSIS.md) for prefix invariants, historical replies, uncertainty, actual process cuts and external-effect limits.\n',encoding='utf8')
    dump('fixtures/replicated/outbox.json',dict(exercise='X-T76-002',uuid=19,schema=7,events=[dict(position=1,key=2,kind=1,value=7),dict(position=2,key=3,kind=1,value=-2)],
         expected=dict(authoritative_inbox_count=2,acknowledged_count=2,rebuilt_total=5,flawed_external_counter=2),
         protocol=dict(request_bytes=64,reply_bytes=24,frame_payload=8,request_wire_bytes=72,reply_wire_bytes=27),
         experiment=dict(callback_boundaries_per_append=37,stores=['inbox','acknowledgments'],persistence_modes=['discard-pending','persist-all-pending'],cases=148),
         limits=['Expected results are a specification; execution requires a matching report.','No cryptographic authentication or physical durability claim.']))
if __name__=='__main__':register()
