"""Idempotent canonical registration of actual T76 successor implementations.

Called after the existing T76 author recipe so regeneration preserves support.
It does not execute a campaign or create acceptance evidence.
"""
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
ANCHORS=['replicated.durable_log','replicated.durable_snapshot','replicated.durable_bytes']


def read(path):return json.loads((ROOT/path).read_text(encoding='utf8'))
def dump(path,value):(ROOT/path).write_text(json.dumps(value,indent=2,ensure_ascii=False)+'\n',encoding='utf8')


def register():
    path='labs/replicated/family.json';family=read(path)
    family['tests']=list(dict.fromkeys(family['tests']+ANCHORS))
    for row in family['projects']+family['exercises']:
        if row['id'] in ['PR-T76-01','X-T76-001']:
            row['tests']=list(dict.fromkeys(row['tests']+ANCHORS))
    family['dependencies']=list(dict.fromkeys(family['dependencies']+['storage','api_depth']))
    imports=[dict(family='storage',artifact='storage.pages',producer='PR-T47-10',
                  learner_source='labs/storage/src/store.c',assessor_source='assessor/storage/store.c',
                  include='labs/storage/include',contract='labs/storage/include/store.h'),
             dict(family='api_depth',artifact='codec.records',producer='PR-T20-01',
                  learner_source='labs/api_depth/architecture/learner.c',assessor_source='assessor/api_depth/architecture/core.c',
                  include='labs/api_depth/architecture',contract='labs/api_depth/architecture/CONTRACTS.md')]
    for item in imports:
        family['source_imports']=[r for r in family['source_imports'] if r['artifact']!=item['artifact']]+[item]
    family['campaign']['successor_producer']='tools/run_replicated_followthrough.py'
    family['campaign']['successor_report']='research/v5/modern/replicated_durable_validation.json'
    dump(path,family)
    cm=ROOT/'labs/replicated/CMakeLists.txt';body=cm.read_text(encoding='utf8')
    marker='include("${CMAKE_CURRENT_SOURCE_DIR}/followthrough/CMakeLists.txt")'
    if marker not in body:cm.write_text(body.rstrip()+'\n'+marker+'\n',encoding='utf8')
    brief=ROOT/'curriculum/v5/exercises/X-T76-001.md';body=brief.read_text(encoding='utf8')
    before,rest=body.split('```json\n',1);raw,after=rest.split('\n```',1);meta=json.loads(raw)
    meta['lab']['tests']=list(dict.fromkeys(meta['lab']['tests']+ANCHORS))
    contract='labs/replicated/followthrough/DURABLE.md'
    meta['shared_contracts']=list(dict.fromkeys(meta['shared_contracts']+[contract]))
    meta['fixtures']=list(dict.fromkeys(meta['fixtures']+['fixtures/replicated/durable.json']))
    marker='\n## Actual durable successor support\n'
    after=after.split(marker,1)[0].rstrip()+marker+'''

The complete versioned record/page-store adapter, positioned snapshot contract,
three executable anchors, six semantic controls, independent dictionary/byte
oracle and bounded persistence-prefix investigation are specified in
[DURABLE.md](../../../labs/replicated/followthrough/DURABLE.md). It calls the
actual T20 ar_encode/ar_decode, T47 store and RPLog/DOD producers; their exact
interfaces and changed assumptions are explicit. Run the original memory anchor
as well. Source-bound successor reports establish only their stated author
execution scope; the delayed unaided attempt, published-edition access gaps,
physical failures and the separate durable inbox/outbox investigation remain
distinct. This addition supplies the required code/support for the durable-log
variant without rewriting historical acceptance or claiming learner mastery.
'''
    brief.write_text(before+'```json\n'+json.dumps(meta,indent=2,ensure_ascii=False)+'\n```'+after,encoding='utf8')
    analysis=ROOT/meta['assessor_analysis'];body=analysis.read_text(encoding='utf8')
    heading='\n## Durable successor analysis\n'
    body=body.split(heading,1)[0].rstrip()+heading+'\nSee [DURABLE_ANALYSIS.md](../followthrough/DURABLE_ANALYSIS.md) for the actual imported producer protocol, publication rollback, prefix-cut proof and residual effect/physical limitations.\n'
    analysis.write_text(body,encoding='utf8')
    from verify_replicated_durable import record
    dump('fixtures/replicated/durable.json',{'exercise':'X-T76-001','format':'RPL1 event64',
        'cases':[{'input':{'uuid':9,'schema':17,'position':3,'kind':2,'key':31,'value':-(1<<31)},
                  'expected_octets':record(9,17,3,2,31,-(1<<31))}],
        'fault_model':{'page_size':4096,'write_boundaries':34,'flush_boundaries':3,
                       'append_byte_and_flush_prefix_cases':139502,'recovery_byte_and_flush_prefix_cases':139502},
        'limits':['The expected counts specify the experiment; actual execution requires a matching source-bound report.',
                  'Prefix persistence is a finite model, not all unordered sector subsets or physical durability.']})
    from register_replicated_outbox import register as register_outbox
    register_outbox()
    from register_replicated_clocks import register as register_clocks
    register_clocks()
    from register_replicated_hashing import register as register_hashing
    register_hashing()
    from register_replicated_membership import register as register_membership
    register_membership()
    from register_replicated_deltas import register as register_deltas
    register_deltas()
    from register_replicated_sequence_depth import register as register_sequence_depth
    register_sequence_depth()
    from register_replicated_merkle_repair import register as register_merkle_repair
    register_merkle_repair()
    from register_replicated_wire_depth import register as register_wire_depth
    register_wire_depth()
    from register_replicated_replay_depth import register as register_replay_depth
    register_replay_depth()
    from register_replicated_ordinary_depth import register as register_ordinary_depth
    register_ordinary_depth()


if __name__=='__main__':register()
