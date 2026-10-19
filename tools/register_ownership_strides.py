"""Integrate required exact strided and padded-tile author support."""
import json
from common import ROOT,dump,read_json,sha
ANCHORS=['strides_base','strides_sets','strides_tile','strides_transaction','strides_identity','strides_bounds','strides_access']
MARKERS=['OSV_BASE actual-interval','OSV_STRIDED even-odd-exact-sets','OSV_TILE actual-padded-array','OSV_TRANSACTION whole-book','OSV_IDENTITY cross-book','OSV_BOUNDS checked-width','OSV_ACCESS actual-array']

def register():
    public='labs/ownership/strides';contract=public+'/STRIDES.md';source='research/v5/modern/ownership_strides_resources.json';gate='research/v5/modern/ownership_strides_research_gate.json';tests=['ownership.'+a for a in ANCHORS]
    f=read_json(ROOT/'labs/ownership/family.json')
    for r in f['exercises']:
        if r['id']=='X-T66-004':r['tests']=list(dict.fromkeys(r['tests']+tests))
    f['tests']=list(dict.fromkeys(f['tests']+tests));f['campaign'].update(strides_producer='tools/run_ownership_strides.py',strides_report='research/v5/modern/ownership_strides_validation.json');dump(ROOT/'labs/ownership/family.json',f)
    p=ROOT/'labs/ownership/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/strides/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    p=ROOT/'curriculum/v5/exercises/X-T66-004.md';a,rest=p.read_text(encoding='utf8').split('```json\n',1);raw,b=rest.split('\n```',1);m=json.loads(raw);m['lab']['tests']=list(dict.fromkeys(m['lab']['tests']+tests));m['shared_contracts']=list(dict.fromkeys(m['shared_contracts']+[contract]));heading='\n## Required exact-set and padded-tile borrow investigation\n';b=b.split(heading,1)[0].rstrip()+heading+'\n[STRIDES.md](../../../labs/ownership/strides/STRIDES.md) specifies exact bounded address sets, transactional row/column splitting, actual guarded padded-array writes and finite retirement. Retain actual ow_borrow/ow_release/ow_split and separately compare conservative bounding intervals. Original R1-R4 and X-T66-004-DELAYED remain independently assessed.\n' ;p.write_text(a+'```json\n'+json.dumps(m,indent=2,ensure_ascii=False)+'\n```'+b,encoding='utf8')
    inspected=[('assessor/ownership/ownership.c',['Actual ow_borrow/ow_release/ow_split and OW_MUTANT4/14']),('labs/ownership/CONTRACTS.md',['Interval exclusion, finite retirement, transactional split and raw alias limits']),('assessor/ownership/analyses/X-T66-004.md',['Independent finite-set oracle and changed padded tile']),('curriculum/v5/exercises/X-T66-004.md',['Original four criteria and delayed two-dimensional split'])]
    reused=[dict(id='R-MOD-REFCELL',path='research/v5/modern/ownership_resources.json',passages=['borrow and try_borrow','borrow_mut and try_borrow_mut','replace','get_mut','try_borrow_unguarded'],scope='Reuse existing bounded inspected RefCell source record; runtime borrow model comparison does not establish C pointer lifetime or concurrency.')]
    dump(ROOT/source,dict(schema_version=1,resources=[],reused_inspected_records=reused,inspected_artifacts=[dict(path=p,sha256=sha(ROOT/p),passages=s) for p,s in inspected],reference_gaps=['Published C17 remains inaccessible.','No independent human/learner or physical execution evidence.']))
    dump(ROOT/gate,dict(status='passage-inspected',domains=['T66'],reviewer='author only',research_input_sha256={p:sha(ROOT/p) for p in [source,contract,'research/v5/modern/ownership_resources.json']}))
    imports=['labs/ownership/CONTRACTS.md','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','labs/ownership/src/ownership.c','assessor/ownership/ownership.c',source,gate,'research/v5/modern/ownership_resources.json','tools/register_ownership_strides.py']
    dump(ROOT/public/'campaign.json',dict(module='strides',target='ownership-strides',learner_option='CMASTERY_OSV_LEARNER',anchors=ANCHORS,control_count=8,observation_markers=MARKERS,oracle='tools/verify_ownership_strides.py',metrics=['metrics'],object_sources=['strides.c','ownership.c'],producer='tools/run_ownership_strides.py',report='research/v5/modern/ownership_strides_validation.json',imports=imports))
if __name__=='__main__':register()
