"""Adversarial source-bound guard for the four exact parser packages."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief,validate
from corpus_export_bindings import errors as export_binding_errors
HERE=ROOT/'research/v5/expansion/corpus_parsing'
IDS=['X-T15-010','X-T15-011','X-T15-012','X-T15-013']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def check(data):
 errors=[]
 expected={x for id in IDS for x in [id,id+'-DELAYED',*[id+'-R'+str(i) for i in range(1,4)]]}
 rows=data.get('updates',[])
 if len(rows)!=20 or {x['id'] for x in rows}!=expected:errors.append('missing or duplicate exact obligation')
 for n,h in data.get('source_sha256',{}).items():
  if not (ROOT/n).is_file() or sha(ROOT/n)!=h:errors.append('stale source/output '+n)
 if not data.get('source_sha256'):errors.append('missing source envelope')
 for row in rows:
  id=row['parent_exercise'];meta,body=parse_brief(ROOT/'curriculum/v5/exercises'/f'{id}.md')
  try:validate(meta,body)
  except ValueError as e:errors.append(str(e))
  if row['parent_project']!=meta['project'] or row['original_delayed_variant']!=id+'-DELAYED' or row['minimum_days']!=7:errors.append('changed binding/delay')
  for key,state in [('external_review','not-run'),('independent_learner','not-assessed'),('physical_observation','not-run')]:
   if row['dimensions'][key]['state']!=state or row['dimensions'][key]['evidence']:errors.append('inflated external evidence')
  if row['dimensions']['implementation']['state']!='complete' or not row['artifacts']:errors.append('lost implementation')
  if not row['evidence_limits']:errors.append('lost limits')
 reports=data.get('current_reports',[])
 if len(reports)!=3:errors.append('missing reports');return errors
 campaign=json.loads((ROOT/reports[0]).read_text());measure=json.loads((ROOT/reports[1]).read_text());exports=json.loads((ROOT/reports[2]).read_text())
 if data.get('_test_campaign') is not None:campaign=data['_test_campaign']
 if data.get('_test_measure') is not None:measure=data['_test_measure']
 if data.get('_test_exports') is not None:exports=data['_test_exports']
 for report in [campaign,measure,exports]:
  if report.get('passed') is not True:errors.append('report did not pass')
  for key in ['source_sha256','inspected_paths','outputs_sha256']:
   for n,h in report.get(key,{}).items():
    if not (ROOT/n).is_file() or sha(ROOT/n)!=h:errors.append('stale report pin')
  for cmd in report.get('commands',report.get('executions',[])):
   if 'log' in cmd and sha(ROOT/cmd['log'])!=cmd['log_sha256']:errors.append('stale command output')
 anchors={'parsing_depth.'+x for x in ['regex','lr','pratt','scannerless']}
 profiles=campaign.get('profiles',[])
 if len(profiles)!=6 or {(p['compiler'],p['profile']) for p in profiles}!={(c,p) for c in ['gcc','clang'] for p in ['debug','release','asan']}:errors.append('missing profile')
 for p in profiles:
  if not p['passed'] or set(p['anchors'])!=anchors:errors.append('missing anchor')
  for anchor in anchors:
   want={anchor.replace('parsing_depth.','parsing_depth.mutation.')+'.'+str(i) for i in [1,2]}
   if not want<=set(p['compiled_mutant_controls']):errors.append('missing intended mutant')
 if {r['test'] for r in campaign.get('learner_stub_rejections',[]) if r['rejected']}!=anchors:errors.append('missing stub rejection')
 if not campaign.get('scope_source_stable'):errors.append('unstable campaign')
 oracle_commands=[x for x in campaign.get('executions',[]) if any('oracle.py' in str(a) for a in x['command']) and '--driver' in x['command']]
 if len(oracle_commands)!=6 or any(x['exit_code']!=0 for x in oracle_commands):errors.append('missing independent C oracle')
 if len(measure.get('samples',[]))!=2 or any(len(s['rows'])!=81 for s in measure.get('samples',[])):errors.append('missing workload samples')
 if 'm4/lr' not in measure.get('undefined_symbols',{}):errors.append('lost target helper audit')
 if {x['exercise'] for x in exports.get('exports',[]) if x['assessor_files_absent'] and x['semantic_rejection']}!=set(IDS):errors.append('missing isolated learner export')
 errors += export_binding_errors(exports,IDS)
 return errors
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--self-test',action='store_true');parser.add_argument('--report');args=parser.parse_args();data=json.loads((HERE/'completion_dispositions.json').read_text());errors=check(data);controls=[]
 if args.self_test:
  tests=[]
  def altered(name,fn):
   x=copy.deepcopy(data);fn(x);tests.append((name,x))
  altered('missing-root',lambda x:x['updates'].pop(0));altered('missing-delayed',lambda x:x['updates'].pop(4));altered('stale-source',lambda x:x['source_sha256'].update({next(iter(x['source_sha256'])):'0'*64}));altered('wrong-parent',lambda x:x['updates'][0].update(parent_project='PR-T15-10'));altered('short-delay',lambda x:x['updates'][0].update(minimum_days=0));altered('lost-limits',lambda x:x['updates'][0].update(evidence_limits=[]))
  for dimension in ['external_review','independent_learner','physical_observation']:altered('inflated-'+dimension,lambda x,k=dimension:x['updates'][0]['dimensions'][k].update(state='passed'))
  for name,fn in [('missing-profile',lambda d:d['profiles'].pop()),('missing-mutant',lambda d:d['profiles'][0]['compiled_mutant_controls'].pop()),('missing-stub',lambda d:d['learner_stub_rejections'].pop()),('missing-oracle',lambda d:d.update(executions=[x for x in d['executions'] if not any('oracle.py' in str(a) for a in x['command'])])),('unstable-input',lambda d:d.update(scope_source_stable=False))]:
   x=copy.deepcopy(data);c=json.loads((ROOT/data['current_reports'][0]).read_text());fn(c);x['_test_campaign']=c;tests.append((name,x))
  for name,key,index,fn in [('missing-measurement','_test_measure',1,lambda d:d['samples'].pop()),('missing-helper-audit','_test_measure',1,lambda d:d['undefined_symbols'].pop('m4/lr')),('assessor-leak','_test_exports',2,lambda d:d['exports'][0].update(assessor_files_absent=False))]:
   x=copy.deepcopy(data);r=json.loads((ROOT/data['current_reports'][index]).read_text());fn(r);x[key]=r;tests.append((name,x))
  for name,x in tests:
   rejected=bool(check(x));controls.append(dict(control=name,rejected=rejected))
   if not rejected:errors.append('vacuous negative control '+name)
 result=dict(passed=not errors,updates=len(data['updates']),negative_controls=controls,errors=errors,limits='Author source/evidence audit, not external review or learner assessment.')
 if args.report:(ROOT/args.report).write_text(json.dumps(result,indent=2)+'\n')
 print(json.dumps(result,indent=2));raise SystemExit(bool(errors))
if __name__=='__main__':main()
