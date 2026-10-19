"""Attach the exact storage codec successor without rewriting baseline observations."""
from common import ROOT,dump,read_json,sha
BASE='research/v5/expansion/legacy_storage_followthrough'
PUBLIC='labs/storage_followthrough/codec'
def main():
    inspected=[('R-STORAGE-FOLLOW-CODEC','CST3L leaf and disposable creation contract',PUBLIC+'/CONTRACTS.md',['Representation and version policy','Fresh creation and failed open','Required work, evidence and scoring','Validation and cost limits']),('R-STORAGE-FOLLOW-CST2','Retained actual CST2 format contract','labs/storage/FORMAT.md',['Complete CST2 page and recovery contract'])]
    resources=[]
    for identity,title,path,locators in inspected:
        resources.append(dict(id=identity,title=title,url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T47'],role='primary',reason='Exact local representation, failure and ownership authority for the original teaching component.',prerequisites=['PR-T47-01','PR-T47-09 for fresh-open integration'],assignable=True,locators=locators,inspection=dict(date='2026-10-18',method='Read the actual full local contract',scope='; '.join(locators),limits='Original author contract, not a substitute for published ISO C17 or independent assessment.',local_sha256=sha(ROOT/path))))
    dump(ROOT/BASE/'codec_resources.json',dict(resources=resources))
    dump(ROOT/BASE/'codec_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={path:sha(ROOT/path) for _,_,path,_ in inspected},resources=BASE+'/codec_resources.json',limits=['Published C17/C23 access gaps remain.','No fresh complete ARIES/book inspection or physical drive behavior claimed.']))
    p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T47-01.json';d=read_json(p)
    assets=[PUBLIC+'/'+name for name in ['codec.h','codec.c','tests.c','CMakeLists.txt','expect_failure.py','CONTRACTS.md']]+['labs/storage/include/store.h','labs/storage/src/store.c','labs/storage/FORMAT.md']
    d['supplied_artifacts']=list(dict.fromkeys(d['supplied_artifacts']+assets))
    d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[PUBLIC+'/CONTRACTS.md']))
    source=dict(id='R-STORAGE-FOLLOW-CODEC',path=BASE+'/codec_resources.json',passages=inspected[0][3])
    d['source_records']=[r for r in d['source_records'] if r['id']!=source['id']]+[source]
    binding=dict(path=BASE+'/codec_completion_dispositions.json',criterion_ids=[c['id'] for c in d['criteria']],scope='Actual C17 CST2/CST3L codec, independent byte/extent oracle and fresh disposable-format integration; exact original ordinary and delayed tasks. Learner, human review and physical dimensions remain separate.')
    d['execution_support']['storage_codec_successor']=binding
    for c in d['criteria']:c['successor_support']=dict(binding,criterion_ids=[c['id']])
    dump(p,d)
    print({'attached':'PR-T47-01','ordinary':3,'delayed':3,'baseline_observations':'preserved'})
if __name__=='__main__':main()
