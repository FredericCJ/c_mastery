"""Bind all five exact interval obligations to actual execution and export evidence."""
import hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief,validate
from corpus_export_bindings import errors as export_binding_errors
HERE=ROOT/'research/v5/expansion/corpus_interval'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 reports=['reviews/v5/expansion/corpus_interval/'+n for n in ['numeric_interval_campaign.json','exports.json']]
 files={pathlib.Path(__file__),ROOT/'tools/check_numeric_interval.py'}
 for folder in ['labs/numeric_interval','assessor/numeric_interval','fixtures/numeric_interval','research/v5/expansion/corpus_interval']:
  files|={p for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts and p.name!='completion_dispositions.json'}
 for name in reports:
  report=json.loads((ROOT/name).read_text());assert report['passed'];files.add(ROOT/name)
  for key in ['source_sha256','inspected_paths','outputs_sha256']:
   for n,h in report.get(key,{}).items():assert sha(ROOT/n)==h,(name,n);files.add(ROOT/n)
  for command in report.get('commands',report.get('executions',[])):
   if 'log' in command:assert sha(ROOT/command['log'])==command['log_sha256'];files.add(ROOT/command['log'])
 id='X-T17-003';brief=ROOT/'curriculum/v5/exercises'/f'{id}.md';meta,body=parse_brief(brief);validate(meta,body);files.add(brief)
 assert not export_binding_errors(json.loads((ROOT/reports[-1]).read_text()),['X-T17-003'])
 rows=[]
 for name,kind,objective in [(id,'root',None),*[(r['id'],'rubric',r['objective']) for r in meta['rubric']],(id+'-DELAYED','delayed',None)]:
  artifacts=[brief.relative_to(ROOT).as_posix(),'labs/numeric_interval/CONTRACTS.md','assessor/numeric_interval/interval.c',meta['assessor_analysis'],'assessor/numeric_interval/oracle.py']
  rows.append(dict(id=name,parent_exercise=id,parent_project=meta['project'],kind=kind,objective=objective,minimum_days=7,original_delayed_variant=id+'-DELAYED',dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:2]),('implementation','complete',artifacts),('author_validation','passed',reports),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]},artifacts=artifacts,remaining_author_work=[],closure_rationale='Actual exact-grid interval operations, correlation-aware square, precision transfer, independent rational model and distinct falsification controls are executed. Scope is the named mechanism, not all numerical analysis.',evidence_limits=['Finite author execution and proof obligations are distinct.','No floating-environment or IEEE1788 conformance validation.','No physical input-bound, external-review or learner-mastery claim.']))
 (HERE/'completion_dispositions.json').write_text(json.dumps(dict(schema_version=1,source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=reports,updates=rows),indent=2)+'\n');print('PASS five exact interval dispositions')
if __name__=='__main__':main()
