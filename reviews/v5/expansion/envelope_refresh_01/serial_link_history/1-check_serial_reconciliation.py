"""Falsify missing original IDs, misleading scope and stale scoped evidence."""
import argparse,copy,functools,json,pathlib,sys
from close_serial_reconciliation import ROOT,HERE,MAP,BASE,rows,brief,read,LIMITS,sha
@functools.lru_cache(None)
def cached_sha(name,size,mtime):return sha(ROOT/name)
def current(name):
 p=ROOT/name
 if not p.is_file():return None
 s=p.stat();return cached_sha(name,s.st_size,s.st_mtime_ns)
def validate(d,reports):
 errors=[]
 def need(ok,message):
  if not ok:errors.append(message)
 need(d.get('updates')==rows(),'exact original root/rubric/variant/task/check/delay dimensions')
 need(d.get('evidence_limits')==LIMITS,'all scoped limits')
 need(d.get('remaining_author_work')==['Mixed LRUN23/LRUN24 retain their separate scope; independent delayed learner and physical obligations remain unperformed.'],'remaining separate obligations')
 for suffix in MAP:
  x='X-T'+suffix;_,m,t=brief(x);old=read_brief(HERE/(x+'.before-reconciliation.md'));need(all(m[k]==old[k] for k in ['rubric','objective_coverage','delayed_transfer','id','project','objectives']),'preserved original assessment '+x)
  contracts={BASE[token.split(':')[0]][1] for spec,_ in MAP[suffix].values() for token in spec.split()};need(contracts<=set(m['shared_contracts']),'required published contract '+x)
 need(bool(d.get('source_sha256')),'nonempty input identity')
 for n,h in d.get('source_sha256',{}).items():need(current(n)==h,'stale input '+n)
 for name,r in reports.items():
  need(r==read(ROOT/name),'source-bound report identity '+name)
  need(r.get('status') in ['pass','passed'] and r.get('commands') and all(q.get('passed') is True for q in r['commands']),'successful actual campaign '+name)
  pin=r.get('source_sha256',r.get('inspected_paths',{}));need(bool(pin),'scope pins '+name)
  for n,h in pin.items():need(current(n)==h,'stale campaign scope '+n)
  for n,h in r.get('outputs_sha256',{}).items():need(current(n)==h,'stale observation '+n)
  for q in r['commands']:
   if 'output_sha256' in q:need(current(q['output'])==q['output_sha256'],'command output '+str(q.get('id')))
   if 'exit_code' in q:need(q['exit_code']==q['expected_exit'],'actual exit '+str(q.get('id')))
  if 'inspected_paths' in r:
   tests=[q for q in r['commands'] if 'ctest' in q.get('command',[])];need(tests and all(q['returncode']==q['expected'] and '100% tests passed' in q.get('stdout','') for q in tests),'actual original CTest anchor/mutant reports '+name)
 for row in d.get('updates',[]):
  for selected in row.get('author_validation',{}).get('scoped_evidence',[]):
   r=reports.get(selected['report'],{});available={q.get('id','command:'+str(i)) for i,q in enumerate(r.get('commands',[]))};need(selected.get('checks') and set(selected['checks'])<=available,'actual selected case '+row['id'])
 exp=reports.get('research/v5/expansion/legacy_serial_reconciliation/exports.json',{});need(len(exp.get('commands',[]))==10 and {q.get('exercise') for q in exp.get('commands',[])}=={'X-T'+s for s in MAP},'actual10 exported original exercises');need(all(q.get('assessor_files')==0 for q in exp.get('commands',[])),'learner/assessor separation')
 return errors

def read_brief(p):return json.loads(p.read_text(encoding='utf8').split('```json')[1].split('```')[0])
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');reports={n:read(ROOT/n) for n in d['current_reports']};errors=validate(d,reports);controls=[]
 if a.self_test and not errors:
  for name in ['missing-root','missing-rubric','missing-variant','changed-task','wrong-delay','irrelevant-check','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','missing-campaign','missing-mutant','failed-command','missing-export','lost-limits','lost-remaining']:
   bad=copy.deepcopy(d);rs=copy.deepcopy(reports)
   if name.startswith('missing-') and name.split('-',1)[1] in ['root','rubric','variant']:
    kind=name.split('-',1)[1];i=next(i for i,q in enumerate(bad['updates']) if (q['id'].count('-')==2 if kind=='root' else '-R' in q['id'] if kind=='rubric' else '-V' in q['id']));bad['updates'].pop(i)
   elif name=='changed-task':bad['updates'][1]['original_task']='easier replacement'
   elif name=='wrong-delay':bad['updates'][1]['minimum_days']=0
   elif name=='irrelevant-check':bad['updates'][1]['author_validation']['scoped_evidence'][0]['checks']=['gcc.version']
   elif name.startswith('inflated-'):bad['updates'][0]['dimensions'][{'review':'external_review','learner':'independent_learner','physical':'physical_observation'}[name[9:]]]['state']='passed'
   elif name=='stale-source':bad['source_sha256'][next(iter(bad['source_sha256']))]='0'*64
   elif name=='stale-output':r=rs[BASE['can'][0]];r['outputs_sha256'][next(iter(r['outputs_sha256']))]='0'*64
   elif name=='missing-campaign':rs[BASE['b45'][0]]['commands']=[]
   elif name=='missing-mutant':r=rs[BASE['can'][0]];r['commands']=[q for q in r['commands'] if q['id']!='gcc.debug.mutation.CRC']
   elif name=='failed-command':rs[BASE['can'][0]]['commands'][0]['passed']=False
   elif name=='missing-export':rs['research/v5/expansion/legacy_serial_reconciliation/exports.json']['commands'].pop()
   elif name=='lost-limits':bad['evidence_limits']=[]
   else:bad['remaining_author_work']=[]
   rejected=bool(validate(bad,rs));controls.append(dict(control=name,rejected=rejected))
   if not rejected:errors.append('control accepted '+name)
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
