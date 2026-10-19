"""Bind original T27 obligations to current author evidence, preserving external work."""
from pathlib import Path
import hashlib,json,copy,subprocess
ROOT=Path(__file__).resolve().parents[1]
HERE=ROOT/'research/v5/expansion/legacy_advanced_bringup'
REVIEW='reviews/v5/expansion/legacy_advanced_bringup/'
SUPPORT='research/v5/expansion/legacy_advanced_bringup/completion_dispositions.json'
REPORTS=[REVIEW+n for n in ['current_validation.json','current_images.json','exports.json','support_validation.json','integration.json']]
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def put(p,d):(ROOT/p).write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def main():
 from check_bringup_depth import validate
 core,images=map(read,REPORTS[:2]);assert not validate(core,images)
 files=set(REPORTS+[SUPPORT.replace('completion_dispositions.json','objective_mapping.json'),'tools/close_bringup_depth.py','tools/check_bringup_package.py','research/v5/expansion/legacy_advanced/author_support.py'])
 for name in REPORTS:
  r=read(name);assert r.get('passed') is True,name
  for key in ['input_sha256','outputs_sha256']:
   for f,h in r.get(key,{}).items():assert sha(f)==h,(name,f)
  files.update(r.get('input_sha256',{}))
 files.update(p.relative_to(ROOT).as_posix() for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json')
 cases=read('assessor/bringup_depth/assessment_cases.json')['cases'];byid={c['criterion']:c for c in cases};updates=[]
 # Each category links exact evidence labels; reasoning-only clauses retain their
 # explicit premises/analysis rather than borrowing a narrow executable test.
 selectors={
 '01':['core compile','independent image'], '02':['boot-offset','software-reset','watchdog-cross','independent set oracle'],
 '03':['independent image','raw ROM image','eight actual ELF mutations','gc-missing','ram-vectors','boot-offset'],
 '04':['stack argument','guard','stack-overlap','stack-align','disassembly','attributes','forbidden helper'],
 '05':['independent set oracle','independent image','dma-bank','execute 2'],
 '06':['execute 3','execute 0','fp disassembly','fp attributes'],
 '07':['execute 0','lifecycle','forbidden helper','ordinary execute'],
 '08':['vendor','physical-cross','mps2-missing-copy','mps2-repaired-copy','original'],
 '09':['independent set oracle','execute 4','execute 5','lifecycle'],
 '10':['ram-markers','gc-missing','bad-lma','bad-zero','two-defects','boot-offset','software-reset']}
 for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T27-*.json')):
  rel=path.relative_to(ROOT).as_posix();d=read(rel);files.add(rel)
  old=json.loads(subprocess.check_output(['git','show','HEAD:'+rel],cwd=ROOT,text=True,encoding='utf8'))
  assert old['criteria']==d['criteria'] and old['baseline_binding']==d['baseline_binding']
  for c in d['criteria']:
   case=byid.pop(c['id']);v=c['variants'][0];assert case['delayed_id']==v['id'] and case['minimum_days']==v['minimum_days']==7
   topic=c['objective'].split('.')[1];evidence=[]
   for report,r in zip(REPORTS[:2],[core,images]):
    evidence += [dict(report=report,check=x['label']) for x in r['checks'] if any(s in x['label'] for s in selectors[topic])]
   assert evidence
   artifacts=['labs/bringup_depth/CONTRACTS.md','labs/bringup_depth/BOARD_CAPTURE.md','assessor/bringup_depth/ANALYSIS.md','assessor/bringup_depth/assessment_cases.json','research/v5/expansion/legacy_advanced_bringup/resources.json']
   for delayed in [False,True]:
    dimensions={k:dict(state=state,evidence=ev) for k,state,ev in [
     ('teaching','complete',artifacts[:2]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),
     ('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    updates.append(dict(id=v['id'] if delayed else c['id'],parent_project=d['project'],objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=7,dimensions=dimensions,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=evidence,analysis_case='assessor/bringup_depth/assessment_cases.json#'+c['id'],package_reports=REPORTS),closure_rationale=case['changed_expected'] if delayed else case['ordinary_expected'],remaining_author_work=[],evidence_limits=['This closes author implementation of the specified model/image/protocol, not a physical board observation.','No independent learner, delayed attempt or non-author review; source inspection is limited to named passages.','MPS2 is not STM32; F439 cold/watchdog are cross-builds; no hardware clock/cache/DMA timing is inferred.']))
 assert not byid and len(updates)==60
 put(SUPPORT,dict(schema_version=1,scope='Thirty original T27 criteria and thirty unchanged delayed tasks; author implementation and bounded execution only',source_sha256={f:sha(f) for f in sorted(files)},current_reports=REPORTS,updates=updates,remaining_parents=['Hardware reset, electrical/board identity and physical timing observations remain pending under their originating records.','LRUN and source-access parents are not closed by association.'],summary=dict(criteria=30,delayed=30,core_checks=len(core['checks']),image_checks=len(images['checks']),export_commands=len(read(REPORTS[2])['commands']))))
 print('PASS sixty exact T27 author dispositions')
if __name__=='__main__':main()
