"""T14 exact-pair and evidence guard, including deliberate rejection controls."""
import argparse,copy,hashlib,importlib.util,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
FOLDER=ROOT/'research/v5/expansion/legacy_containers'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def validate(data,report=None):
 errors=[]
 def need(condition,message):
  if not condition:errors.append(message)
 report=report or json.loads((FOLDER/'validation.json').read_text(encoding='utf8'))
 spec=importlib.util.spec_from_file_location('containers_guard_bindings',FOLDER/'bindings.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
 expected={}
 for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T14-*.json')):
  parent=json.loads(path.read_text(encoding='utf8'))
  for criterion in parent['criteria']:
   for delayed in [False,True]:
    variant=criterion['variants'][0];ident=variant['id'] if delayed else criterion['id'];expected[ident]=(criterion,variant,delayed,parent['project'])
 rows=data.get('updates',[]);ids=[r.get('id') for r in rows]
 need(len(ids)==len(set(ids))==90 and set(ids)==set(expected),'original criterion/delayed identity set mismatch')
 for key in ['source_sha256']:
  pins=data.get(key,{});need(bool(pins),'envelope source pins absent')
  for path,value in pins.items():need((ROOT/path).is_file() and sha(ROOT/path)==value,'stale envelope source '+path)
 need(data.get('current_reports')==['research/v5/expansion/legacy_containers/validation.json'],'current report identity mismatch')
 need(report.get('status')=='pass' and bool(report.get('commands')) and all(r.get('passed') is True for r in report.get('commands',[])),'passing command evidence absent')
 for key in ['source_sha256','outputs_sha256']:
  pins=report.get(key,{});need(bool(pins),key+' absent')
  for path,value in pins.items():need((ROOT/path).is_file() and sha(ROOT/path)==value,'stale '+key+' '+path)
 commands={r['id']:r for r in report.get('commands',[])}
 for compiler in ['gcc','clang']:
  for profile in ['debug','optimized','sanitized']:
   for control in ['mutant'+str(i) for i in range(1,8)]+[unit+'.learner' for unit in ['sequence','ordered','indexed','transfer','payload','graph_consumer']]:
    key=compiler+'.'+profile+'.'+control;r=commands.get(key,{})
    need(r.get('passed') is True and r.get('exit_code',0)!=0 and 'ASSERT' in r.get('output',''),'missing intended semantic rejection '+key)
   for unit in ['sequence','ordered','indexed','transfer','payload','graph_consumer','vector','mutant8']:
    key=compiler+'.'+profile+'.'+unit;r=commands.get(key,{})
    need(r.get('passed') is True and r.get('exit_code')==0,'missing positive/model check '+key)
    need(commands.get(key+'.compile',{}).get('passed') is True,'missing compilation '+key)
    if unit=='mutant8':need('"intended_mutant_rejected": true' in r.get('output',''),'wrong minimizer result '+key)
  for key in [compiler+'.benchmark',compiler+'.strict.link',compiler+'.strict.final-undefined']:
   need(commands.get(key,{}).get('passed') is True,'missing measured/audited check '+key)
 need(commands.get('finite-oracles',{}).get('passed') is True,'finite transfer oracle missing')
 required_pins={p.relative_to(ROOT).as_posix() for p in [*(ROOT/'labs/containers_depth').glob('*'),*(ROOT/'assessor/containers_depth').glob('*'),FOLDER/'resources.json',ROOT/'tools/run_containers_followthrough.py'] if p.is_file()}
 need(set(report.get('source_sha256',{}))==required_pins,'exact report source closure mismatch')
 for row in rows:
  ident=row.get('id')
  if ident not in expected:continue
  criterion,variant,delayed,parent=expected[ident];objective=criterion['objective'];ordinary,transfer,analysis=module.BINDINGS[objective]
  need(row.get('objective')==objective and row.get('parent_project')==parent,'parent/objective changed '+ident)
  need(row.get('original_task')==(variant['task'] if delayed else criterion['task']),'original task changed '+ident)
  need(row.get('original_delayed_variant')==variant['id'] and row.get('minimum_days')==variant['minimum_days'],'delay identity/policy changed '+ident)
  need(row.get('closure_rationale')==analysis,'binding analysis changed '+ident)
  checks=set()
  for selector in (transfer if delayed else ordinary):
   matches={x for x in commands if x.startswith(selector) and not x.endswith('.compile')}
   if not matches:matches={x for x in commands if x==selector}
   need(bool(matches),'binding names no actual check '+ident+':'+selector);checks.update(matches)
  need(set(row.get('author_validation',{}).get('checks',[]))==checks,'exact objective check binding mismatch '+ident)
  need(row.get('author_validation',{}).get('report')=='research/v5/expansion/legacy_containers/validation.json','wrong report '+ident)
  dimensions=row.get('dimensions',{})
  need(set(dimensions)=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},'six dimensions absent '+ident)
  for dimension,state in [('external_review','not-run'),('independent_learner','not-assessed')]:need(dimensions.get(dimension)=={'state':state,'evidence':[]},'inflated '+dimension+' '+ident)
  need(dimensions.get('physical_observation',{}).get('state')=='not-applicable-to-host-or-object-contract','physical evidence inflated '+ident)
 return errors
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--self-test',action='store_true');args=parser.parse_args()
 data=json.loads((FOLDER/'completion_dispositions.json').read_text(encoding='utf8'));report=json.loads((FOLDER/'validation.json').read_text(encoding='utf8'));errors=validate(data,report);controls=[]
 if args.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','inflated-independent','wrong-binding','missing-positive']:
   d=copy.deepcopy(data);r=copy.deepcopy(report)
   if kind in ['missing-criterion','missing-delayed']:d['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind=='missing-positive':r['commands']=[x for x in r['commands'] if x['id']!='gcc.debug.payload']
   elif kind=='stale-source':r['source_sha256'][next(iter(r['source_sha256']))]='0'*64
   elif kind=='stale-output':r['outputs_sha256'][next(iter(r['outputs_sha256']))]='0'*64
   elif kind=='missing-mutant':r['commands']=[x for x in r['commands'] if x['id']!='gcc.debug.mutant1']
   elif kind=='inflated-independent':d['updates'][0]['dimensions']['independent_learner']={'state':'passed','evidence':['research/v5/expansion/legacy_containers/validation.json']}
   elif kind=='wrong-binding':d['updates'][0]['author_validation']['checks']=['gcc.version']
   rejected=bool(validate(d,r));controls.append({'control':kind,'rejected':rejected})
   if not rejected:errors.append('negative control accepted '+kind)
 print(json.dumps({'passed':not errors,'updates':len(data['updates']),'commands':len(report['commands']),'negative_controls':controls,'errors':errors},indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
