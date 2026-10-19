"""Bind exact storage roots, rubric criteria and named delayed obligations."""
import hashlib,json,pathlib,sys,datetime
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief,validate
from corpus_export_bindings import errors as export_errors
HERE=ROOT/'research/v5/expansion/corpus_storage';IDS=['X-T14-015','X-T14-016','X-T15-017']
REPORTS=['reviews/v5/expansion/corpus_storage/'+n for n in ['corpus_storage_campaign.json','exports.json']]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 files={pathlib.Path(__file__),ROOT/'tools/check_corpus_storage.py'}
 for folder in ['labs/corpus_storage','assessor/corpus_storage','fixtures/corpus_storage','research/v5/expansion/corpus_storage']:
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
  artifacts=[brief.relative_to(ROOT).as_posix(),'labs/corpus_storage/CONTRACTS.md',meta['assessor_analysis'],'assessor/corpus_storage/'+meta['lab']['tests'][0].split('.')[-1]+'.c','assessor/corpus_storage/oracle.py']
  for name,kind,obj in [(ident,'root',None),*[(r['id'],'rubric',r['objective']) for r in meta['rubric']],(ident+'-DELAYED','delayed',None)]:
   rows.append(dict(id=name,parent_exercise=ident,parent_project=meta['project'],kind=kind,objective=obj,original_delayed_variant=ident+'-DELAYED',minimum_days=7,dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:2]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]},artifacts=artifacts,remaining_author_work=[],closure_rationale='Actual bounded mechanism and named changed condition have concrete implementations, independent models and distinct causal controls. Exact scope does not imply whole-engine or whole-leaf equivalence.',evidence_limits=['Finite author validation, not learner mastery or elapsed delayed assessment.','Deterministic LCG is not cryptographic or IID; period and unbiased population mapping are separate.','RAM version/publication and explicit hypothetical durability model, not a disk protocol or physical observation.','Profile C permits compiler byte helpers; no strict final image claim.']))
 target=HERE/'completion_dispositions.json'
 if target.is_file():
  history=ROOT/'reviews/v5/expansion/corpus_storage'/('previous-completion-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')+'.json');history.write_bytes(target.read_bytes())
 target.write_text(json.dumps(dict(schema_version=1,source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,scope='Three exact required mechanisms:3 roots,9 objective criteria,3 delayed variants.',summary=dict(exercises=3,rubrics=9,delayed=3,profiles=6,independent_cases_per_profile=8136)),indent=2)+'\n');print('PASS15 storage dispositions')
if __name__=='__main__':main()
