"""Versioned current canonical binding for unchanged prior launcher evidence."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_posix_context';sys.path.insert(0,str(ROOT));sys.path.insert(0,str(ROOT/'tools'))
from check_posix_followthrough import fresh
from research.v5.expansion.legacy_posix_followthrough.author_support import attach as old_attach
from research.v5.expansion.legacy_posix_context.author_support import attach
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def build():
 previous_path=HERE/'launcher-support.before-context.json';before_path=HERE/'PR-T23-01.before-context.json';support_path=ROOT/'research/v5/expansion/legacy_posix_followthrough/support.json';canonical=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T23-01.json'
 assert previous_path.read_bytes()==support_path.read_bytes(),'Old support changed'
 previous=json.loads(previous_path.read_text(encoding='utf8'));before=json.loads(before_path.read_text(encoding='utf8'));current=json.loads(canonical.read_text(encoding='utf8'))
 assert sha(before_path)==previous['source_sha256'][canonical.relative_to(ROOT).as_posix()]
 assert before['criteria']==current['criteria'] and before['baseline_binding']==current['baseline_binding'],'Original criteria/bindings changed'
 assert all(current['execution_support'].get(k)==v for k,v in before['execution_support'].items()),'Prior execution fields changed'
 assert old_attach(copy.deepcopy(current))==current and attach(copy.deepcopy(current))==current,'Non-idempotent additive hook'
 inputs=dict(previous['source_sha256']);inputs[canonical.relative_to(ROOT).as_posix()]=sha(canonical)
 for name,h in inputs.items():assert sha(ROOT/name)==h,('stale old source',name)
 report_path='reviews/v5/expansion/legacy_posix_followthrough/launch_validation.json';report=json.loads((ROOT/report_path).read_text(encoding='utf8'));fresh(report,168,6)
 for p in [previous_path,before_path,support_path,pathlib.Path(__file__),HERE/'author_support.py',ROOT/'tools/check_posix_followthrough.py',ROOT/'research/v5/expansion/legacy_runtime/author_supplements.py']:inputs[p.relative_to(ROOT).as_posix()]=sha(p)
 exports=json.loads((HERE/'exports.json').read_text(encoding='utf8'));assert exports['status']=='pass' and all(c['passed'] for c in exports['commands'])
 for key in ['source_sha256','outputs_sha256']:
  for name,h in exports[key].items():assert sha(ROOT/name)==h,('stale export',name)
 inputs['research/v5/expansion/legacy_posix_context/exports.json']=sha(HERE/'exports.json')
 return dict(schema_version=1,scope='Metadata-only successor; old launcher source, support and execution receipt unchanged',source_sha256=inputs,current_reports=[report_path,'research/v5/expansion/legacy_posix_context/exports.json'],updates=previous['updates'],previous_support=support_path.relative_to(ROOT).as_posix(),previous_support_sha256=sha(support_path),canonical_before=before_path.relative_to(ROOT).as_posix(),criteria_unchanged=True,original_execution_unchanged=True,remaining_author_work=previous['remaining_author_work'],refined_elsewhere='Current posix_context completion dispositions supersede the hard-wait partial and supply T23.02.c; no old limitation text rewritten.',evidence_limits=['No prior experiment rerun is claimed; its actual full source/log closure was revalidated.','Prior nine dimensional claims are preserved exactly; apply the newer criterion-specific package afterward.'])
def validate(d):return [] if d==build() else ['Versioned binding differs from exact original/current inputs']
def main():
 p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');p.add_argument('--self-test',action='store_true');a=p.parse_args();path=HERE/'launcher_binding_successor.json'
 if not a.check:path.write_text(json.dumps(build(),indent=2)+'\n',encoding='utf8')
 d=json.loads(path.read_text(encoding='utf8'));errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['changed-criteria','changed-original-update','stale-current','missing-original','invented-execution']:
   x=copy.deepcopy(d)
   if kind=='changed-criteria':x['criteria_unchanged']=False
   elif kind=='changed-original-update':x['updates'][0]['dimensions']['independent_learner']['state']='passed'
   elif kind=='stale-current':x['source_sha256']['curriculum/v5/expansion/legacy_assessments/PR-T23-01.json']='0'*64
   elif kind=='missing-original':x.pop('previous_support_sha256')
   else:x['original_execution_unchanged']=False
   rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
