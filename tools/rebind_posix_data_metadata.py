"""Current PR-T23-02 metadata/export binding; original three supports stay bytes-exact."""
import argparse,copy,datetime,hashlib,json,pathlib,subprocess,sys,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from corpus_export_bindings import capture,errors as export_errors
from check_posix_followthrough import fresh
from check_posix_retry import validate as validate_retry
HERE=ROOT/'research/v5/expansion/legacy_posix_binding_current'
REVIEW=ROOT/'reviews/v5/expansion/posix_binding_current'
CARD='curriculum/v5/expansion/legacy_assessments/PR-T23-02.json'
SPECS=[('io','io_validation.json',149,8),('namespace','namespace_validation.json',132,6),('retry','retry_validation.json',30,6)]
OLD='Reproduces the ordinary case but cannot reject this leaf\u00e2\u20ac\u2122s stated misconception or establish its boundary.'
NEW='Reproduces the ordinary case but cannot reject this leaf\u2019s stated misconception or establish its boundary.'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def preserve(p):
 if p.exists():
  q=REVIEW/'history'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')/p.relative_to(ROOT);q.parent.mkdir(parents=True,exist_ok=True);q.write_bytes(p.read_bytes())
def criterion_projection(old,current):
 projected=copy.deepcopy(old['criteria']);changes=[]
 prior=read(HERE/'pre_restore_PR-T23-02.json')
 for c in prior['criteria']:
  if c['rubric']['1']==OLD:changes.append(dict(criterion=c['id'],field='rubric.1',before=OLD,after=NEW))
 actual=copy.deepcopy(current['criteria'])
 for c in actual:c.pop('successor_support',None)
 if projected!=actual or old['baseline_binding']!=current['baseline_binding']:raise ValueError('Unapproved original task or baseline change')
 if not all(current['execution_support'].get(k)==v for k,v in old['execution_support'].items()):raise ValueError('Historical observation changed')
 if len(changes)!=12:raise ValueError('Unexpected encoding-correction scope')
 return changes
def build():
 current=read(ROOT/CARD);baseline=read(HERE/'baseline_PR-T23-02.json');corrections=criterion_projection(baseline,current)
 export=read(REVIEW/'data_exports.json');assert export['passed'] and not export_errors(export,project_ids=['PR-T23-02'])
 files={pathlib.Path(__file__),HERE/'baseline_PR-T23-02.json',ROOT/CARD,REVIEW/'data_exports.json'}|{ROOT/n for n in export['source_sha256']};rows=[];reports=[];origins=[]
 for name,report_name,count,profiles in SPECS:
  support=ROOT/f'research/v5/expansion/legacy_posix_{name}/support.json';old=read(support);history=HERE/(name+'_original_support.json')
  assert support.read_bytes()==history.read_bytes(),'Historical support changed'
  for n,h in old['source_sha256'].items():
   if n!=CARD:assert sha(ROOT/n)==h,('Old execution/source changed',n)
   files.add(ROOT/n)
  campaign=ROOT/f'reviews/v5/expansion/legacy_posix_{name}/{report_name}';actual=read(campaign)
  if name=='retry':validate_retry(actual)
  else:fresh(actual,count,profiles)
  legacy_guard=ROOT/f'reviews/v5/expansion/legacy_posix_{name}/current_export_guard.json';guard=read(legacy_guard)
  failed=[c['name'] for c in guard['checks'] if not c['passed']]
  assert guard['passed'] and not failed,('Current preservation/export guard failed',name,failed)
  assert len(guard['checks'])==(14 if name=='retry' else 11)
  for n,h in guard['inspected_paths'].items():assert sha(ROOT/n)==h;files.add(ROOT/n)
  for n,h in actual['inspected_paths'].items():assert sha(ROOT/n)==h;files.add(ROOT/n)
  for n,h in actual['log_sha256'].items():assert sha(ROOT/n)==h;files.add(ROOT/n)
  files|={support,history,campaign,legacy_guard,HERE/'pre_restore_PR-T23-02.json'};rows+=copy.deepcopy(old['updates']);reports.append(campaign.relative_to(ROOT).as_posix());origins.append(dict(path=support.relative_to(ROOT).as_posix(),sha256=sha(support),historical_canonical_sha256=old['source_sha256'][CARD],unchanged_source_and_execution=True,current_guard=legacy_guard.relative_to(ROOT).as_posix(),correction='Twelve exact rubric.1 mojibake strings restored to retained baseline; every original criterion body and variant now matches baseline exactly after removing additive successor pointers.'))
 return dict(schema_version=1,scope='Metadata-only current binding for unchanged byte-I/O, namespace and retry supports; no C campaign rerun',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[*reports,(REVIEW/'data_exports.json').relative_to(ROOT).as_posix()],updates=rows,original_supports=origins,encoding_corrections=corrections,evidence_limits=['Original support files and actual C receipts remain byte-identical.','The current card is compared to retained baseline by a twelve-field exact encoding correction, not a generic text normalization.','Fresh CLI export and existing standalone learner rejection checks are separate from old C execution; external/learner/physical dimensions remain unchanged.','Historical full card payloads for the three intermediate supports were not retained; their hashes remain historical evidence, not reconstructed content.'])
def check(d):
 try:return [] if d==build() else ['Exact current/history/task/source/claim binding mismatch']
 except (ValueError,AssertionError,KeyError) as e:return [str(e)]
def export():
 binding=capture(project_ids=['PR-T23-02']);commands=[]
 with tempfile.TemporaryDirectory(prefix='posix current metadata ') as tmp:
  workspace=pathlib.Path(tmp)/'learner';cmd=[sys.executable,'-m','cmastery','init','PR-T23-02',str(workspace),'--family','posix_lifecycle_depth'];p=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=180);assert p.returncode==0,(p.stdout,p.stderr)
  from envelope_export_inputs import bind_project_exports
  files={ROOT/n for n in binding['source_sha256']};problems=[];bind_project_exports(ROOT,files,[dict(command=cmd,returncode=p.returncode)],problems);assert not problems
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  for n in read(ROOT/CARD)['supplied_artifacts']:assert (workspace/n).read_bytes()==(ROOT/n).read_bytes(),n
  commands.append(dict(command=cmd,exit_code=p.returncode,passed=True,output=p.stdout+p.stderr,assessor_files_absent=True,all_supplied_bytes_verified=True))
  binding['source_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files|{pathlib.Path(__file__)})}
 assert not export_errors(binding,project_ids=['PR-T23-02'])
 path=REVIEW/'data_exports.json';preserve(path);put(path,dict(passed=True,status='passed',**binding,commands=commands,scope='Actual selected family export and complete public asset byte comparison; existing scoped guards execute each exported compiling stub.'))
def main():
 p=argparse.ArgumentParser();p.add_argument('--prepare',action='store_true');p.add_argument('--export',action='store_true');p.add_argument('--write',action='store_true');p.add_argument('--self-test',action='store_true');a=p.parse_args();HERE.mkdir(parents=True,exist_ok=True);REVIEW.mkdir(parents=True,exist_ok=True)
 if a.prepare:
  b=subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+CARD],cwd=ROOT);dest=HERE/'baseline_PR-T23-02.json';assert not dest.exists() or dest.read_bytes()==b;dest.write_bytes(b)
  for name,_,_,_ in SPECS:
   src=ROOT/f'research/v5/expansion/legacy_posix_{name}/support.json';dest=HERE/(name+'_original_support.json');assert not dest.exists() or dest.read_bytes()==src.read_bytes();dest.write_bytes(src.read_bytes())
 if a.export:export()
 path=HERE/'completion_dispositions.json'
 if a.write:preserve(path);put(path,build())
 if a.self_test:
  d=read(path);errors=check(d);controls=[]
  for key in ['missing-row','changed-task','stale-source','inflated-review','lost-history','broaden-correction','invented-run']:
   x=copy.deepcopy(d)
   if key=='missing-row':x['updates'].pop()
   elif key=='changed-task':x['updates'][0]['original_objective']='T23.01.a'
   elif key=='stale-source':x['source_sha256'][CARD]='0'*64
   elif key=='inflated-review':x['updates'][0]['dimensions']['external_review']['state']='passed'
   elif key=='lost-history':x['original_supports'].pop()
   elif key=='broaden-correction':x['encoding_corrections'][0]['field']='task'
   else:x['original_supports'][0]['unchanged_source_and_execution']=False
   controls.append(dict(control=key,rejected=bool(check(x))))
  old=read(HERE/'baseline_PR-T23-02.json');cur=read(ROOT/CARD)
  for key in ['wrong-task','wrong-delay','wrong-rubric']:
   bad=copy.deepcopy(cur)
   if key=='wrong-task':bad['criteria'][0]['task']='easier'
   elif key=='wrong-delay':bad['criteria'][0]['variants'][0]['minimum_days']=0
   else:bad['criteria'][0]['rubric']['1']='generic'
   try:criterion_projection(old,bad);rejected=False
   except ValueError:rejected=True
   controls.append(dict(control=key,rejected=rejected))
  errors+=['vacuous '+c['control'] for c in controls if not c['rejected']];result=dict(passed=not errors,updates=len(d['updates']),negative_controls=controls,errors=errors);put(REVIEW/'data_guard.json',result);print(json.dumps(result,indent=2));return int(bool(errors))
 return 0
if __name__=='__main__':raise SystemExit(main())
