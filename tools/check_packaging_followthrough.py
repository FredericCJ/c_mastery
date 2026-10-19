"""Strict current-source T22 guard, including rejection of inflated evidence."""
import argparse,copy,hashlib,importlib.util,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_packaging'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def module(p,name):
 spec=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def external(paths):
 if sys.platform=='win32':
  code='import json,pathlib,hashlib,sys;print(json.dumps({n:hashlib.sha256(pathlib.Path(n).read_bytes()).hexdigest() if pathlib.Path(n).is_file() else None for n in json.loads(sys.argv[1])}))'
  # External report keys may include repository files in their WSL spelling.
  r=subprocess.run(['wsl','-e','python3','-c',code,json.dumps(list(paths))],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,check=True);return json.loads(r.stdout)
 return {n:sha(pathlib.Path(n)) if pathlib.Path(n).is_file() else None for n in paths}
def validate(data,report,actual_external):
 errors=[]
 def need(ok,message):
  if not ok:errors.append(message)
 bindings=module(HERE/'bindings.py','packaging_guard_bindings').BINDINGS;runner=module(ROOT/'tools/run_packaging_followthrough.py','packaging_guard_runner');expected={}
 for p in (ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T22-*.json'):
  d=json.loads(p.read_text(encoding='utf8'))
  for c in d['criteria']:
   v=c['variants'][0];expected[c['id']]=(c,v,False,d['project']);expected[v['id']]=(c,v,True,d['project'])
 rows=data.get('updates',[]);ids=[r.get('id') for r in rows];need(len(ids)==len(set(ids))==60 and set(ids)==set(expected),'exact original/delayed set')
 for owner,key in [(data,'source_sha256'),(report,'source_sha256'),(report,'outputs_sha256')]:
  need(bool(owner.get(key)),key+' missing')
  for n,h in owner.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale '+key+' '+n)
 need(report.get('source_sha256')==runner.source_pins(),'exact campaign source closure')
 need(report.get('status')=='pass' and report.get('failure') is None and bool(report.get('commands')) and all(x.get('passed') is True for x in report.get('commands',[])),'stable passing commands')
 need(report.get('external_source_sha256')==actual_external and all(actual_external.values()),'actual external source identity')
 commands={x['id']:x for x in report.get('commands',[])}
 for tool in ['cmake','git','python','binutils','kernel','gcc','clang','doxygen']:need(commands.get(tool+'.version',{}).get('passed') is True,'missing actual tool identity '+tool)
 for cc in ['gcc','clang']:
  for profile in ['debug','optimized','sanitized']:
   prefix=cc+'.'+profile+'.'
   for test in ['buffer','lifecycle','abi','rename','scheduler','allocation']:
    need(commands.get(prefix+test,{}).get('exit_code')==0 and commands.get(prefix+test+'.compile',{}).get('passed') is True,'positive '+prefix+test)
   for test in ['mutant1','mutant2','mutant3','learner']:
    r=commands.get(prefix+test,{});need(r.get('exit_code')==1 and 'LC_ASSERT' in r.get('output',''),'intended rejection '+prefix+test)
 for tag in ['release1','release2','release1_repeat','clang_release1','gcc_lto_package','static','model_backend','both_backends']:
  for key in ['configure','build','install','consumer_configure','consumer_build','consumer','symbols','dependencies','reject.lc_private.h','reject.vendor_only.h']:need(commands.get(tag+'.'+key,{}).get('passed') is True,'package obligation '+tag+'.'+key)
  if tag not in ['static','model_backend','both_backends']:
   ffi=report.get('observations',{}).get(tag+'.ffi',{});need(ffi.get('independent_append_oracle_cases')==450 and ffi.get('callback_retained_until_ack') is True and ffi.get('descriptor_tail_preserved') is True,'foreign oracle/lifetime '+tag)
 for key in ['static.missing_dependency','minimal.missing_private_dependency','bad_example.reject','gcc.abi32','gcc.abi32.elf','doxygen.before','doxygen.after','metrics','trace','models','gcc.lto','gcc.lto.disassembly','deployment.old','deployment.replacement','deployment.rollback','bisect.exact.run','bisect.ambiguous.run','bisect.minimize.1','bisect.minimize.2']:need(commands.get(key,{}).get('passed') is True,'missing concrete requirement '+key)
 observations=report.get('observations',{});need(observations.get('same_compiler_reproducibility',{}).get('equal') is True,'same compiler reproducibility absent');need(len(observations.get('bisect.ambiguous',{}).get('culprits',[]))==2,'skip ambiguity erased');need(observations.get('bisect.minimized',{}).get('input_length')==2,'minimal same-failure input');need(observations.get('models',{}).get('rollback_refusal_preserves_both') is True,'rollback safety absent')
 for mode in ['before','after']:need(observations.get('doxygen.'+mode,{}).get('architecture_and_contract_links_resolve') is True,'documentation link missing '+mode)
 for tag,selected in [('release1','HOST'),('model_backend','MODEL'),('both_backends','BOTH')]:need(observations.get(tag+'.backend',{}).get('selected')==selected,'explicit backend selection '+tag)
 exports=json.loads((HERE/'exports.json').read_text(encoding='utf8'));need(exports.get('status')=='pass' and len(exports.get('commands',[]))==12 and all(x.get('passed') is True for x in exports['commands']),'actual four exports')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in exports.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale export '+n)
 gate=json.loads((HERE/'research_gate.json').read_text(encoding='utf8'))
 for n,h in gate.get('research_input_sha256',{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale research '+n)
 receipt='research/v5/expansion/legacy_packaging/validation.json';need(data.get('current_reports')==[receipt,'research/v5/expansion/legacy_packaging/exports.json'],'report identities')
 for row in rows:
  ident=row.get('id')
  if ident not in expected:continue
  c,v,delayed,parent=expected[ident];normal,changed,why=bindings[c['objective']];need(row.get('objective')==c['objective'] and row.get('parent_project')==parent,'objective/parent '+ident);need(row.get('original_task')==(v['task'] if delayed else c['task']),'task changed '+ident);need(row.get('minimum_days')==v['minimum_days'] and row.get('original_delayed_variant')==v['id'],'delay changed '+ident);need(row.get('closure_rationale')==why,'binding rationale '+ident)
  checks=set()
  for selector in changed if delayed else normal:
   matches={x for x in commands if x.startswith(selector) and not x.endswith('.compile')};need(bool(matches),'empty selector '+ident);checks|=matches
  need(row.get('author_validation',{}).get('checks')==sorted(checks) and row.get('author_validation',{}).get('report')==receipt,'exact check binding '+ident)
  dims=row.get('dimensions',{});need(set(dims)=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},'six dimensions '+ident)
  for dim,state in [('external_review','not-run'),('independent_learner','not-assessed')]:need(dims.get(dim)==dict(state=state,evidence=[]),'inflated '+dim+' '+ident)
  need(dims.get('physical_observation',{}).get('state')==('not-observed' if c['objective']=='T22.01.b' else 'not-applicable-to-host-or-analysis-contract'),'physical claim '+ident)
 return errors
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--self-test',action='store_true');args=parser.parse_args();data=json.loads((HERE/'completion_dispositions.json').read_text(encoding='utf8'));report=json.loads((HERE/'validation.json').read_text(encoding='utf8'));actual=external(report.get('external_source_sha256',{}));errors=validate(data,report,actual);controls=[]
 if args.self_test and not errors:
  for kind in ['omitted-criterion','omitted-delayed','stale-source','stale-output','stale-external','missing-mutant','missing-consumer','missing-32bit','wrong-binding','inflated-learner','inflated-physical','lost-ambiguity','lost-rollback']:
   d=copy.deepcopy(data);r=copy.deepcopy(report)
   if kind.startswith('omitted'):d['updates'].pop(1 if kind=='omitted-delayed' else 0)
   elif kind.startswith('stale'):
    key={'stale-source':'source_sha256','stale-output':'outputs_sha256','stale-external':'external_source_sha256'}[kind];r[key][next(iter(r[key]))]='0'*64
   elif kind.startswith('missing'):
    key={'missing-mutant':'gcc.debug.mutant1','missing-consumer':'release1.consumer','missing-32bit':'gcc.abi32'}[kind];r['commands']=[x for x in r['commands'] if x['id']!=key]
   elif kind=='wrong-binding':d['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind=='inflated-learner':d['updates'][0]['dimensions']['independent_learner']=dict(state='passed',evidence=['research/v5/expansion/legacy_packaging/validation.json'])
   elif kind=='inflated-physical':next(x for x in d['updates'] if x['objective']=='T22.01.b')['dimensions']['physical_observation']['state']='observed'
   elif kind=='lost-ambiguity':r['observations']['bisect.ambiguous']['culprits']=r['observations']['bisect.ambiguous']['culprits'][:1]
   elif kind=='lost-rollback':r['observations']['models']['rollback_refusal_preserves_both']=False
   rejected=bool(validate(d,r,actual));controls.append(dict(control=kind,rejected=rejected));need=not rejected
   if need:errors.append('negative control accepted '+kind)
 print(json.dumps(dict(passed=not errors,updates=len(data['updates']),commands=len(report['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
