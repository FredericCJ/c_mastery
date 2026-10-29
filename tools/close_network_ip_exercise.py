"""Bind the required reassembly exercise and each rubric to its own evidence."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_network_ip';REVIEW='reviews/v5/expansion/legacy_network_ip/'
REPORTS=[REVIEW+p for p in ['current_validation.json','support_validation.json','exercise_export.json']]
SUPPORT=HERE/'exercise_completion_dispositions.json'
sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_network_driver.integration import apply as apply_driver
SELECTORS={
'R1':['gcc-debug validation execute','gcc-debug mutant1 execute','gcc-debug mutant4 execute'],
'R2':['gcc-debug mutant7 execute','gcc-debug independent oracle'],
'R3':['gcc-debug mutant5 execute','clang-release independent oracle'],
'R4':['gcc-debug core execute','gcc-debug independent oracle','clang-O3 resources'],
'R5':['gcc-debug mutant6 execute','gcc-debug mutant8 execute','gcc-debug composition execute'],
'R6':['gcc-debug composition execute','clang-release validation execute','clang-O3 execute']}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def brief():return json.loads((ROOT/'curriculum/v5/exercises/X-T41-002.md').read_text().split('```json\n',1)[1].split('\n```',1)[0])
def main():
 d=brief();report=json.loads((ROOT/REPORTS[0]).read_text());assert report['passed'];files={Path(__file__),ROOT/'tools/check_network_ip_exercise_package.py',ROOT/'curriculum/v5/exercises/X-T41-002.md',HERE/'taxonomy_extensions.json'}
 for name in REPORTS:
  r=json.loads((ROOT/name).read_text());assert r['passed'];files.add(ROOT/name)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(name,f);files.add(ROOT/f)
 rows=[];mapping=[]
 identities=[d['id'],*[d['id']+'-'+v for v in d['delayed_transfer']['variant_ids']],*[r['id'] for r in d['rubric']]]
 for ident in identities:
  key=ident.rsplit('-',1)[-1];selected=SELECTORS.get(key,list(dict.fromkeys(x for values in SELECTORS.values() for x in values)))
  criterion=next((r['criterion'] for r in d['rubric'] if r['id']==ident),d['objective_coverage'][0]['assessed_claim'])
  for label in selected:assert any(x['label']==label and x['passed'] for x in report['checks']),label
  dims={k:dict(state=state,evidence=evidence) for k,state,evidence in [('teaching','complete',['labs/network_followthrough/ip/CONTRACTS.md']),('implementation','partial',['assessor/network_followthrough/ip/ip.c','assessor/network_followthrough/ip/ANALYSIS.md']),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
  rows.append(dict(id=ident,parent_ids=[d['id']] if ident!=d['id'] else [],objective='T41.05.a',dimensions=dims,criterion=criterion,minimum_days=7,author_validation=dict(report=REPORTS[0],checks=selected),closure_rationale='Required original reassembly competency implemented and author-executed within the explicit packet/struct lr_cpu scope; actual driver binding remains a separately named required implementation.',remaining_author_work=['NW-IP-DRIVER'],evidence_limits=report['limits']))
  mapping.append(dict(id=ident,objective='T41.05.a',exercise=d['id'],learner_artifacts=['src/ip.c','src/checksum.c','criterion-indexed ownership/coverage/expiry dossier'],assessment=selected,analysis='assessor/network_followthrough/ip/ANALYSIS.md#05a',source_register='research/v5/expansion/legacy_network_ip/resources.json'))
 put(HERE/'exercise_objective_mapping.json',dict(schema_version=1,mapping=mapping));files.add(HERE/'exercise_objective_mapping.json')
 apply_driver(rows,files,'NW-IP-DRIVER')
 put(SUPPORT,dict(schema_version=1,updates=rows,current_reports=REPORTS,source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},remaining_parents=['Actual driver integration verified by the linked successor','Physical capture and independent/external assessment are unobserved.']));print('PASS9 required exercise/variant/rubric author dispositions with verified driver implementation')
if __name__=='__main__':main()
