"""Reject missing, stale, misbound or inflated T17 evidence."""
import argparse,copy,hashlib,importlib.util,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1];FOLDER=ROOT/'research/v5/expansion/legacy_numerics'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def module(p,name):
 spec=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def validate(data,report):
 errors=[]
 def need(c,m):
  if not c:errors.append(m)
 bindings=module(FOLDER/'bindings.py','numeric_guard_bindings').BINDINGS;runner=module(ROOT/'tools/run_numerical_followthrough.py','numeric_guard_runner');expected={}
 for p in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T17-*.json')):
  d=json.loads(p.read_text(encoding='utf8'))
  for c in d['criteria']:
   v=c['variants'][0];expected[c['id']]=(c,v,False,d['project']);expected[v['id']]=(c,v,True,d['project'])
 rows=data.get('updates',[]);ids=[r.get('id') for r in rows];need(len(ids)==len(set(ids))==60 and set(ids)==set(expected),'exact ordinary/delayed identity mismatch')
 for owner,key in [(data,'source_sha256'),(report,'source_sha256'),(report,'outputs_sha256')]:
  pins=owner.get(key,{});need(bool(pins),'missing '+key)
  for p,v in pins.items():need((ROOT/p).is_file() and sha(ROOT/p)==v,'stale '+key+' '+p)
 receipt='research/v5/expansion/legacy_numerics/validation.json';need(data.get('current_reports')==[receipt,'research/v5/expansion/legacy_numerics/exports.json'],'current report identity mismatch');need(report.get('status')=='pass' and bool(report.get('commands')) and all(r.get('passed') is True for r in report.get('commands',[])),'passing commands missing');need(report.get('source_sha256')==runner.source_pins(),'exact source closure mismatch')
 exported=json.loads((FOLDER/'exports.json').read_text(encoding='utf8'));need(exported.get('status')=='pass' and len(exported.get('commands',[]))==12 and all(x.get('passed') is True for x in exported.get('commands',[])),'export commands missing')
 for key in ['source_sha256','outputs_sha256']:
  for name,value in exported.get(key,{}).items():need((ROOT/name).is_file() and sha(ROOT/name)==value,'stale export '+name)
 commands={r['id']:r for r in report.get('commands',[])}
 for compiler in ['gcc','clang']:
  for profile in ['debug','optimized','sanitized']:
   prefix=compiler+'.'+profile+'.'
   for name in [*runner.UNITS,'oracle']:
    r=commands.get(prefix+name,{});need(r.get('passed') is True and r.get('exit_code')==0,'missing positive '+prefix+name);need(commands.get(prefix+name+'.compile',{}).get('passed') is True,'missing compiled target '+prefix+name)
   for name in ['mutant'+str(i) for i in runner.MUTANTS]+[unit+'.learner' for unit in runner.UNITS]+['filter.producer-learner']:
    r=commands.get(prefix+name,{});need(r.get('passed') is True and r.get('exit_code',0)!=0 and 'ND_ASSERT' in r.get('output',''),'missing intended rejection '+prefix+name)
  for key in [compiler+'.benchmark',compiler+'.benchmark.disassembly',compiler+'.environment.strict',compiler+'.environment.relaxed',compiler+'.profileC.closed',compiler+'.profileC.execute']:need(commands.get(key,{}).get('passed') is True,'missing audit/changed condition '+key)
  need(not commands.get(compiler+'.profileC.closed',{}).get('output','').strip(),'profile-C helper closure failed')
  strict=report.get('observations',{}).get(compiler+'.environment.strict',{});need(strict.get('fast_math')==0,'strict arithmetic flag missing');need(strict.get('three_workers')=='0x0p+0','worker-count experiment missing')
 for key in ['gcc.environment.x87','finite-oracles']:need(commands.get(key,{}).get('passed') is True,'missing transfer '+key)
 for row in rows:
  ident=row.get('id')
  if ident not in expected:continue
  c,v,delayed,parent=expected[ident];ordinary,transfer,reason=bindings[c['objective']]
  need(row.get('objective')==c['objective'] and row.get('parent_project')==parent,'objective/parent mismatch '+ident);need(row.get('original_task')==(v['task'] if delayed else c['task']),'original task changed '+ident);need(row.get('original_delayed_variant')==v['id'] and row.get('minimum_days')==v['minimum_days'],'delayed policy changed '+ident);need(row.get('closure_rationale')==reason,'rationale mismatch '+ident)
  checks=set()
  for selector in (transfer if delayed else ordinary):
   matches={x for x in commands if x.startswith(selector) and not x.endswith('.compile')};need(bool(matches),'empty binding '+ident);checks.update(matches)
  need(set(row.get('author_validation',{}).get('checks',[]))==checks and row.get('author_validation',{}).get('report')==receipt,'exact check binding mismatch '+ident)
  dims=row.get('dimensions',{});need(set(dims)=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},'six dimensions absent '+ident)
  for dim,state in [('external_review','not-run'),('independent_learner','not-assessed')]:need(dims.get(dim)==dict(state=state,evidence=[]),'inflated '+dim+' '+ident)
  need(dims.get('physical_observation',{}).get('state')=='not-applicable-to-host-or-analysis-contract','physical evidence inflated '+ident)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');args=p.parse_args();data=json.loads((FOLDER/'completion_dispositions.json').read_text(encoding='utf8'));report=json.loads((FOLDER/'validation.json').read_text(encoding='utf8'));errors=validate(data,report);controls=[]
 if args.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','missing-positive','inflated-independent','wrong-binding','missing-worker','unclosed-profile-C']:
   d=copy.deepcopy(data);r=copy.deepcopy(report)
   if kind in ['missing-criterion','missing-delayed']:d['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind=='stale-source':r['source_sha256'][next(iter(r['source_sha256']))]='0'*64
   elif kind=='stale-output':r['outputs_sha256'][next(iter(r['outputs_sha256']))]='0'*64
   elif kind=='missing-mutant':r['commands']=[x for x in r['commands'] if x['id']!='gcc.debug.mutant1']
   elif kind=='missing-positive':r['commands']=[x for x in r['commands'] if x['id']!='gcc.debug.solve']
   elif kind=='inflated-independent':d['updates'][0]['dimensions']['independent_learner']=dict(state='passed',evidence=['research/v5/expansion/legacy_numerics/validation.json'])
   elif kind=='wrong-binding':d['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind=='missing-worker':r['observations']['gcc.environment.strict'].pop('three_workers',None)
   elif kind=='unclosed-profile-C':next(x for x in r['commands'] if x['id']=='gcc.profileC.closed')['output']=' U malloc\n'
   rejected=bool(validate(d,r));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('negative control accepted '+kind)
 print(json.dumps(dict(passed=not errors,updates=len(data['updates']),commands=len(report['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
