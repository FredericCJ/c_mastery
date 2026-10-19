"""Exact T16 identity, binding and current evidence guard with falsification."""
import argparse,copy,hashlib,importlib.util,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1];FOLDER=ROOT/'research/v5/expansion/legacy_allocation'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def module(path,name):
 spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def validate(data,report):
 errors=[]
 def need(c,m):
  if not c:errors.append(m)
 expected={};bindings=module(FOLDER/'bindings.py','allocation_guard_bindings').BINDINGS
 for p in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T16-*.json')):
  d=json.loads(p.read_text(encoding='utf8'))
  for c in d['criteria']:
   v=c['variants'][0];expected[c['id']]=(c,v,False,d['project']);expected[v['id']]=(c,v,True,d['project'])
 rows=data.get('updates',[]);ids=[r.get('id') for r in rows];need(len(ids)==len(set(ids))==84 and set(ids)==set(expected),'exact ordinary/delayed identities mismatch')
 for owner,key in [(data,'source_sha256'),(report,'source_sha256'),(report,'outputs_sha256')]:
  pins=owner.get(key,{});need(bool(pins),'missing '+key)
  for p,v in pins.items():need((ROOT/p).is_file() and sha(ROOT/p)==v,'stale '+key+' '+p)
 receipt='research/v5/expansion/legacy_allocation/validation.json';need(data.get('current_reports')==[receipt],'current report identity mismatch')
 need(report.get('status')=='pass' and bool(report.get('commands')) and all(r.get('passed') is True for r in report.get('commands',[])),'passing commands missing')
 commands={r['id']:r for r in report.get('commands',[])}
 need(report.get('source_sha256')==module(ROOT/'tools/run_allocation_followthrough.py','allocation_runner_guard').source_pins(),'exact selected source closure mismatch')
 for compiler in ['gcc','clang']:
  for profile in ['debug','optimized','sanitized']:
   prefix=compiler+'.'+profile+'.'
   for name in ['arena','heap','extents','pools','buddy','backends','placement']:
    r=commands.get(prefix+name,{});need(r.get('passed') is True and r.get('exit_code')==0,'missing positive '+prefix+name);need(commands.get(prefix+name+'.compile',{}).get('passed') is True,'missing compiled target '+prefix+name)
   for name in ['mutant'+str(i) for i in range(1,8)]+[unit+'.learner' for unit in ['arena','heap','extents','pools','buddy','backends']]:
    r=commands.get(prefix+name,{});need(r.get('passed') is True and r.get('exit_code',0)!=0 and 'ASSERT' in r.get('output',''),'missing intended rejection '+prefix+name)
  for key in [compiler+'.benchmark',compiler+'.minimized',compiler+'.strict.link',compiler+'.strict.undefined',compiler+'.strict.execute']:need(commands.get(key,{}).get('passed') is True,'missing audit/model '+key)
 for key in ['tsan.repaired','tsan.racy','finite-oracles']:need(commands.get(key,{}).get('passed') is True,'missing transfer/diagnostic '+key)
 need('ThreadSanitizer: data race' in commands.get('tsan.racy',{}).get('output',''),'intended race diagnostic absent')
 for row in rows:
  ident=row.get('id')
  if ident not in expected:continue
  c,v,delayed,parent=expected[ident];ordinary,transfer,reason=bindings[c['objective']]
  need(row.get('objective')==c['objective'] and row.get('parent_project')==parent,'objective/parent mismatch '+ident);need(row.get('original_task')==(v['task'] if delayed else c['task']),'original task changed '+ident);need(row.get('original_delayed_variant')==v['id'] and row.get('minimum_days')==v['minimum_days'],'delayed policy changed '+ident);need(row.get('closure_rationale')==reason,'rationale mismatch '+ident)
  checks=set()
  for selector in (transfer if delayed else ordinary):
   found={x for x in commands if x.startswith(selector) and not x.endswith('.compile')};need(bool(found),'empty binding '+ident);checks.update(found)
  need(set(row.get('author_validation',{}).get('checks',[]))==checks and row.get('author_validation',{}).get('report')==receipt,'exact check binding mismatch '+ident)
  dimensions=row.get('dimensions',{});need(set(dimensions)=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},'six dimensions absent '+ident)
  for d,s in [('external_review','not-run'),('independent_learner','not-assessed')]:need(dimensions.get(d)==dict(state=s,evidence=[]),'inflated '+d+' '+ident)
  state='not-observed' if c['objective']=='T16.12.a' else 'not-applicable-to-host-or-object-contract';need(dimensions.get('physical_observation',{}).get('state')==state,'physical evidence inflated '+ident)
 return errors
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--self-test',action='store_true');args=parser.parse_args();data=json.loads((FOLDER/'completion_dispositions.json').read_text(encoding='utf8'));report=json.loads((FOLDER/'validation.json').read_text(encoding='utf8'));errors=validate(data,report);controls=[]
 if args.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','inflated-independent','wrong-binding','missing-positive','missing-race']:
   d=copy.deepcopy(data);r=copy.deepcopy(report)
   if kind in ['missing-criterion','missing-delayed']:d['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind=='stale-source':r['source_sha256'][next(iter(r['source_sha256']))]='0'*64
   elif kind=='stale-output':r['outputs_sha256'][next(iter(r['outputs_sha256']))]='0'*64
   elif kind=='missing-mutant':r['commands']=[x for x in r['commands'] if x['id']!='gcc.debug.mutant1']
   elif kind=='missing-positive':r['commands']=[x for x in r['commands'] if x['id']!='gcc.debug.heap']
   elif kind=='missing-race':r['commands']=[x for x in r['commands'] if x['id']!='tsan.racy']
   elif kind=='inflated-independent':d['updates'][0]['dimensions']['independent_learner']=dict(state='passed',evidence=['research/v5/expansion/legacy_allocation/validation.json'])
   elif kind=='wrong-binding':d['updates'][0]['author_validation']['checks']=['gcc.version']
   rejected=bool(validate(d,r));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('negative control accepted '+kind)
 print(json.dumps(dict(passed=not errors,updates=len(data['updates']),commands=len(report['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
