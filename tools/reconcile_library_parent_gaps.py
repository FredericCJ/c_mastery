"""Versioned parent aggregation; original gap records and frozen packages stay intact."""
import argparse,copy,hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1];FOLDER=ROOT/'research/v5/expansion/legacy_parent_reconciliation';ORIGINAL='research/v5/expansion/legacy_api/gap_ledger.json'
PACKAGES={'T20':'research/v5/expansion/legacy_architecture/completion_dispositions.json','T22':'research/v5/expansion/legacy_packaging/completion_dispositions.json'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def build():
 original=json.loads((ROOT/ORIGINAL).read_text(encoding='utf8'));files={ORIGINAL,pathlib.Path(__file__).relative_to(ROOT).as_posix()};rows=[];reports=[]
 for domain,path in PACKAGES.items():
  package=json.loads((ROOT/path).read_text(encoding='utf8'));files.add(path)
  for name,h in package['source_sha256'].items():assert (ROOT/name).is_file() and sha(ROOT/name)==h,('stale child',name)
  parent=next(g for g in original['gaps'] if g['id']=='G-LDEPTH-'+domain);children=package['updates'];assert len(children)==60 and {r['objective'] for r in children}==set(parent['objectives'])
  assert all(r['dimensions']['author_validation']['state']=='passed' and r['dimensions']['implementation']['state']=='complete' for r in children)
  for report in package['current_reports']:
   reports.append(report);files.add(report);d=json.loads((ROOT/report).read_text(encoding='utf8'));assert d['status']=='pass' and d['commands'] and all(x['passed'] for x in d['commands'])
  dimensions={k:dict(state=state,evidence=evidence) for k,state,evidence in [('teaching','complete',[path]),('implementation','complete',[path]),('author_validation','passed',package['current_reports']),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-observed',[path])]}
  rows.append(dict(id=parent['id'],domain=domain,original_record=parent,dimensions=dimensions,child_disposition_package=path,child_ids=sorted(r['id'] for r in children),closure_rationale='All exact parent objectives have guarded ordinary and changed-condition author packages. This refines only the author dimensions; original learner, external source/review and applicable physical obligations remain open.',artifacts=[path],evidence_limits=['Original parent wording, objective list and historical status preserved verbatim.','Source-access gaps and independent learner/review are not closed by authored test evidence.','No physical observation inferred from model, compiler or emulator results.']))
 return dict(schema_version=1,scope='Versioned aggregation of existing G-LDEPTH-T20/T22 parent IDs only',source_sha256={p:sha(ROOT/p) for p in sorted(files)},current_reports=sorted(set(reports)),updates=rows,domain_accounting={'T13-T17':'No G-LDEPTH parent record naming these domains was found in canonical gap_ledger/remaining_work JSON; existing exact criterion packages remain the completion evidence. No parent ID invented.'})
def validate(d):
 expected=build();errors=[]
 if d!=expected:errors.append('parent aggregate differs from exact current children/original records')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');p.add_argument('--self-test',action='store_true');a=p.parse_args();path=FOLDER/'completion_dispositions.json'
 if not a.check:FOLDER.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(build(),indent=2)+'\n',encoding='utf8')
 d=json.loads(path.read_text(encoding='utf8'));errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-parent','missing-child','stale-input','inflated-review','inflated-physical','changed-original']:
   x=copy.deepcopy(d)
   if kind=='missing-parent':x['updates'].pop()
   elif kind=='missing-child':x['updates'][0]['child_ids'].pop()
   elif kind=='stale-input':x['source_sha256'][next(iter(x['source_sha256']))]='0'*64
   elif kind=='inflated-review':x['updates'][0]['dimensions']['external_review']['state']='passed'
   elif kind=='inflated-physical':x['updates'][0]['dimensions']['physical_observation']['state']='observed'
   else:x['updates'][0]['original_record']['status']='complete'
   rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('bad control '+kind)
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
