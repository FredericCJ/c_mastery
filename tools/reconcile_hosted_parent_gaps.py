"""Aggregate exact LRUN09–13 child author evidence without rewriting the parent."""
from copy import deepcopy
from pathlib import Path
import argparse,hashlib,json
ROOT=Path(__file__).resolve().parents[1];ORIGINAL='research/v5/expansion/legacy_runtime/gap_ledger.json';FOLDER=ROOT/'research/v5/expansion/legacy_hosted_parent_reconciliation'
PACKAGES={'context':'research/v5/expansion/legacy_posix_context/completion_dispositions.json',**{n:'research/v5/expansion/legacy_'+n+'/support.json' for n in ['events','framing','reactor','timers','completion_io']}}
PARENTS={'LRUN-09':['context','timers'],'LRUN-10':['events'],'LRUN-11':['events','framing'],'LRUN-12':['reactor'],'LRUN-13':['completion_io']}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(path):return json.loads((ROOT/path).read_text(encoding='utf8'))
def build():
 original=read(ORIGINAL);files={ORIGINAL,Path(__file__).relative_to(ROOT).as_posix()};packages={};reports=set();updates=[]
 for name,path in PACKAGES.items():
  value=read(path);packages[name]=value;files.add(path)
  for p,h in value['source_sha256'].items():assert (ROOT/p).is_file() and sha(ROOT/p)==h,('Stale child package',name,p)
 for ident,names in PARENTS.items():
  parent=next(r for r in original['gaps'] if r['id']==ident);topics=set(parent['topics']);expected=set();objectives=set()
  for p in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T2[34]-*.json')):
   record=json.loads(p.read_text(encoding='utf8'));selected=[r for r in record['criteria'] if r['objective'].rsplit('.',1)[0] in topics]
   if not selected:continue
   files.add(p.relative_to(ROOT).as_posix())
   for row in selected:expected.add(row['id']);expected.update(v['id'] for v in row['variants']);objectives.add(row['objective'])
  children=[r for name in names for r in packages[name]['updates'] if (r.get('objective') or r.get('original_objective','')).rsplit('.',1)[0] in topics]
  assert expected and len(children)==len(expected) and {r['id'] for r in children}==expected,('Missing/duplicate exact children',ident,expected-{r['id'] for r in children})
  child_reports=set()
  for r in children:
   dims=r['dimensions'];assert dims['teaching']['state'] in ('complete','complete-for-declared-criterion','complete-for-declared-criterion-with-source-gaps'),(r['id'],dims['teaching'])
   assert dims['implementation']['state'] in ('complete','complete-for-declared-reference-contract'),(r['id'],dims['implementation'])
   assert dims['author_validation']['state'] in ('passed','passed-with-stated-scope'),(r['id'],dims['author_validation'])
   for p in dims['author_validation']['evidence']:
    assert (ROOT/p).is_file(),p
    if p.endswith('.json'):
     report=read(p);assert report.get('passed') is True or report.get('status')=='pass',('Failed child receipt',p);files.add(p);child_reports.add(p)
  reports|=child_reports;paths=[PACKAGES[n] for n in names]
  source_gap_children=[r['id'] for r in children if r['dimensions']['teaching']['state'].endswith('-with-source-gaps')]
  updates.append(dict(id=ident,original_source=ORIGINAL,original_record=parent,domain='T23/T24' if ident=='LRUN-09' else 'T24',objectives=sorted(objectives),child_ids=sorted(expected),child_packages=paths,source_gap_children=source_gap_children,child_dimension_states={r['id']:{k:v['state'] for k,v in r['dimensions'].items()} for r in children},closure_rationale='The exact parent topics now have concrete ordinary and changed-condition reference implementations, primary-interface scopes and guarded author execution. Original learner wording is retained: this closes executable author work only, not source-access gaps, integration of an actual learner portfolio or independent assessment.',dimensions={key:dict(state=state,evidence=evidence) for key,state,evidence in [('teaching','complete-for-declared-parent-author-scope'+('-with-source-gaps' if source_gap_children else ''),paths),('implementation','complete-for-declared-parent-author-scope',paths),('author_validation','passed-with-child-scope-limitations',sorted(child_reports)),('external_review','not-run',[]),('independent_learner','not-assessed; original minimum7days unchanged',[]),('physical_observation','not-observed',[])]},evidence_limits=['Original parent record/status preserved verbatim; source-access LRUN02 remains distinct and child source gaps are explicitly propagated.','Models, actual local guest operations, compiler observations and physical measurements are different evidence classes.','No finite campaign certifies universal correctness, worst-case latency or absolute mastery.']))
 return dict(schema_version=1,required=True,scope='Versioned aggregation of existing LRUN09–13 only; six dimensions remain separate',source_sha256={p:sha(ROOT/p) for p in sorted(files)},current_reports=sorted(reports),updates=updates)
def validate(d):return [] if d==build() else ['Aggregate differs from exact current children/original parents']
def main():
 p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');p.add_argument('--self-test',action='store_true');p.add_argument('--report',required=True);a=p.parse_args();path=FOLDER/'completion_dispositions.json'
 if not a.check:FOLDER.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(build(),indent=2)+'\n',encoding='utf8',newline='\n')
 d=json.loads(path.read_text(encoding='utf8'));errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-parent','missing-child','wrong-objective','stale-input','inflated-review','inflated-physical','changed-original','lost-delay']:
   bad=deepcopy(d)
   if kind=='missing-parent':bad['updates'].pop()
   elif kind=='missing-child':bad['updates'][0]['child_ids'].pop()
   elif kind=='wrong-objective':bad['updates'][0]['objectives']=['unrelated']
   elif kind=='stale-input':bad['source_sha256'][next(iter(bad['source_sha256']))]='0'*64
   elif kind=='inflated-review':bad['updates'][0]['dimensions']['external_review']['state']='passed'
   elif kind=='inflated-physical':bad['updates'][0]['dimensions']['physical_observation']['state']='observed'
   elif kind=='changed-original':bad['updates'][0]['original_record']['status']='complete'
   else:bad['updates'][0]['child_ids']=[v for v in bad['updates'][0]['child_ids'] if not v.endswith('-DELAYED')]
   rejected=bool(validate(bad));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('Bad guard '+kind)
 result=dict(passed=not errors,updates=len(d['updates']),negative_controls=controls,errors=errors,input_sha256={path.relative_to(ROOT).as_posix():sha(path),Path(__file__).relative_to(ROOT).as_posix():sha(Path(__file__))});out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8',newline='\n');print(dict(passed=not errors,updates=len(d['updates']),checks=len(controls),errors=errors));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
