"""Execute source-stage investigations; answer checks remain distinct from assessment."""
from datetime import datetime,timezone
from pathlib import Path
import json,shutil,subprocess,sys
from common import ROOT,dump,sha
BASE='research/v5/expansion/legacy_portability_followthrough'
REPORT=BASE+'/lexical_validation.json'
PUBLIC='labs/language_portability/lexical';PRIVATE='assessor/language_portability/lexical'

def observation(d,stage_records=True):
 o=d['observed'];assert o['splice_exit']==1 and o['trigraph_exit']==(1 if d['mode']=='c17' else 9) and o['literals_exit']==0
 assert 'value=9' not in o['splice_preprocessed'] and ('value=9' in o['trigraph_preprocessed'])==(d['mode']=='c23')
 assert 'int separated=7;' in o['literals_preprocessed'] and '"VALUE"' in o['literals_preprocessed'] and '"north:" "gate"' in o['literals_preprocessed']
 assert o['bad_preprocess_exit']==0 and o['bad_compile_exit'] not in [0,None] and 'int, generated;' in o['bad_tokens']
 assert o['repair_exit']==0 and 'int generated;' in o['repair_tokens']
 events=o['dependency_events'];assert len(events)==3 and [r['compiled'] for r in events]==[['a','b','main'],[],['a']] and [r['exit_code'] for r in events]==[15,15,19]
 for n in ['a','b','main']:
  assert events[0]['object_sha256'][n]==events[1]['object_sha256'][n]
  assert (events[1]['object_sha256'][n]!=events[2]['object_sha256'][n])==(n=='a')
  assert any(p.endswith('/config.h') for p in events[0]['dependencies'][n])==(n=='a')
 if stage_records:
  assert len(d['executions'])==23
  expected={2:1,5:1 if d['mode']=='c17' else 9,10:1,18:15,19:15,22:19}
  for i,r in enumerate(d['executions']):assert r['exit_code']==expected.get(i,0)
 return True

def main():
 public=ROOT/PUBLIC;private=ROOT/PRIVATE
 inputs={p.relative_to(ROOT).as_posix():sha(p) for folder in [public,private] for p in folder.iterdir() if p.is_file()}
 for p in ['tools/run_portability_lexical.py','tools/common.py']:inputs[p]=sha(ROOT/p)
 external={};records=[];profiles=[];judgments=[]
 runs=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('lexical-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
 def execute(cmd,label,expected=0):
  cmd=list(map(str,cmd));p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=120);text=p.stdout+p.stderr;log=runs/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_exit=expected,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert p.returncode==expected,text;return text
 for tool in ['gcc','clang']:
  path=Path(shutil.which(tool)).resolve();external[str(path)]=sha(path);execute([tool,'--version'],tool+'-version')
 external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve())
 for cc in ['gcc','clang']:
  for mode in ['c17','c23']:
   for opt in ['O0','O3']:
    label=cc+'-'+mode+'-'+opt;work=runs/label
    execute([sys.executable,public/'experiment.py','--compiler',cc,'--mode',mode,'--optimization=-'+opt,'--output',work],label)
    d=json.loads((work/'report.json').read_text());observation(d)
    for r in d['executions']:
     r=dict(r);r['log']=Path(r['log']).relative_to(ROOT).as_posix();records.append(r)
    for e in d['observed']['dependency_events']:
     for deps in e['dependencies'].values():
      for p,h in deps.items():
       if not Path(p).is_relative_to(ROOT):external[p]=h
    flags=[cc,'-std='+mode,'-'+opt,'-Wall','-Wextra','-pedantic-errors'];control=runs/(label+'-controls');control.mkdir()
    s=control/'no_splice.c';s.write_text((public/'splice.c').read_text().replace('\\\n','\n'),encoding='utf8');exe=control/'no_splice'
    execute([*flags,s,'-o',exe],label+'-splice-control-build');execute([exe],label+'-splice-control-run',9)
    s=control/'merged.c';s.write_text((public/'literals.c').read_text().replace('A/**/B','AB'),encoding='utf8');out=execute([*flags,'-fsyntax-only',s],label+'-merge-control',1);assert 'error:' in out
    exe=control/'stale';execute([cc,*[work/'dependency'/(n+'0.o') for n in ['a','b','main']],'-o',exe],label+'-stale-build');execute([exe],label+'-stale-run',15)
    profiles.append(dict(profile=label,report=(work/'report.json').relative_to(ROOT).as_posix(),report_sha256=sha(work/'report.json'),observed=d['observed'],counter_controls=dict(no_splice=9,merged_diagnostic=True,stale=15),passed=True))
 key=private/'answers.json';answer=json.loads(key.read_text())
 for label,candidate,code in [('unfinished',public/'predictions.json',1),('reference',key,0)]:
  result=json.loads(execute([sys.executable,public/'judge.py',candidate,'--key',key],'judge-'+label,code));judgments.append(dict(kind=label,result=result))
 for field in answer:
  wrong=dict(answer);wrong[field]='wrong';p=runs/('wrong-'+field+'.json');dump(p,wrong);result=json.loads(execute([sys.executable,public/'judge.py',p,'--key',key],'judge-'+field,1));assert result['failed_fields']==[field];judgments.append(dict(kind='wrong-field',field=field,result=result))
 artifacts=[dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)) for p in runs.rglob('*') if p.is_file()]
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 dump(ROOT/REPORT,dict(schema_version=1,status='passed',passed=True,producer='tools/run_portability_lexical.py',input_sha256=inputs,external_input_sha256=external,source_stable=stable,external_inputs_stable=extstable,generated_artifacts=artifacts,executions=records,profiles=profiles,judgments=judgments,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Eight real compiler stage profiles;not a complete preprocessor implementation or language-conformance suite.','Independent expected semantic derivation is private;answer comparison is not assessment of reasoning.','Exact published C17/C23 unavailable;N1570/N3096 remain drafts.']))
 print(json.dumps(dict(passed=True,commands=len(records),profiles=len(profiles),judgments=len(judgments))))
if __name__=='__main__':main()
