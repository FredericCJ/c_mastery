"""Read-only exact origin, task, source and transitive evidence guard."""
import argparse,copy,datetime,json
import audit_named_parents as a
import build_named_parents as b
import check_residual_followthrough as t

def evidence_audit(providers):
 audit=t.evidence_audit(providers);resolved=[]
 # This retained producer explicitly makes its vendor paths relative to its
 # recorded external root. Resolve that convention; never waive a missing pin.
 for report in audit['reports']:
  data=a.read(a.ROOT/report)
  if 'external_source_root' not in data:continue
  root=data['external_source_root'];require(root.startswith('/'),'absolute external source root')
  for name,digest in data.get('external_source_sha256',{}).items():
   require(not name.startswith('/') and '..' not in name.split('/'),'contained relative vendor source')
   message=report+'/external_source_sha256: '+name
   if message in audit['missing']:
    audit['missing'].remove(message);path=root.rstrip('/')+'/'+name
    audit['external'].setdefault(path,[]).append(digest);resolved.append(dict(report=report,relative=name,path=path,sha256=digest))
 audit['resolved_external_relative']=resolved
 return audit

def require(ok,message):
 if not ok:raise ValueError(message)

def expected():
 rows,bindings,files,providers=b.rows();files.add((a.BASE/'evidence_map.json').relative_to(a.ROOT).as_posix())
 return rows,dict(schema_version=1,scope='Original parent clauses and complete exact canonical child bindings',bindings=bindings),{p:a.sha(a.ROOT/p) for p in sorted(files)},sorted(providers)

def inspect(support,mapping,audit,gold):
 rows,bindings,pins,providers=gold
 require(support['updates']==rows and len(rows)==7,'exact parent dimensions and remaining requirements')
 require(mapping==bindings,'exact original clause/task/provider binding')
 require(support['source_sha256']==pins,'current exact source set and hashes')
 require(support['providers']==providers==audit['providers'],'exact providers')
 require(not audit['missing'] and not audit['stale'],'missing/stale transitive evidence')
 require(audit['external_validation']['passed'],'external identities')
 require(len(audit.get('resolved_external_relative',[]))==30,'exact thirty FreeRTOS relative vendor pins')
 require(not any(v.get('passed') is False or v.get('status') in ['fail','failed','not-run'] or v.get('source_stable') is False or v.get('scope_source_stable') is False for v in audit['report_statuses'].values()),'failed or unstable receipt')
 require(not {'curriculum/v5/expansion/work_inventory.json','curriculum/v5/expansion/implementation_updates.json'}&pins.keys(),'mutable global pin')
 require(len({c['id'] for r in bindings['bindings'] for c in r['children']})==552,'all exact ordinary and delayed children')
 indexed={r['id']:r for r in rows}
 require(indexed['LRUN-20']['dimensions']['teaching']['state'].startswith('partial') and indexed['G-LCON-PROOFS']['dimensions']['teaching']['state'].startswith('partial'),'unavailable schematic/proof promoted')
 require(not indexed['LSYS-BOOKS']['required_criteria_and_variants'],'source reading promoted to implementation')
 for r in rows:
  require(r['dimensions']['external_review']['state']=='not-run' and r['dimensions']['independent_learner']['state']=='not-assessed' and r['dimensions']['physical_observation']['state']=='not-claimed;original-physical-obligation-retained','invented independent or physical evidence')
 source=a.read(a.ROOT/'reviews/v5/expansion/systems_book_followthrough/validation_02.json')
 require(source['passed'] and len(source['negative_controls'])==8 and all(c['rejected'] for c in source['negative_controls']),'fresh source scope guard')
 return dict(parents=7,children=552,providers=len(providers),partial_source_parents=['LRUN-20','G-LCON-PROOFS'])

def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report',required=True);args=p.parse_args();out=(a.ROOT/args.report).resolve();require(out.is_relative_to((a.ROOT/'reviews/v5/expansion/named_parent_followthrough').resolve()) and not out.exists(),'scoped fresh review path')
 support=a.read(a.BASE/'support.json');mapping=a.read(a.BASE/'evidence_map.json');audit=evidence_audit(support['providers']);errors=[];controls=[];summary={}
 try:
  gold=expected();audit['external_validation']=t.external_check(audit['external']);summary=inspect(support,mapping,audit,gold)
  if args.self_test:
   for name in ['lost-origin','changed-clause','changed-task','lost-child','lost-delayed','zero-delay','wrong-provider','wrong-check','missing-parent','inflated-review','inflated-learner','inflated-physical','erased-source-gap','stale-source','stale-transitive','lost-transitive','failed-receipt','external-unchecked','global-pin','lost-remaining']:
    s=copy.deepcopy(support);m=copy.deepcopy(mapping);v=copy.deepcopy(audit);withchildren=next(r for r in m['bindings'] if r['children'])
    if name=='lost-origin':withchildren['origins']=[]
    elif name=='changed-clause':withchildren['clause_analysis']='heading only'
    elif name=='changed-task':withchildren['children'][0]['original_task']='easier'
    elif name=='lost-child':withchildren['children'].pop()
    elif name=='lost-delayed':withchildren['children']=[c for c in withchildren['children'] if not c['variant']]
    elif name=='zero-delay':next(c for c in withchildren['children'] if c['variant'])['variant']['minimum_days']=0
    elif name=='wrong-provider':withchildren['children'][0]['provider']=support['providers'][-1]
    elif name=='wrong-check':withchildren['children'][0]['checks']=['compiler --version']
    elif name=='missing-parent':s['updates'].pop()
    elif name.startswith('inflated-'):s['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[name]]['state']='passed'
    elif name=='erased-source-gap':next(r for r in s['updates'] if r['id']=='G-LCON-PROOFS')['dimensions']['teaching']['state']='complete'
    elif name=='stale-source':s['source_sha256'][next(iter(s['source_sha256']))]='0'*64
    elif name=='stale-transitive':v['stale']=[dict(path='changed producer')]
    elif name=='lost-transitive':v['missing']=['actual execution log']
    elif name=='failed-receipt':v['report_statuses'][next(iter(v['report_statuses']))]['status']='failed'
    elif name=='external-unchecked':v['external_validation']['passed']=False
    elif name=='global-pin':s['source_sha256']['curriculum/v5/expansion/work_inventory.json']='0'*64
    else:s['updates'][0]['remaining_required_work']=[]
    try:inspect(s,m,v,gold);rejected=False
    except (ValueError,KeyError,AssertionError):rejected=True
    controls.append(dict(control=name,rejected=rejected));require(rejected,'false packet accepted '+name)
  require(all(a.sha(a.ROOT/p)==h for p,h in gold[2].items()) and all(a.sha(a.ROOT/p)==h for p,h in audit['document_sha256'].items()),'inputs changed during guard')
 except (ValueError,KeyError,AssertionError) as e:errors.append(str(e))
 audit.update(schema_version=1,created_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),passed=not errors and not audit['missing'] and not audit['stale'],scope='Exact source/target parent bridge; no new child execution or physical/learner evidence',summary=summary,negative_controls=controls,errors=errors)
 out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(audit,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=audit['passed'],documents=audit['documents'],files=audit['files'],summary=summary,errors=errors)));return int(not audit['passed'])
if __name__=='__main__':raise SystemExit(main())
