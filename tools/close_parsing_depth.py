"""Package exact parser roots, rubric criteria and required delayed variants."""
import hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief,validate
from corpus_export_bindings import errors as export_binding_errors
HERE=ROOT/'research/v5/expansion/corpus_parsing';REPORTS=['reviews/v5/expansion/corpus_parsing/'+n for n in ['parsing_depth_campaign.json','measurement.json','exports.json']]
IDS=['X-T15-010','X-T15-011','X-T15-012','X-T15-013']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 files={pathlib.Path(__file__),ROOT/'tools/check_parsing_depth.py'}
 for folder in ['labs/parsing_depth','assessor/parsing_depth','fixtures/parsing_depth']:
  files|={p for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts}
 files|={HERE/n for n in ['resources.json','research_gate.json','sources.md','reflection.md','taxonomy_refinement.json','author.py','make_stub.py']}
 for name in REPORTS:
  report=json.loads((ROOT/name).read_text());assert report.get('passed') is True,name;files.add(ROOT/name)
  for key in ['source_sha256','inspected_paths','outputs_sha256']:
   for n,h in report.get(key,{}).items():assert sha(ROOT/n)==h,('stale report',name,n);files.add(ROOT/n)
  for command in report.get('commands',report.get('executions',[])):
   if 'log' in command:assert sha(ROOT/command['log'])==command['log_sha256'];files.add(ROOT/command['log'])
 assert not export_binding_errors(json.loads((ROOT/REPORTS[-1]).read_text()),IDS)
 rows=[]
 for id in IDS:
  brief=ROOT/'curriculum/v5/exercises'/f'{id}.md';meta,body=parse_brief(brief);validate(meta,body);files.add(brief)
  entries=[(id,'root',None)]+[(r['id'],'rubric',r['objective']) for r in meta['rubric']]+[(x,'delayed',None) for x in meta['delayed_transfer']['variant_ids']]
  for identifier,kind,objective in entries:
   artifacts=[str(brief.relative_to(ROOT)).replace('\\','/'),'labs/parsing_depth/CONTRACTS.md',meta['assessor_analysis'],'assessor/parsing_depth/'+meta['lab']['tests'][0].split('.')[-1]+'.c','assessor/parsing_depth/oracle.py']
   rows.append(dict(id=identifier,parent_exercise=id,parent_project=meta['project'],objective=objective,kind=kind,original_delayed_variant=meta['delayed_transfer']['variant_ids'][0],minimum_days=7,dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:2]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]},artifacts=artifacts,closure_rationale='The exact named mechanism, its distinct semantic mutation controls and the named changed condition are implemented and exercised. Source derivations/independent flat models are separate from C representation; no whole-objective closure or learner performance is inferred.',remaining_author_work=[],evidence_limits=['Finite bounded implementation and author experiments only.','Cortex-M4 object audit reports compiler-generated support; a complete freestanding target link is not claimed.','Broader corpus screening is separately recorded in corpus_followthrough/screening_map.json.']))
 result=dict(schema_version=1,source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,scope='Four exact newly authored parser mechanisms: four roots,twelve rubric criteria,four delayed variants; distinct from the corpus-wide discovery obligation.',summary=dict(exercises=4,rubrics=12,delayed=4,oracle_observations_per_profile=12064,profiles=6,timing_rows=162))
 (HERE/'completion_dispositions.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS20 exact parser mechanism dispositions')
if __name__=='__main__':main()
