"""Fresh source, exact parent and executable semantic-control validation."""
import argparse,copy,datetime,json
import build_systems_source_followthrough as b
import record_systems_source_followthrough as source
from check_residual_followthrough import external_check

def require(v,m):
 if not v:raise ValueError(m)

def expected():
 rows,mapping,files=b.make();files.add((b.BASE/'evidence_map.json').relative_to(b.ROOT).as_posix());return rows,mapping,{p:b.sha(b.ROOT/p) for p in sorted(files)}

def inspect(support,mapping,campaign,gold):
 rows,wanted,pins=gold
 require(support['updates']==rows and len(rows)==5,'exact original parent and companion dimensions')
 require(mapping==wanted,'exact original source-parent and child bodies')
 require(support['source_sha256']==pins,'current exact source envelope')
 require(campaign['passed'] and len(campaign['checks'])==1044 and len(campaign['commands'])==1082,'complete source-bound campaign')
 require(all(x['passed'] for x in campaign['commands']+campaign['checks']),'failed executed case')
 require(all(b.sha(b.ROOT/p)==h for p,h in campaign['source_sha256'].items()),'stale executable sources')
 names={c['name']:c for c in campaign['commands']}
 for compiler in ['gcc','clang']:
  for suffix in ['SF_MUT_CHECKSUM','SF_MUT_SEGMENT','SF_MUT_ENTRY','SF_MUT_WINDOW','stub']:
   c=names[compiler+'-'+suffix+'-reject'];require(c['exit_code']==1 and 'SOURCE_HEX_ASSERT:' in c['stdout']+c['stderr'],'missing intended compiling semantic rejection')
 for name in ['debug','optimized','sanitized']:
  for compiler in ['gcc','clang']:require(names[compiler+'-'+name+'-test']['exit_code']==0,'missing positive profile')
 resources,captures=source.make_resources();require(resources==b.read(b.BASE/'resources.json'),'inspected source identity/ranges')
 require(len(resources['resources'])==7,'missing actual source')
 inspection=b.read(b.BASE/'inspection.json');require(inspection['external_source_sha256']=={p:h for p,h in captures.items() if p.startswith('/')},'source capture identity')
 gate=b.read(b.BASE/'research_gate.json');require(gate['status']=='passage-inspected' and all(b.sha(b.ROOT/p)==h for p,h in gate['research_input_sha256'].items()),'source gate')
 for r in rows:
  require(r['dimensions']['external_review']['state']=='not-run' and r['dimensions']['independent_learner']['state']=='not-assessed','invented independent evidence')
 require(not {'curriculum/v5/expansion/implementation_updates.json','curriculum/v5/expansion/work_inventory.json'}&pins.keys(),'mutable global source pin')
 return dict(source_parents=3,companion_tasks=2,resources=7,profiles=6,commands=1082,oracle_checks=1044,mutants=8,learner_stub_rejections=2)

def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report',required=True);args=p.parse_args();out=(b.ROOT/args.report).resolve();require(out.is_relative_to((b.ROOT/'reviews/v5/expansion/systems_source_followthrough').resolve()) and not out.exists(),'fresh scoped guard report')
 support=b.read(b.BASE/'support.json');mapping=b.read(b.BASE/'evidence_map.json');campaign=b.read(b.ROOT/b.REPORT);errors=[];controls=[];summary={}
 try:
  gold=expected();summary=inspect(support,mapping,campaign,gold)
  external=external_check({p:[h] for p,h in b.read(b.BASE/'inspection.json')['external_source_sha256'].items()});require(external['passed'],'actual primary sources')
  if args.self_test:
   for name in ['lost-origin','lost-child','changed-clause','missing-parent','invented-learner','invented-review','invented-physical','stale-source','missing-oracle','missing-mutant','wrong-rejection','failed-command','global-pin']:
    s=copy.deepcopy(support);m=copy.deepcopy(mapping);c=copy.deepcopy(campaign)
    if name=='lost-origin':m['bindings'][0]['origins']=[]
    elif name=='lost-child':m['bindings'][0]['selected_source_children'].pop()
    elif name=='changed-clause':m['bindings'][0]['clause_analysis']='whole library proved'
    elif name=='missing-parent':s['updates'].pop()
    elif name.startswith('invented-'):s['updates'][0]['dimensions'][{'invented-learner':'independent_learner','invented-review':'external_review','invented-physical':'physical_observation'}[name]]['state']='passed'
    elif name=='stale-source':s['source_sha256'][next(iter(s['source_sha256']))]='0'*64
    elif name=='missing-oracle':c['checks'].pop()
    elif name=='missing-mutant':c['commands']=[x for x in c['commands'] if 'SF_MUT_WINDOW' not in x['name']]
    elif name=='wrong-rejection':next(x for x in c['commands'] if x['name']=='gcc-SF_MUT_ENTRY-reject')['stderr']='unrelated failure'
    elif name=='failed-command':c['commands'][0]['passed']=False
    else:s['source_sha256']['curriculum/v5/expansion/work_inventory.json']='0'*64
    try:inspect(s,m,c,gold);rejected=False
    except (ValueError,KeyError,AssertionError):rejected=True
    controls.append(dict(control=name,rejected=rejected));require(rejected,'false packet accepted '+name)
  require(all(b.sha(b.ROOT/p)==h for p,h in gold[2].items()),'inputs changed during guard')
 except (ValueError,KeyError,AssertionError) as e:errors.append(str(e))
 result=dict(schema_version=1,created_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),passed=not errors,summary=summary,negative_controls=controls,errors=errors,scope='Source-parent scope and actual parser execution; not independent mastery',input_sha256={p:b.sha(b.ROOT/p) for p in [(b.BASE/'support.json').relative_to(b.ROOT).as_posix(),b.REPORT,'tools/check_systems_source_followthrough.py']})
 out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print(json.dumps(result['summary']|dict(passed=result['passed'],errors=errors)));return int(not result['passed'])
if __name__=='__main__':raise SystemExit(main())
