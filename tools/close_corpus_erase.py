"""Bind exact storage roots, rubric criteria and named delayed obligations."""
import hashlib,json,pathlib,sys,datetime
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief,validate
from corpus_export_bindings import errors as export_errors
HERE=ROOT/'research/v5/expansion/corpus_erase';IDS=['X-T14-017']
REPORTS=['reviews/v5/expansion/corpus_erase/'+n for n in ['corpus_erase_campaign.json','exports.json']]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 files={pathlib.Path(__file__),ROOT/'tools/check_corpus_erase.py'}
 for folder in ['labs/corpus_erase','assessor/corpus_erase','fixtures/corpus_erase','research/v5/expansion/corpus_erase']:
  files|={p for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts and p.name!='completion_dispositions.json'}
 for name in REPORTS:
  report=json.loads((ROOT/name).read_text());assert report['passed'];files.add(ROOT/name)
  for key in ['source_sha256','inspected_paths','outputs_sha256']:
   for n,h in report.get(key,{}).items():assert sha(ROOT/n)==h,(name,n);files.add(ROOT/n)
  for command in report.get('commands',report.get('executions',[])):
   if 'log' in command:assert sha(ROOT/command['log'])==command['log_sha256'];files.add(ROOT/command['log'])
 assert not export_errors(json.loads((ROOT/REPORTS[-1]).read_text()),IDS)
 rows=[]
 for ident in IDS:
  brief=ROOT/'curriculum/v5/exercises'/f'{ident}.md';meta,body=parse_brief(brief);validate(meta,body);files.add(brief)
  artifacts=[brief.relative_to(ROOT).as_posix(),'labs/corpus_erase/CONTRACTS.md',meta['assessor_analysis'],'assessor/corpus_erase/'+'erase.c','assessor/corpus_erase/oracle.py']
  for name,kind,obj in [(ident,'root',None),*[(r['id'],'rubric',r['objective']) for r in meta['rubric']],(ident+'-DELAYED','delayed',None)]:
   rows.append(dict(id=name,parent_exercise=ident,parent_project=meta['project'],kind=kind,objective=obj,original_delayed_variant=ident+'-DELAYED',minimum_days=7,dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:2]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]},artifacts=artifacts,remaining_author_work=[],closure_rationale='Actual bounded mechanism and named changed condition have concrete implementations, independent models and distinct causal controls. Exact scope does not imply whole-engine or whole-leaf equivalence.',evidence_limits=['Finite author validation, not learner mastery or elapsed delayed assessment.','ISO C does not enforce ownership or callback effects; public tokens are not unforgeable capabilities.','Current C++ working draft and separately selected C++17 implementation observations are distinct evidence.','Profile C permits compiler byte helpers; no strict final image claim.']))
 target=HERE/'completion_dispositions.json'
 if target.is_file():
  history=ROOT/'reviews/v5/expansion/corpus_erase'/('previous-completion-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')+'.json');history.write_bytes(target.read_bytes())
 target.write_text(json.dumps(dict(schema_version=1,source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,scope='One exact required mechanism:1 root,3 objective criteria,1 delayed owning/fallible variant.',summary=dict(exercises=1,rubrics=3,delayed=1,profiles=6,independent_cases_per_profile=4097)),indent=2)+'\n');print('PASS5 erase dispositions')
if __name__=='__main__':main()
