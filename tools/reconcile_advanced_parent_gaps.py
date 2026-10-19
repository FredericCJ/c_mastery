"""Existing LRUN18/19 author aggregation; never invent domain parent identities."""
import argparse,copy,functools,hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_parent_reconciliation';ORIGINAL='research/v5/expansion/legacy_runtime/gap_ledger.json';PACKAGES={n:'research/v5/expansion/legacy_advanced_'+n+'/completion_dispositions.json' for n in ['spsc','history','identity','reclamation','grace','kernel']}
@functools.lru_cache(None)
def cached(p,size,mtime):return hashlib.sha256(p.read_bytes()).hexdigest()
def sha(p):
 s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def build():
 original=read(ORIGINAL);files={ORIGINAL,pathlib.Path(__file__).relative_to(ROOT).as_posix()};packages={};reports=set();rows=[]
 for name,path in PACKAGES.items():
  package=read(path);packages[name]=package;files.add(path)
  for n,h in package['source_sha256'].items():assert (ROOT/n).is_file() and sha(ROOT/n)==h,('stale child',name,n)
  for report in package['current_reports']:
   r=read(report);assert r.get('status')=='pass' and r.get('commands') and all(c['passed'] for c in r['commands']),report
   for key in ['source_sha256','outputs_sha256']:
    for n,h in r.get(key,{}).items():assert (ROOT/n).is_file() and sha(ROOT/n)==h,('stale child evidence',report,n)
   for n,h in r.get('external_input_sha256',{}).items():assert pathlib.Path(n).is_file() and sha(pathlib.Path(n))==h,('stale external',n)
   reports.add(report);files.add(report)
 for ident,names in [('LRUN-18',['spsc','history','identity','reclamation','grace']),('LRUN-19',['kernel'])]:
  parent=next(x for x in original['gaps'] if x['id']==ident);topics=set(parent['topics']);expected=set();objectives=set()
  for p in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T26-*.json')):
   c=json.loads(p.read_text(encoding='utf8'));chosen=[x for x in c['criteria'] if x['objective'].rsplit('.',1)[0] in topics]
   if chosen:files.add(p.relative_to(ROOT).as_posix())
   for x in chosen:expected.add(x['id']);expected.update(v['id'] for v in x['variants']);objectives.add(x['objective'])
  children=[x for name in names for x in packages[name]['updates']];assert len(children)==len(expected) and {x['id'] for x in children}==expected,(ident,'missing/duplicate child');assert all(x['dimensions']['implementation']['state']=='complete' and x['dimensions']['author_validation']['state']=='passed' for x in children)
  paths=[PACKAGES[n] for n in names];child_reports=sorted({r for n in names for r in packages[n]['current_reports']});dimensions={key:dict(state=state,evidence=e) for key,state,e in [('teaching','complete-for-declared-parent-author-scope',paths),('implementation','complete-for-declared-parent-author-scope',paths),('author_validation','passed-with-child-scope-limitations',child_reports),('external_review','not-run',[]),('independent_learner','not-assessed; original minimum7days unchanged',[]),('physical_observation','not-observed',[])]}
  rows.append(dict(id=ident,domain='T26',original_source=ORIGINAL,original_record=parent,objectives=sorted(objectives),child_ids=sorted(expected),child_packages=paths,dimensions=dimensions,closure_rationale='All exact original parent-topic ordinary and delayed criterion identities have concrete guarded author packages. This refines author implementation/validation only; no independent learner/review or physical completion follows.',artifacts=paths,evidence_limits=['Original parent wording/status and accepted records remain unchanged.','Bounded source/model/native/guest/target-closure evidence preserves each package limit; no universal progress or mastery claim.','Final published C17/C23 and independent/physical dimensions remain separate; no G-LDEPTH-T26 parent is invented.']))
 return dict(schema_version=1,scope='Versioned existing LRUN18/19 aggregation,96 exact ordinary/delayed children',source_sha256={n:sha(ROOT/n) for n in sorted(files)},current_reports=sorted(reports),updates=rows)
def validate(d):return [] if d==build() else ['Aggregate differs from current exact children/original parents']
def main():
 p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');p.add_argument('--self-test',action='store_true');a=p.parse_args();path=HERE/'completion_dispositions.json'
 if not a.check:HERE.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(build(),indent=2)+'\n',encoding='utf8')
 d=json.loads(path.read_text(encoding='utf8'));errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-parent','missing-child','lost-delay','wrong-objective','stale-input','inflated-review','inflated-physical','changed-original']:
   x=copy.deepcopy(d)
   if kind=='missing-parent':x['updates'].pop()
   elif kind=='missing-child':x['updates'][0]['child_ids'].pop()
   elif kind=='lost-delay':x['updates'][0]['child_ids']=[n for n in x['updates'][0]['child_ids'] if not n.endswith('-DELAYED')]
   elif kind=='wrong-objective':x['updates'][0]['objectives']=['unrelated']
   elif kind=='stale-input':x['source_sha256'][next(iter(x['source_sha256']))]='0'*64
   elif kind=='inflated-review':x['updates'][0]['dimensions']['external_review']['state']='passed'
   elif kind=='inflated-physical':x['updates'][0]['dimensions']['physical_observation']['state']='observed'
   else:x['updates'][0]['original_record']['status']='complete'
   rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),children=sum(len(x['child_ids']) for x in d['updates']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
