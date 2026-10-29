"""Actual B02 facade import,type-stage controls and strict delayed sequencing."""
from datetime import datetime,timezone
from pathlib import Path
import json,re,shlex,shutil,subprocess,sys
from common import ROOT,dump,sha
BASE='research/v5/expansion/legacy_portability_followthrough';REPORT=BASE+'/facade_companion_validation.json'
def main():
 public=ROOT/'labs/language_portability/facade';private=ROOT/'assessor/language_portability/facade';inputs={p.relative_to(ROOT).as_posix():sha(p) for folder in [public,private] for p in folder.rglob('*') if p.is_file()}
 for p in ['tools/run_portability_facade_companion.py','tools/common.py','labs/b02_depth/include/depth.h','labs/b02_depth/include/facade.h','assessor/b02_depth/depth.c','assessor/b02_depth/facade.h','labs/b02_depth/CONTRACTS.md']:inputs[p]=sha(ROOT/p)
 home=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('facade-companion-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));home.mkdir(parents=True);records=[];external={};dependencies=[];profiles=[];strict=[];runtime=[]
 def execute(cmd,label,expected=0):
  cmd=list(map(str,cmd));dep=None
  if cmd[0] in ['gcc','clang'] and '-c' in cmd and expected==0:dep=home/(str(len(records))+'-'+label+'.d');cmd+=['-MD','-MF',str(dep)]
  p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=90);out=p.stdout+p.stderr;log=home/(str(len(records))+'-'+label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_exit=expected,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert p.returncode==expected,out
  if dep:
   paths=[Path(q).resolve() for q in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1])];dependencies.append(dict(execution=len(records)-1,paths=[str(q) for q in paths]))
   for q in paths:
    if not q.is_relative_to(ROOT):external[str(q)]=sha(q)
  return p.stdout,p.stderr
 for tool in ['gcc','clang','nm','readelf','size','objdump','ldd']:
  path=Path(shutil.which(tool)).resolve();external[str(path)]=sha(path);execute([tool,'--version'],tool+'-version')
 includes=['-I'+str(public),'-I'+str(ROOT/'labs/b02_depth/include'),'-DBF_B02_FACADE="'+str(ROOT/'assessor/b02_depth/facade.h')+'"']
 for cc in ['gcc','clang']:
  for opt in ['O0','O2']:
   label=cc+'-'+opt;flags=[cc,'-std=c17','-'+opt,'-Wall','-Wextra','-Werror','-pedantic-errors',*includes];start=len(records)
   for case in range(1,7):
    _,err=execute([*flags,'-DCASE='+str(case),'-fsyntax-only',public/'probes.c'],label+'-type-negative'+str(case),1);assert 'error:' in err
   _,err=execute([*flags,'-DCASE=7','-E','-P',public/'probes.c'],label+'-invalid-token',1);assert 'pasting' in err
   output=home/(label+'-wrong-type.i');execute([*flags,'-DCASE=8','-E',public/'probes.c','-o',output],label+'-nested-preprocess');_,err=execute([cc,'-std=c17','-pedantic-errors','-fsyntax-only',output],label+'-nested-semantic',1);assert 'error:' in err and ('struct bf_record' in err or 'incompatible' in err)
   expanded=home/(label+'-positive.i');execute([*flags,'-E','-P',public/'probes.c','-o',expanded],label+'-positive-preprocess');text=expanded.read_text();assert '"A"' in text and '"73"' in text and 'b2_identity_int(73)' in text
   objects=[]
   for i,src in enumerate([public/'probes.c',private/'facade.c',ROOT/'assessor/b02_depth/depth.c']):
    obj=home/(label+'-'+str(i)+'.o');execute([*flags,'-c',src,'-o',obj],label+'-object');objects.append(obj)
   exe=home/label;execute([cc,*objects,'-o',exe],label+'-link');out,_=execute([exe],label+'-run');assert out in ['direct=LR counts=1,1\nsequenced=LR counts=1,1 raw=A expanded=73\n','direct=RL counts=1,1\nsequenced=LR counts=1,1 raw=A expanded=73\n'];runtime.append(exe)
   execute(['objdump','-dr',objects[0]],label+'-assembly');execute(['size',exe],label+'-size')
   mutant=home/(label+'-identity-mutant');execute([*flags,'-DB2_MUTATE_MACRO',public/'probes.c',private/'facade.c',ROOT/'assessor/b02_depth/depth.c','-o',mutant],label+'-mutant-build');execute([mutant],label+'-mutant-reject',1);runtime.append(mutant)
   profiles.append(dict(profile=label,negative_type_cases=[1,2,3,4,5,6],invalid_token_observation_only=True,nested_preprocess_passed=True,nested_semantic_rejected=True,static_type_assertions=6,direct_order=out.splitlines()[0],sequenced_order='LR',identity_mutant_rejected=True,executions=list(range(start,len(records)))))
   flags=[cc,'-std=c17','-'+opt,'-ffreestanding','-ffunction-sections','-fdata-sections','-fno-builtin','-fno-stack-protector','-fno-pie','-Wall','-Wextra','-Werror','-pedantic-errors',*includes];objects=[]
   for i,src in enumerate([private/'facade.c',ROOT/'assessor/b02_depth/depth.c',public/'target.c']):
    obj=home/(label+'-strict'+str(i)+'.o');execute([*flags,'-c',src,'-o',obj],label+'-strict-object');objects.append(obj)
   raw_undefined,_=execute(['nm','-u',objects[1]],label+'-provider-object-undefined');exe=home/(label+'-strict');execute([cc,'-nostdlib','-static','-no-pie',*objects,public/'entry.S','-Wl,--gc-sections','-Wl,-Map,'+str(home/(label+'-link.map')),'-Wl,-e,_start','-o',exe],label+'-strict-link');u,_=execute(['nm','-u',exe],label+'-undefined');d,_=execute(['readelf','-d',exe],label+'-dynamic');h,_=execute(['readelf','-h',exe],label+'-elf');assert not u.strip() and 'NEEDED' not in d and 'ELF64' in h and 'X86-64' in h
   symbols,_=execute(['nm','--defined-only',exe],label+'-selected-symbols');selected=sorted(re.findall(r'\b(b2_\w+)$',symbols,re.M));assert selected==['b2_identity_int','b2_identity_unsigned']
   execute([exe],label+'-strict-run');execute(['size',exe],label+'-strict-size');strict.append(dict(profile=label,executed=True,undefined=[],needed=[],component_library_calls=False,provider='assessor/b02_depth/depth.c',provider_object_undefined=raw_undefined.split(),reachable_provider_functions=selected))
 for i,exe in enumerate(runtime):
  out,_=execute(['ldd',exe],'runtime'+str(i))
  for value in re.findall(r'(/[^\s()]+)',out):
   path=Path(value).resolve()
   if path.is_file():external[str(path)]=sha(path)
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 artifacts=[dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)) for p in home.rglob('*') if p.is_file()]
 dump(ROOT/REPORT,dict(status='passed',passed=True,input_sha256=inputs,external_input_sha256=external,source_stable=stable,external_inputs_stable=extstable,executions=records,compiler_dependencies=dependencies,generated_artifacts=artifacts,profiles=profiles,strict_targets=strict,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Invalid concatenation is an observed preprocessing diagnostic,not a defined executable behavior.','Call-argument function bodies are not interleaved;their relative order is not fixed by the facade.','Borrow validity depends on live caller objects;no lifetime-invalid pointer is executed.']))
 print(json.dumps(dict(passed=True,commands=len(records),profiles=len(profiles),dependencies=len(dependencies))))
if __name__=='__main__':main()
