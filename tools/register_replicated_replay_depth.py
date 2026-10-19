"""Canonical registration of required T76.09 prediction/effect/target support."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ANCHORS=['replay_prediction','replay_snapshot','replay_durable','replay_effects','replay_transport','replay_native']
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def dump(p,x):(ROOT/p).write_text(json.dumps(x,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
def register():
    tests=['replicated.'+x for x in ANCHORS];f=read('labs/replicated/family.json');f['tests']=list(dict.fromkeys(f['tests']+tests))
    for row in f['projects']+f['exercises']:
        if row['id'] in ['PR-T76-09','X-T76-009']:row['tests']=list(dict.fromkeys(row['tests']+tests))
    f['campaign']['replay_depth_producer']='tools/run_replicated_replay_depth.py';f['campaign']['replay_depth_report']='research/v5/modern/replicated_replay_depth_validation.json';f['campaign']['replay_target_report']='research/v5/modern/replicated_replay_target_validation.json';dump('labs/replicated/family.json',f)
    p=ROOT/'labs/replicated/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/replay_depth/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    p=ROOT/'curriculum/v5/exercises/X-T76-009.md';body=p.read_text(encoding='utf8');before,rest=body.split('```json\n',1);raw,after=rest.split('\n```',1);m=json.loads(raw)
    m['lab']['tests']=list(dict.fromkeys(m['lab']['tests']+tests));m['shared_contracts']=list(dict.fromkeys(m['shared_contracts']+['labs/replicated/replay_depth/REPLAY.md']));m['fixtures']=list(dict.fromkeys(m['fixtures']+['fixtures/replicated/replay_depth.json']))
    heading='\n## Required two-player replay and durable effect integration\n'
    after=after.split(heading,1)[0].rstrip()+heading+'''

Implement [REPLAY.md](../../../labs/replicated/replay_depth/REPLAY.md). Its actual
T20 decision adapter, T24 framed local processes, T48 snapshot transaction and
T76 durable intent sink require confirmed-prefix publication, prediction
replacement and stable effect retry. Preserve all original R1–R4 and HAMT
checkpoint obligations. The required changed target is actual big-endian PPC64
execution of canonical state traces; the hosted store/process experiment remains
separate. Six anchors, twelve hosted controls, the wrong-endian control and a
hidden-dependency audit accompany an independent integer/byte oracle. Neither
these author executions nor the supplied variants manufacture the learner's
X-T76-009-DELAYED attempt.
'''
    p.write_text(before+'```json\n'+json.dumps(m,indent=2,ensure_ascii=False)+'\n```'+after,encoding='utf8')
    p=ROOT/m['assessor_analysis'];body=p.read_text(encoding='utf8');heading='\n## Two-player replay and effect publication analysis\n';p.write_text(body.split(heading,1)[0].rstrip()+heading+'\nSee [ANALYSIS.md](../replay_depth/ANALYSIS.md) for exact prediction, prefix, failure and changed-target arguments.\n',encoding='utf8')
    dump('fixtures/replicated/replay_depth.json',dict(exercise='X-T76-009',initial=[[0,0,0,1],[0,7,0,9]],timeline=9,horizon=4,local=[1,0,-1,1],remote=[-1,1,0,-1],arrival=[2,1,4,3,2],confirmed=[0,2,2,4,4],snapshot_bytes=512,input_bytes=16,framed_bytes=18,ordinary_histories=9841,changed_order_histories=768,snapshots=319681,durable_cuts=296,big_endian_traces=128,limits=['Counts specify experiments; only current source-bound reports establish observed results.','Durable intent idempotence does not provide exactly-once external devices.','No physical or unaided learner evidence is inferred.']))
    imports=['labs/replicated/include/replicated.h','assessor/replicated/replicated.c','labs/replicated/CONTRACTS.md','labs/replicated/followthrough/expect_failure.py','labs/replicated/followthrough/durable.h','assessor/replicated/followthrough/durable.c','labs/replicated/followthrough/DURABLE.md','labs/replicated/hashing/hashing.h','assessor/replicated/hashing/hashing.c','labs/replicated/hashing/HASHING.md','labs/dod/include/dod.h','assessor/dod/dod.c','labs/dod/CONTRACTS.md','labs/storage/include/store.h','assessor/storage/store.c','labs/storage/FORMAT.md','labs/api_depth/architecture/arch.h','labs/api_depth/architecture/effects.h','labs/api_depth/architecture/CONTRACTS.md','assessor/api_depth/architecture/core.c','assessor/api_depth/architecture/effects.c','labs/hosted_systems/stream_depth/stream.h','labs/hosted_systems/stream_depth/CONTRACTS.md','assessor/hosted_systems/stream_depth/stream.c']
    dump('labs/replicated/replay_depth/campaign.json',dict(module='replay_depth',target='replicated-replay_depth',learner_option='CMASTERY_REPLAY_DEPTH_LEARNER',anchors=ANCHORS,control_count=12,observation_markers=['REPLAY_PREDICTION suffix-replacement','REPLAY_SNAPSHOT bits4096','REPLAY_DURABLE cuts296','REPLAY_EFFECTS confirmed-only','REPLAY_TRANSPORT actual-parser','REPLAY_COUNTEREXAMPLES hidden-random','REPLAY_NATIVE local-two-player'],oracle='tools/verify_replicated_replay_depth.py',metrics=['metrics'],object_sources=['replay_core.c','replay_wire.c','replay_store.c','replay_transport.c'],producer='tools/run_replicated_replay_depth.py',report='research/v5/modern/replicated_replay_depth_validation.json',imports=imports))
if __name__=='__main__':register()
