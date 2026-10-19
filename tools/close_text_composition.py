"""Bind exact text roots, objective criteria and named delayed variants."""
import hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief,validate
from corpus_export_bindings import errors as export_errors
HERE=ROOT/'research/v5/expansion/corpus_text'
IDS=['X-T15-014','X-T15-015','X-T15-016']
REPORTS=['reviews/v5/expansion/corpus_text/'+n for n in ['text_composition_campaign.json','exports.json']]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 files={pathlib.Path(__file__),ROOT/'tools/check_text_composition.py'}
 for folder in ['labs/text_composition','assessor/text_composition','fixtures/text_composition','research/v5/expansion/corpus_text']:
  files|={p for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts and p.name!='completion_dispositions.json'}
 for name in REPORTS:
  report=json.loads((ROOT/name).read_text());assert report['passed'];files.add(ROOT/name)
  for key in ['source_sha256','inspected_paths','outputs_sha256']:
   for n,h in report.get(key,{}).items():assert sha(ROOT/n)==h,(name,n);files.add(ROOT/n)
  for command in report.get('commands',report.get('executions',[])):
   if 'log' in command:assert sha(ROOT/command['log'])==command['log_sha256'];files.add(ROOT/command['log'])
 assert not export_errors(json.loads((ROOT/REPORTS[-1]).read_text()),IDS+['X-T15-012'])
 rows=[]
 for ident in IDS:
  brief=ROOT/'curriculum/v5/exercises'/f'{ident}.md';meta,body=parse_brief(brief);validate(meta,body);files.add(brief)
  artifacts=[brief.relative_to(ROOT).as_posix(),'labs/text_composition/CONTRACTS.md',meta['assessor_analysis'],'assessor/text_composition/'+{'recovery':'recovery','layout':'layout','template':'template'}[meta['lab']['tests'][0].split('.')[-1]]+'.c','assessor/text_composition/oracle.py']
  for name,kind,obj in [(ident,'root',None),*[(r['id'],'rubric',r['objective']) for r in meta['rubric']],(ident+'-DELAYED','delayed',None)]:
   rows.append(dict(id=name,parent_exercise=ident,parent_project=meta['project'],kind=kind,objective=obj,original_delayed_variant=ident+'-DELAYED',minimum_days=7,dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:2]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]},artifacts=artifacts,remaining_author_work=[],closure_rationale='Actual bounded mechanism and named changed condition are implemented with the actual prerequisite, distinct causal mutations and independent models. This does not close every mechanism of the original topic.',evidence_limits=['Finite author validation, not learner mastery or elapsed delayed assessment.','No generic compiler recovery, optimal layout, formal model-view separation or universal context-encoding guarantee.','Hosted component, not a strict freestanding image or physical observation.']))
 result=dict(schema_version=1,source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,scope='Three exact text mechanisms:3 roots,9 rubric criteria,3 delayed variants.',summary=dict(exercises=3,rubrics=9,delayed=3,profiles=6,independent_cases_per_profile=5563))
 target=HERE/'completion_dispositions.json'
 if target.is_file():
  history=ROOT/'reviews/v5/expansion/corpus_text'/'previous_completion_dispositions.json';history.write_bytes(target.read_bytes())
 target.write_text(json.dumps(result,indent=2)+'\n');print('PASS15 text composition dispositions')
if __name__=='__main__':main()
