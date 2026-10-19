"""Actual cleanup-extension/removal equivalence and live-only pointer control."""
from datetime import datetime,timezone
from pathlib import Path
import json,shlex,shutil,subprocess,sys
from common import ROOT,dump,sha,read_json
BASE='research/v5/expansion/legacy_portability_followthrough';REPORT=BASE+'/replacement_companion_validation.json'
def main():
 public=ROOT/'labs/language_portability/replacement';private=ROOT/'assessor/language_portability/replacement'
 paths=['tools/run_portability_replacement_companion.py','tools/verify_portability_replacement.py','tools/common.py','fixtures/language/PR-T05-05.json']+[p.relative_to(ROOT).as_posix() for d in [public,private] for p in d.iterdir() if p.is_file()]
 inputs={p:sha(ROOT/p) for p in paths};external={};records=[];profiles=[];artifacts=[];dependencies=[];observations=[]
 runs=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('replacement-companion-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
 def execute(cmd,label,expect=0,markers=()):
  cmd=list(map(str,cmd));dep=None
  if cmd[0] in ['gcc','clang'] and '-c' in cmd and expect==0:dep=runs/(f'{len(records):03d}-'+label+'.d');cmd+=['-MD','-MF',str(dep)]
  p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=60);out=p.stdout+p.stderr;log=runs/(f'{len(records):03d}-'+label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_exit=expect,markers=list(markers),log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert (p.returncode==expect if expect is not None else p.returncode!=0) and all(m in out for m in markers),out
  if expect is None:assert 'error:' in out
  if dep:
   deps=[Path(x).resolve() for x in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1])];dependencies.append(dict(execution=len(records)-1,paths=[str(q) for q in deps]))
   for q in deps:
    if q.is_relative_to(ROOT):inputs.setdefault(q.relative_to(ROOT).as_posix(),sha(q))
    else:external.setdefault(str(q),sha(q))
  return out
 for cc in ['gcc','clang','nm','objdump','size','ld']:
  tool=Path(shutil.which(cc)).resolve();external[str(tool)]=sha(tool);execute([cc,'--version'],cc+'-version')
 external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve())
 for cc in ['gcc','clang']:
  for mode in ['debug','release','asan']:
   label=cc+'-'+mode;home=runs/label;home.mkdir();start=len(records);flags=['-O3' if mode=='release' else '-O0','-Wall','-Wextra','-Werror','-pedantic-errors','-I'+str(public)];san=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie'] if mode=='asan' else [];link=['-fsanitize=address,undefined','-no-pie'] if mode=='asan' else []
   for variant in ['baseline','extension']:
    source=private/('replacement.c' if variant=='baseline' else 'extension.c');standard='c17' if variant=='baseline' else 'c23';objects=[]
    for i,src in enumerate([source,public/'tests.c']):
     obj=home/(variant+str(i)+'.o');execute([cc,*flags,*san,'-std='+standard,'-c',src,'-o',obj],variant+'-object');objects.append(obj)
    exe=home/variant;execute([cc,*link,*objects,'-o',exe],variant+'-link')
    for anchor in ['failure','order','transfer']:execute([exe,anchor],variant+'-'+anchor,markers=['LP '+anchor+' validated'])
    out=execute([sys.executable,ROOT/'tools/verify_portability_replacement.py','--driver',exe],variant+'-oracle');observations.append(dict(profile=label,variant=variant,result=json.loads(out)))
    text=execute([cc,*flags,'-std='+standard,'-E','-P',source],variant+'-preprocessed')
    if variant=='baseline':assert 'gnu::cleanup' not in text and '__attribute__' not in source.read_text() and '[[' not in source.read_text()
    execute(['nm','-u',objects[0]],variant+'-symbols');execute(['objdump','-dr',objects[0]],variant+'-assembly');execute(['size',exe],variant+'-size');out=execute(['ldd',exe],variant+'-runtime')
    for token in out.split():
     q=Path(token)
     if q.is_absolute() and q.is_file():external[str(q.resolve())]=sha(q.resolve())
   execute([cc,*flags,'-std=c17','-fsyntax-only',private/'extension.c'],'strict-bracket-rejection',None)
   obj=home/'live.o';exe=home/'live';execute([cc,*flags,*san,'-std=c17','-c',public/'provenance_live.c','-o',obj],'live-object');execute([cc,*link,obj,'-o',exe],'live-link');execute([exe],'live-run',markers=['LP_PROVENANCE live void-pointer integer roundtrip only'])
   profiles.append(dict(profile=label,variants=['baseline','extension'],strict_rejection=True,extension_free_removal=True,live_only=True,executions=list(range(start,len(records)))))
 fixture=read_json(ROOT/'fixtures/language/PR-T05-05.json');print(fixture)
 actual=[bool(x['fallback'] and x['boundary']) for x in fixture['input']['extensions']];assert actual==fixture['expected']['ready']
 for p in runs.rglob('*'):
  if p.is_file() and p.suffix!='.log':artifacts.append(dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)))
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 dump(ROOT/REPORT,dict(schema_version=1,status='passed',passed=True,producer='tools/run_portability_replacement_companion.py',input_sha256=inputs,source_stable=stable,external_input_sha256=external,external_inputs_stable=extstable,generated_artifacts=artifacts,executions=records,profiles=profiles,compiler_dependencies=dependencies,observations=observations,synthetic_fixture_ready=actual,expired_case_executed=False,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Only normal return and injected acquisition failure are extension-equivalent;nonlocal exits are outside contract.','N3231 is a dated working draft,not published TS6010;N3245 download was not parseable.','Expired source is reasoning-only;live uintptr_t control is not a provenance-model proof.']))
 print(json.dumps(dict(passed=True,commands=len(records),profiles=len(profiles))))
if __name__=='__main__':main()
