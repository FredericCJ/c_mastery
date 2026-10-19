"""Require current actual stack, native histories, models and target closure."""
import argparse,copy,json,re
from pathlib import Path
from common import ROOT,sha
from check_expansion_campaigns import generic_errors,pin_errors,execution_errors
DEFAULT='reviews/v5/expansion/corpus_treiber/current_validation.json'
def validate(d,files=True):
 meta=json.loads((ROOT/'labs/corpus_treiber/family.json').read_text());errors=generic_errors('corpus_treiber',meta,d)
 def need(x,label):
  if not x:errors.append(label)
 need(d.get('passed') is True and d.get('scope_source_stable') is True,'successful stable campaign')
 need(d.get('physical_validation')=='not-run' and d.get('learner_mastery')=='not-assessed','evidence boundary')
 controls={'corpus_treiber.mutation.'+a+'.'+str(m) for a,ms in [('stack',[1,2]),('reclaim',[3,4]),('retirement',[3,4])] for m in ms}
 for p in d.get('profiles',[]):need(set(p.get('compiled_mutant_controls',[]))==controls and p.get('ctest_count')==9,'six semantic controls')
 need({(x.get('model'),x.get('mutation'),x.get('outcome')) for x in d.get('model_checks',[])}=={(1,0,'bounded-no-error'),(2,0,'bounded-no-error'),(1,5,'data-race-counterexample'),(2,6,'data-race-counterexample')},'actual finite models and distinct causal controls')
 need(all(x.get('passed') is True and x.get('complete_executions',0)>0 for x in d.get('model_checks',[])),'model observations')
 need(d.get('model_tool',{}).get('model')=='RC11' and 'v0.19.0' in d.get('model_tool',{}).get('version',''),'model identity')
 records={r.get('id'):r for r in d.get('executions',[])}
 for model,mutation in [(1,0),(2,0),(1,5),(2,6)]:
  row=records.get(f'model{model}.mutation{mutation}',{});text=row.get('stdout','');command=row.get('command',[])
  need(row.get('exit_code')==(42 if mutation else 0) and row.get('passed') is True,'model exact observed outcome')
  need(all(flag in command for flag in ['-rc11','-mode=verify','-disable-mm-detector','-disable-estimation','-std=c17','-DTS_MODEL='+str(model),'-DTS_MUTANT='+str(mutation),'-include']) and any(str(x).endswith('assessor/corpus_treiber/treiber.c') for x in command) and command[-1:].pop().endswith('labs/corpus_treiber/model.c') if command else False,'actual implementation/model invocation')
  need('Compilation complete.' in text and 'Transformation complete.' in text and ('Error: Non-atomic race!' in text and 'Verification unsuccesful.' in text if mutation else 'No errors were detected.' in text and 'Verification complete.' in text),'model semantic observation')
  found=re.search(r'Number of complete executions explored: (\d+)',text)
  summary=next((r for r in d.get('model_checks',[]) if r.get('model')==model and r.get('mutation')==mutation),{})
  need(bool(found) and int(found[1])==summary.get('complete_executions'),'raw exploration count')
 need({(r.get('compiler'),r.get('optimization')) for r in d.get('target_audits',[])}=={(c,o) for c in ['arm-none-eabi-gcc','clang'] for o in ['-O0','-O3']},'four strict target audits')
 for r in d.get('target_audits',[]):need(r.get('execution')=='not-run' and r.get('language')=='c17' and r.get('undefined_symbols')==[] and r.get('libraries')==[] and r.get('atomic_instructions')==['ldrex','strex','dmb'],'target closure/scope')
 need(len(d.get('generated_artifact_sha256',{}))==14,'actual artifacts')
 if files:
  errors+=pin_errors(d)+execution_errors(d);oracles=[]
  for r in d.get('executions',[]):
   text=(ROOT/r['log']).read_text() if r.get('log') else r.get('stdout','')
   if '--driver' in r.get('command',[]):oracles.append(text)
   if r.get('id','').endswith(('.undefined','.link-undefined')):need(not text.strip(),'no implicit helper')
  need(len(oracles)==8 and all('"sequential_transitions": 12288' in t and '"actual_concurrent_histories": 256' in t for t in oracles),'all native independent histories')
  for p,h in d.get('generated_artifact_sha256',{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'target/model artifact '+p)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);p.add_argument('--self-test',action='store_true');p.add_argument('--output',type=Path);a=p.parse_args();d=json.loads((ROOT/a.report).read_text());errors=validate(d);controls=[]
 def model_record(x):return next(r for r in x['executions'] if r.get('id')=='model1.mutation5')
 edits=[('failed',lambda x:x.update(passed=False)),('changed',lambda x:x.update(scope_source_stable=False)),('profile',lambda x:x['profiles'].pop()),('anchor',lambda x:x['profiles'][0]['anchors'].pop()),('mutant',lambda x:x['profiles'][0]['compiled_mutant_controls'].pop()),('mapping',lambda x:x['profiles'][0].update(controls_by_anchor={})),('stub',lambda x:x['learner_stub_rejections'].pop()),('model',lambda x:x['model_checks'].pop()),('wrong model',lambda x:x['model_tool'].update(model='SC')),('no exploration',lambda x:x['model_checks'][0].update(complete_executions=0)),('model missing release run',lambda x:model_record(x).update(command=['false'])),('model wrong exit',lambda x:model_record(x).update(exit_code=0)),('model compile failure',lambda x:model_record(x).update(stdout='Compilation failed')),('target',lambda x:x['target_audits'].pop()),('helper',lambda x:x['target_audits'][0].update(undefined_symbols=['__atomic_load'])),('target execution',lambda x:x['target_audits'][0].update(execution='passed')),('artifact',lambda x:x['generated_artifact_sha256'].pop(next(iter(x['generated_artifact_sha256'])))),('physical',lambda x:x.update(physical_validation='passed')),('learner',lambda x:x.update(learner_mastery='passed'))]
 if a.self_test:
  for name,edit in edits:
   c=copy.deepcopy(d);edit(c);rejected=bool(validate(c,False));controls.append(dict(name=name,rejected=rejected))
   if not rejected:errors.append('accepted '+name)
 result=dict(passed=not errors,errors=errors,negative_controls=controls,report=a.report,report_sha256=sha(ROOT/a.report))
 if a.output:a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
 print(json.dumps(result));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
