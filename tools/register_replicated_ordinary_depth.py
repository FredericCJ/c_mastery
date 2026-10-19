"""Keep required original mechanisms and law falsification connected."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=['log','projection','clocks','counters','sets','sequence','merkle','wire','replay']
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def dump(p,x):(ROOT/p).write_text(json.dumps(x,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
def register():
    anchors=['ordinary_'+a for a in CASES]+['ordinary_laws'];f=read('labs/replicated/family.json');f['tests']=list(dict.fromkeys(f['tests']+['replicated.'+a for a in anchors]))
    for i,case in enumerate(CASES,1):
        tests=['replicated.ordinary_'+case]+(['replicated.ordinary_laws'] if i==4 else [])
        for row in f['projects']+f['exercises']:
            if row['id'] in [f'PR-T76-{i:02d}',f'X-T76-{i:03d}']:row['tests']=list(dict.fromkeys(row['tests']+tests))
        p=ROOT/f'curriculum/v5/exercises/X-T76-{i:03d}.md';body=p.read_text(encoding='utf8');before,rest=body.split('```json\n',1);raw,after=rest.split('\n```',1);m=json.loads(raw)
        m['lab']['tests']=list(dict.fromkeys(m['lab']['tests']+tests));m['shared_contracts']=list(dict.fromkeys(m['shared_contracts']+['labs/replicated/ordinary_depth/ORDINARY.md','labs/replicated/COST.md']))
        heading='\n## Retained mechanism and current-source evidence\n';after=after.split(heading,1)[0].rstrip()+heading+'\nThe required [ordinary-depth contract](../../../labs/replicated/ordinary_depth/ORDINARY.md) recompiles the actual retained provider and its semantic controls. It preserves the original objective and the separate successor investigations; current author evidence does not establish unaided or delayed learner performance.\n'
        p.write_text(before+'```json\n'+json.dumps(m,indent=2,ensure_ascii=False)+'\n```'+after,encoding='utf8')
    f['campaign']['ordinary_depth_producer']='tools/run_replicated_ordinary_depth.py';f['campaign']['ordinary_depth_report']='research/v5/modern/replicated_ordinary_depth_validation.json';dump('labs/replicated/family.json',f)
    p=ROOT/'labs/replicated/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/ordinary_depth/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    dump('labs/replicated/ordinary_depth/campaign.json',dict(module='ordinary_depth',target='replicated-ordinary_depth',learner_option='CMASTERY_ORDINARY_DEPTH_LEARNER',anchors=anchors,control_count=20,observation_markers=['ORDINARY_BASE '+a+' passed' for a in CASES]+['ORDINARY_LAWS pn-triples262144 permutations6 register-triples1000'],oracle='tools/verify_replicated_ordinary_depth.py',metrics=['metrics'],object_sources=['replicated.c','dod.c'],producer='tools/run_replicated_ordinary_depth.py',report='research/v5/modern/replicated_ordinary_depth_validation.json',imports=['tools/verify_replicated_fixtures.py','fixtures/replicated/cases.json','labs/replicated/include/replicated.h','assessor/replicated/replicated.c','labs/replicated/src/replicated.c','labs/replicated/CONTRACTS.md','labs/replicated/tests/tests.c','labs/replicated/tests/oracle_driver.c','labs/replicated/followthrough/expect_failure.py','labs/dod/include/dod.h','assessor/dod/dod.c','labs/dod/CONTRACTS.md']))
if __name__=='__main__':register()
