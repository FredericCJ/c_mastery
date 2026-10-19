import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief,validate
from corpus_export_bindings import errors as export_binding_errors
from tools.expansion_evidence import require_batch_research
HERE=ROOT/'research/v5/expansion/corpus_interval'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def check(data):
 errors=[];id='X-T17-003';rows=data.get('updates',[])
 if len(rows)!=5 or {r['id'] for r in rows}!={id,id+'-DELAYED',*[id+'-R'+str(i) for i in [1,2,3]]}:errors.append('exact identities')
 if not data.get('source_sha256'):errors.append('missing pins')
 for n,h in data.get('source_sha256',{}).items():
  if not (ROOT/n).is_file() or sha(ROOT/n)!=h:errors.append('stale '+n)
 try:
  meta,body=parse_brief(ROOT/'curriculum/v5/exercises'/f'{id}.md');validate(meta,body);require_batch_research(HERE)
 except (ValueError,RuntimeError,AssertionError) as e:errors.append(str(e))
 for r in rows:
  if r['parent_project']!='PR-T17-01' or r['original_delayed_variant']!=id+'-DELAYED' or r['minimum_days']!=7:errors.append('binding')
  if not r['artifacts'] or not r['evidence_limits']:errors.append('lost evidence/limits')
  for k,state in [('external_review','not-run'),('independent_learner','not-assessed'),('physical_observation','not-run')]:
   if r['dimensions'][k]!=dict(state=state,evidence=[]):errors.append('inflated independent evidence')
 reports=data.get('current_reports',[])
 if len(reports)!=2:return errors+['missing reports']
 campaign=data.get('_campaign',json.loads((ROOT/reports[0]).read_text()));exports=data.get('_exports',json.loads((ROOT/reports[1]).read_text()))
 for report in [campaign,exports]:
  if report.get('passed') is not True:errors.append('failed report')
  for key in ['source_sha256','inspected_paths','outputs_sha256']:
   for n,h in report.get(key,{}).items():
    if not (ROOT/n).is_file() or sha(ROOT/n)!=h:errors.append('stale report input/output')
  for cmd in report.get('commands',report.get('executions',[])):
   if 'log' in cmd and sha(ROOT/cmd['log'])!=cmd['log_sha256']:errors.append('stale command log')
 profiles=campaign.get('profiles',[])
 if len(profiles)!=6 or {(r['compiler'],r['profile']) for r in profiles}!={(c,p) for c in ['gcc','clang'] for p in ['debug','release','asan']}:errors.append('profiles')
 for p in profiles:
  if not p['passed'] or p['anchors']!=['numeric_interval.enclosure']:errors.append('anchor')
  if not {'numeric_interval.mutation.enclosure.1','numeric_interval.mutation.enclosure.2'}<=set(p['compiled_mutant_controls']):errors.append('mutants')
 if not campaign.get('scope_source_stable'):errors.append('unstable scope')
 if {x['test'] for x in campaign.get('learner_stub_rejections',[]) if x['rejected']}!={'numeric_interval.enclosure'}:errors.append('stub')
 if len([x for x in campaign.get('executions',[]) if '--driver' in x['command'] and any('oracle.py' in str(a) for a in x['command']) and x['exit_code']==0])!=6:errors.append('independent model')
 if exports.get('exports')!=[dict(exercise=id,assessor_files_absent=True,canonical_contract_valid=True,semantic_rejection=True)]:errors.append('export')
 errors += export_binding_errors(exports,[id])
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--report');p.add_argument('--self-test',action='store_true');a=p.parse_args();data=json.loads((HERE/'completion_dispositions.json').read_text());errors=check(data);controls=[]
 if a.self_test:
  changes=[('missing-root',lambda d:d['updates'].pop(0)),('missing-delay',lambda d:d['updates'].pop()),('stale-pin',lambda d:d['source_sha256'].update({next(iter(d['source_sha256'])):'0'*64})),('wrong-parent',lambda d:d['updates'][0].update(parent_project='PR-T17-02')),('short-delay',lambda d:d['updates'][0].update(minimum_days=0)),('missing-limits',lambda d:d['updates'][0].update(evidence_limits=[]))]
  for k in ['external_review','independent_learner','physical_observation']:changes.append(('inflated-'+k,lambda d,k=k:d['updates'][0]['dimensions'][k].update(state='passed')))
  for name,fn in changes:
   d=copy.deepcopy(data);fn(d);controls.append(dict(control=name,rejected=bool(check(d))))
  for name,fn in [('profile',lambda c:c['profiles'].pop()),('mutant',lambda c:c['profiles'][0]['compiled_mutant_controls'].pop()),('stub',lambda c:c['learner_stub_rejections'].clear()),('oracle',lambda c:c.update(executions=[])),('unstable',lambda c:c.update(scope_source_stable=False))]:
   d=copy.deepcopy(data);d['_campaign']=json.loads((ROOT/data['current_reports'][0]).read_text());fn(d['_campaign']);controls.append(dict(control=name,rejected=bool(check(d))))
  d=copy.deepcopy(data);d['_exports']=json.loads((ROOT/data['current_reports'][1]).read_text());d['_exports']['exports'][0]['assessor_files_absent']=False;controls.append(dict(control='assessor-leak',rejected=bool(check(d))))
  errors+=['vacuous '+c['control'] for c in controls if not c['rejected']]
 result=dict(passed=not errors,updates=len(data['updates']),negative_controls=controls,errors=errors,limits='Author guard; no independent human or learner claim.')
 if a.report:(ROOT/a.report).write_text(json.dumps(result,indent=2)+'\n')
 print(json.dumps(result,indent=2));raise SystemExit(bool(errors))
if __name__=='__main__':main()
