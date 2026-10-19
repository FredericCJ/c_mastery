"""Original staged/macros/unfused/caller-frame/push alternatives and bounded measurements."""
from pathlib import Path
import datetime,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
 build=ROOT/'build/t71-t72-alternatives';build.mkdir(parents=True,exist_ok=True);rows=[];files=[Path(__file__)]
 def run(name,cmd,negative=None):
  try:
   p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=120);out=p.stdout+p.stderr;r=dict(label=name,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=p.returncode==0 if negative is None else p.returncode!=0 and negative in out)
  except (OSError,subprocess.TimeoutExpired) as e:r=dict(label=name,command=list(map(str,cmd)),passed=False,error=str(e))
  rows.append(r);print(name,r['passed'],flush=True);return r['passed']
 profiles=[('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O0'],'clang-debug'),('clang',['-O3'],'clang-release'),('gcc',['-O1','-g','-fsanitize=address,undefined'],'gcc-sanitize'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]
 for family,cases,mutants,macro,marker in [('protocols',['alternatives'],{1:'alternatives',2:'alternatives'},'PA_MUTANT','PA_CHECK:'),('coroutines',['iterator','generator','push'],{1:'iterator',2:'iterator',3:'generator',4:'generator',5:'push',6:'push'},'CX_MUTANT','CX_CHECK:')]:
  lab=ROOT/'labs'/family/'followthrough';ref=ROOT/'assessor'/family/'followthrough';core=ROOT/'assessor'/family/(family+'.c');imports=[core]+([ref/'query_follow.c'] if family=='coroutines' else [])
  files += [lab/'alternatives.h',lab/'alternatives.c',lab/'alternatives_tests.c',ref/'alternatives.c',*imports,ROOT/'labs'/family/'include'/(family+'.h')]
  for cc,flags,tag in profiles:
   base=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-Wno-implicit-fallthrough','-fno-pie','-no-pie',*flags,'-I'+str(lab),'-I'+str(ROOT/'labs'/family/'include')]
   for m in [0,*mutants,'stub']:
    exe=build/(tag+'-'+family+'-'+str(m));cmd=[*base,(lab if m=='stub' else ref)/'alternatives.c',*imports,lab/'alternatives_tests.c','-o',exe]
    if m!='stub':cmd.append('-D'+macro+'='+str(m))
    if run(tag+' '+family+' build '+str(m),cmd):
     for case in cases if m in [0,'stub'] else [mutants[m]]:run(tag+' '+family+' '+case+' '+str(m),[exe,*([case] if family=='coroutines' else [])],None if m==0 else marker)
     if family=='coroutines' and m==0 and 'release' in tag:run(tag+' raw equal-contract iterator measurements',[exe,'cost'])
   if family=='protocols':run(tag+' wrong staged wrapper diagnostic',*[([x for x in base if x!='-no-pie']+['-c',lab/'wrong_builder_stage.c','-o',build/(tag+'-bad.o')])],negative='incompatible')
  if family=='coroutines':
   files.append(lab/'query_follow.h')
   for cc in ['gcc','clang']:run(cc+' optimized alternatives disassembly',['objdump','-d',build/(cc+'-release-coroutines-0')])
 files.append(ROOT/'labs/protocols/followthrough/wrong_builder_stage.c')
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),checks=rows,input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},limits=['Exact operations and emitted values precede timing; raw local monotonic timings do not establish a portable speedup','Macro fall-through warning is suppressed only for the intentional PT_WAIT case-label construction; other diagnostics remain errors','No independent learner or external reviewer claim'])
 (ROOT/'research/v5/modern/t71_t72_alternatives_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
