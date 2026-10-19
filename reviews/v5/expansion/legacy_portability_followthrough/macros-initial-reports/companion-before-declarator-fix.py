"""Actual macro stage failures, repairs, placemarkers and admitted generated C."""
from datetime import datetime,timezone
from pathlib import Path
import json,shutil,subprocess,sys
from common import ROOT,dump,sha,read_json
BASE='research/v5/expansion/legacy_portability_followthrough';REPORT=BASE+'/macros_companion_validation.json'
def main():
 public=ROOT/'labs/language_portability/macros';private=ROOT/'assessor/language_portability/macros'
 inputs={p.relative_to(ROOT).as_posix():sha(p) for folder in [public,private] for p in folder.iterdir() if p.is_file()}
 for p in ['tools/run_portability_macros_companion.py','tools/common.py','fixtures/language/PR-T06-01.json']:inputs[p]=sha(ROOT/p)
 home=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('macros-companion-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));home.mkdir(parents=True);records=[];profiles=[];external={};registry=[]
 def execute(cmd,label,expected=0):
  cmd=list(map(str,cmd));p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=60);out=p.stdout+p.stderr;log=home/(str(len(records))+'-'+label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_exit=expected,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert p.returncode==expected,out;return p.stdout,p.stderr
 for cc in ['gcc','clang']:
  tool=Path(shutil.which(cc)).resolve();external[str(tool)]=sha(tool);execute([cc,'--version'],cc+'-version')
 external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve())
 expected={1:'7 6 12\n',2:'13 13 4 4 13 4\n13\n',7:'alpha|alpha|\n',8:''}
 for cc in ['gcc','clang']:
  for opt in ['O0','O3']:
   label=cc+'-'+opt;start=len(records);observed={};flags=[cc,'-std=c17','-'+opt,'-Wall','-Wextra','-Werror','-pedantic-errors']
   for case in range(1,9):
    cf=[*flags,'-DCASE='+str(case)];bad=case in [3,4,5];out,err=execute([*cf,'-E','-P',public/'probes.c'],label+'-preprocess'+str(case),1 if bad else 0);observed[str(case)]=dict(preprocessed=out,preprocess_exit=1 if bad else 0)
    if bad:
     assert 'error:' in err and ('argument' in err if case in [3,4] else 'past' in err.lower());continue
    if case==6:
     out,err=execute([*cf,'-fsyntax-only',public/'probes.c'],label+'-semantic',1);assert 'conflicting types' in err;observed[str(case)]['compile_exit']=1;continue
    exe=home/(label+'-'+str(case));execute([*cf,public/'probes.c','-o',exe],label+'-compile'+str(case));out,err=execute([exe],label+'-execute'+str(case));assert out==expected[case];observed[str(case)]['stdout']=out
   profiles.append(dict(profile=label,executions=list(range(start,len(records))),observed=observed,passed=True))
 cases=[('ordinary',dict(prefix='item_',names=['status','start','stop']),True),('singleton',dict(prefix='item_',names=['solo']),True),('maximum',dict(prefix='item_',names=['n'+str(i) for i in range(32)]),True),('empty',dict(prefix='item_',names=[]),False),('too-many',dict(prefix='item_',names=['n'+str(i) for i in range(33)]),False),('duplicate',dict(prefix='item_',names=['a','a']),False),('invalid-token',dict(prefix='item_',names=['a+']),False),('empty-suffix',dict(prefix='item_',names=['']),False),('reserved-prefix',dict(prefix='__',names=['Private']),False),('reserved-suffix-policy',dict(prefix='item_',names=['_Hidden']),False),('unicode',dict(prefix='item_',names=['caf\u00e9']),False)]
 for label,value,valid in cases:
  inp=home/(label+'.json');out=home/(label+'.h');dump(inp,value);out.write_bytes(b'SENTINEL\n');execute([sys.executable,public/'generate_names.py',inp,out],'registry-'+label,0 if valid else 1)
  if not valid:assert out.read_bytes()==b'SENTINEL\n'
  else:
   expected_text=''.join('static const int item_'+n+'='+str(i)+';\n' for i,n in enumerate(value['names']));assert out.read_text()==expected_text
   src=home/(label+'.c');src.write_text('#include "'+out.name+'"\nint main(void){return '+(' || '.join('item_'+n+'!='+str(i) for i,n in enumerate(value['names'])))+';}\n',encoding='utf8')
   for cc in ['gcc','clang']:
    exe=home/(label+'-'+cc);execute([cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic-errors',src,'-o',exe],'registry-compile-'+cc);execute([exe],'registry-execute-'+cc)
  registry.append(dict(case=label,admitted=valid,preserved_on_refusal=not valid,output=out.relative_to(ROOT).as_posix(),sha256=sha(out)))
 fixture=read_json(ROOT/'fixtures/language/PR-T06-01.json');repl=fixture['input']['replacements'];token=fixture['input']['token'];seen=set()
 while token in repl:assert token not in seen;seen.add(token);token=repl[token]
 assert token==fixture['expected']['rescanned']=='7' and fixture['expected']['direct_string']==fixture['input']['spelling']=='VALUE' and repl['VALUE']==fixture['expected']['expanded_string']=='7'
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 artifacts=[dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)) for p in home.rglob('*') if p.is_file()]
 dump(ROOT/REPORT,dict(status='passed',passed=True,input_sha256=inputs,external_input_sha256=external,source_stable=stable,external_inputs_stable=extstable,executions=records,profiles=profiles,registry=registry,generated_artifacts=artifacts,synthetic_fixture_checked=True,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Invalid paste observed only as preprocessing diagnostic;no portable executable result or mandatory-diagnostic claim.','Restricted original acyclic fixture is not an implementation of general macro rescanning.']))
 print(json.dumps(dict(passed=True,commands=len(records),profiles=len(profiles),registry=len(registry))))
if __name__=='__main__':main()
