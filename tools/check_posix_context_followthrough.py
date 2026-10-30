"""Exact original-task/current-evidence guard with adversarial negative controls."""
import argparse,copy,hashlib,importlib.util,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_posix_context'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def module(p,name):
 spec=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def external(paths):
 if sys.platform=='win32':
  code='import json,pathlib,hashlib,sys;print(json.dumps({n:hashlib.sha256(pathlib.Path(n).read_bytes()).hexdigest() if pathlib.Path(n).is_file() else None for n in json.loads(sys.argv[1])}))'
  r=subprocess.run(['wsl','-e','python3','-c',code,json.dumps(list(paths))],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,check=True);return json.loads(r.stdout)
 return {n:sha(pathlib.Path(n)) if pathlib.Path(n).is_file() else None for n in paths}
def validate(data,report,actual_external,preserved=None):
 errors=[]
 def need(ok,message):
  if not ok:errors.append(message)
 binding_path=HERE/'launcher_binding_successor.json'
 if preserved is None:preserved=json.loads(binding_path.read_text(encoding='utf8'))
 rebind=module(ROOT/'tools/rebind_posix_launcher_metadata.py','pcguardrebind')
 need(not rebind.validate(preserved),'exact preserved launcher source/report/canonical binding')
 b=module(HERE/'bindings.py','pcguardbindings');runner=module(ROOT/'tools/run_posix_context_followthrough.py','pcguardrunner');expected={}
 for n in ['01','05','06']:
  d=json.loads((ROOT/f'curriculum/v5/expansion/legacy_assessments/PR-T23-{n}.json').read_text(encoding='utf8'))
  for c in d['criteria']:
   if c['objective'] in b.BINDINGS:
    v=c['variants'][0];expected[c['id']]=(c,v,False,d['project']);expected[v['id']]=(c,v,True,d['project'])
 rows=data.get('updates',[]);ids=[x.get('id') for x in rows];need(len(ids)==len(set(ids))==31 and set(ids)==set(expected)|{'LRUN-08'},'exact original criterion/delayed/parent IDs')
 for owner,key in [(data,'source_sha256'),(report,'source_sha256'),(report,'outputs_sha256')]:
  need(bool(owner.get(key)),'missing '+key)
  for p,h in owner.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+key+' '+p)
 need(report.get('source_sha256')==runner.pins(),'exact current source closure')
 need(report.get('status')=='pass' and report.get('failure') is None and report.get('commands') and all(x.get('passed') is True for x in report['commands']),'passing source-bound campaign')
 need(report.get('external_source_sha256')==actual_external and all(actual_external.values()),'external source identity')
 commands={x['id']:x for x in report.get('commands',[])};need(len(commands)==len(report.get('commands',[])),'unique command IDs')
 intended={'process':'r.kind==1&&r.value==0','storage':'e==(failure?EIO:0)','timing':'pc_remaining(d,105)==15','capability':'pc_operation_status(ENOSYS,0)==1','reap':'t.phase==1&&t.pid==pid'}
 for cc in ['gcc','clang']:
  for profile in ['debug','optimized','sanitized']:
   prefix=cc+'.'+profile
   for case in intended:
    p=commands.get(prefix+'.'+case,{});need(p.get('exit_code')==0 and commands.get(prefix+'.'+case+'.compile',{}).get('passed') is True,'positive '+prefix+'.'+case)
    for kind in ['mutation','learner']:
     r=commands.get(prefix+'.'+kind+'.'+case,{});need(r.get('exit_code')==41 and commands.get(prefix+'.'+kind+'.'+case+'.compile',{}).get('passed') is True,'compiled semantic rejection '+prefix+'.'+kind+'.'+case)
     path=ROOT/r.get('output','missing');text=path.read_text(encoding='utf8') if path.is_file() else '';need('PC_ASSERT:' in text,'missing assertion '+prefix+'.'+kind+'.'+case)
     if kind=='mutation':need(''.join(intended[case].split()) in ''.join(text.split()),'wrong mutation assertion '+prefix+'.'+case)
  for probe in ['pipe2','openat','clock']:
   for mode,code in [('hidden',1),('visible',0)]:need(commands.get('feature.'+cc+'.'+probe+'.'+mode,{}).get('exit_code')==code,'declaration evidence '+cc+'.'+probe+'.'+mode)
 for name in ['environment.kernel','environment.libc','environment.mount','gcc.version','clang.version','models']:need(commands.get(name,{}).get('passed') is True,'missing '+name)
 observations=report.get('observations',[]);timing=[x for x in observations if x.get('kind')=='timing'];storage=[x for x in observations if x.get('kind')=='storage'];need(len(timing)==6 and len(storage)==6,'actual observations per profile')
 for t in timing:need(len(t.get('samples',[]))==12 and t.get('upper_bound_claim') is False,'timing scope')
 for s in storage:need(s.get('process_cuts')==4 and s.get('physical_persistence') is False and s.get('filesystem')=='ef53','declared actual filesystem and cut scope')
 exports=json.loads((HERE/'exports.json').read_text(encoding='utf8'));need(exports.get('status')=='pass' and len(exports.get('commands',[]))==9 and all(x.get('passed') is True for x in exports['commands']),'three actual learner exports')
 for key in ['source_sha256','outputs_sha256']:
  for p,h in exports.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale export '+p)
 gate=json.loads((HERE/'research_gate.json').read_text(encoding='utf8'))
 for p,h in gate.get('research_input_sha256',{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale source gate '+p)
 receipt='research/v5/expansion/legacy_posix_context/validation.json';prior='reviews/v5/expansion/legacy_posix_followthrough/launch_validation.json';need(data.get('current_reports')==[receipt,'research/v5/expansion/legacy_posix_context/exports.json',prior],'exact current reports')
 for row in rows:
  ident=row.get('id');dims=row.get('dimensions',{});need(set(dims)=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},'six dimensions '+str(ident))
  for dim,state in [('external_review','not-run'),('independent_learner','not-assessed')]:need(dims.get(dim)==dict(state=state,evidence=[]),'inflated '+dim+' '+str(ident))
  for dim,state in [('teaching','complete'),('implementation','complete'),('author_validation','passed')]:need(dims.get(dim,{}).get('state')==state and bool(dims.get(dim,{}).get('evidence')),'unsupported '+dim+' '+str(ident))
  if ident in expected:
   c,v,delayed,project=expected[ident];normal,changed,why=b.BINDINGS[c['objective']];kind=changed if delayed else normal
   need(row.get('objective')==c['objective'] and row.get('parent_project')==project,'objective binding '+ident);need(row.get('original_task')==(v['task'] if delayed else c['task']),'original task '+ident);need(row.get('minimum_days')==v['minimum_days'] and row.get('original_delayed_variant')==v['id'],'exact delay '+ident);need(row.get('closure_rationale')==why,'rationale '+ident);need(row.get('artifacts')==b.ARTIFACTS,'exact analysis artifacts '+ident)
   need(row.get('author_validation',{}).get('checks')==b.checks(commands,kind) and row.get('author_validation',{}).get('report')==receipt,'exact concrete checks '+ident)
   if c['objective']=='T23.02.a':need(row.get('preserved_evidence')==dict(binding='research/v5/expansion/legacy_posix_context/launcher_binding_successor.json',report=prior,scope='Unchanged launcher/error-channel execution plus the new owned-child hard-wait recovery adapter; no execution rerun or independent learner inference.'),'composed original launcher evidence '+ident)
   need(dims.get('physical_observation',{}).get('state')==('not-observed' if c['objective'].startswith(('T23.12','T23.13')) else 'not-applicable-to-declared-host-or-analysis-contract'),'physical scope '+ident)
  elif ident=='LRUN-08':
   original=next(g for g in json.loads((ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json').read_text(encoding='utf8'))['gaps'] if g['id']==ident);need(row.get('original_record')==original,'parent identity');need(dims.get('physical_observation',{}).get('state')=='not-observed','parent physical scope')
 need(data.get('remaining_cross_scope')==[dict(id='LRUN-09',completed_here=['T23.13','T23.14'],remaining=['T24.09 actual accumulated timerfd/eventfd counts'],reason='No whole-parent closure from only T23 child evidence.')],'cross-scope parent not overstated')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((HERE/'completion_dispositions.json').read_text(encoding='utf8'));r=json.loads((HERE/'validation.json').read_text(encoding='utf8'));actual=external(r.get('external_source_sha256',{}));errors=validate(d,r,actual);controls=[]
 if a.self_test and not errors:
  for kind in ['omitted-criterion','omitted-delayed','stale-source','stale-output','stale-external','missing-mutant','missing-feature','wrong-binding','inflated-review','inflated-learner','inflated-physical','physical-cut-claim','timing-bound-claim','parent-scope','changed-task','missing-preserved-evidence','altered-prior-claim']:
   x=copy.deepcopy(d);y=copy.deepcopy(r);preserved=None
   if kind.startswith('omitted'):x['updates'].pop(1 if kind=='omitted-delayed' else 0)
   elif kind.startswith('stale'):
    key={'stale-source':'source_sha256','stale-output':'outputs_sha256','stale-external':'external_source_sha256'}[kind];y[key][next(iter(y[key]))]='0'*64
   elif kind.startswith('missing'):
    name='gcc.debug.mutation.storage' if kind=='missing-mutant' else 'feature.gcc.pipe2.hidden';y['commands']=[q for q in y['commands'] if q['id']!=name]
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):
    dim={'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind];x['updates'][0]['dimensions'][dim]['state']='passed'
   elif kind=='physical-cut-claim':next(q for q in y['observations'] if q['kind']=='storage')['physical_persistence']=True
   elif kind=='timing-bound-claim':next(q for q in y['observations'] if q['kind']=='timing')['upper_bound_claim']=True
   elif kind=='parent-scope':x['remaining_cross_scope']=[]
   elif kind=='missing-preserved-evidence':next(q for q in x['updates'] if q.get('objective')=='T23.02.a').pop('preserved_evidence')
   elif kind=='altered-prior-claim':
    preserved=json.loads((HERE/'launcher_binding_successor.json').read_text(encoding='utf8'));preserved['updates'][0]['dimensions']['independent_learner']['state']='passed'
   else:x['updates'][0]['original_task']='generic easier replacement'
   rejected=bool(validate(x,y,actual,preserved));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('negative control accepted '+kind)
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
