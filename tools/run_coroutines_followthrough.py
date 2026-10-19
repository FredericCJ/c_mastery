"""Source-bound required coroutine variants; keep host, ABI and learner evidence distinct."""
from pathlib import Path
import argparse,datetime,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--parts',nargs='*');args=ap.parse_args();lab=ROOT/'labs/coroutines/followthrough';ref=ROOT/'assessor/coroutines/followthrough';build=ROOT/'build/coroutines-followthrough';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(label,cmd,negative=None,timeout=90):
  try:
   p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=timeout);out=p.stdout+p.stderr;r=dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=p.returncode==0 if negative is None else p.returncode!=0 and negative in out)
  except (OSError,subprocess.TimeoutExpired) as e:r=dict(label=label,command=list(map(str,cmd)),passed=False,error=str(e))
  rows.append(r);print(label,r['passed'],flush=True);return r['passed']
 parts=[('owned_stream','owned_stream_tests.c',['mapper','generator'],{1:'mapper',2:'mapper',3:'generator',4:'generator'},'CF_MUTANT','CF_CHECK:'),('query_follow','query_tests.c',['query'],{1:'query',2:'query'},'CQ_MUTANT','CQ_CHECK:'),('fiber_follow','fiber_tests.c',['fiber'],{1:'fiber',2:'fiber'},'FF_MUTANT','FF_CHECK:'),('expression_follow','expression_tests.c',['expression'],{1:'expression',2:'expression'},'CE_MUTANT','CE_CHECK:'),('parser_follow','parser_tests.c',['cut','memo'],{1:'cut',4:'cut',2:'memo',3:'memo'},'CP_MUTANT','CP_CHECK:'),('async_follow','async_tests.c',['async'],{1:'async',2:'async'},'CA_MUTANT','CA_CHECK:'),('effect_follow','effect_tests.c',['effect'],{1:'effect',2:'effect'},'CH_MUTANT','CH_CHECK:'),('channel_follow','channel_tests.c',['channel'],{1:'channel',2:'channel'},'CC_MUTANT','CC_CHECK:')]
 profiles=[('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O0'],'clang-debug'),('clang',['-O3'],'clang-release'),('gcc',['-O1','-g','-fsanitize=address,undefined','-DCO_ASAN'],'gcc-sanitize'),('clang',['-O1','-g','-fsanitize=address,undefined','-DCO_ASAN'],'clang-sanitize')]
 for cc,flags,tag in profiles:
  for stem,test,cases,mutants,macro,marker in parts:
   if args.parts and stem not in args.parts:continue
   imports=[ROOT/'assessor/coroutines/fiber.c',ROOT/'assessor/coroutines/fiber_x86_64.S'] if stem=='fiber_follow' else [ROOT/'assessor/coroutines/coroutines.c'] if stem in ['query_follow','effect_follow'] else [ref/'async_follow.c',ROOT/'assessor/messaging/messaging.c'] if stem=='channel_follow' else []
   for m in [0,*mutants,'stub']:
    exe=build/(tag+'-'+stem+'-'+str(m));src=(lab if m=='stub' else ref)/(stem+'.c')
    cmd=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-Wno-misleading-indentation','-pthread','-fno-pie','-no-pie','-fcf-protection=none',*flags,'-I'+str(lab),'-I'+str(ROOT/'labs/coroutines/include'),'-I'+str(ROOT/'labs/messaging/include'),src,*imports,lab/test,'-o',exe]
    if stem=='fiber_follow':cmd.extend(['-Wl,--wrap=munmap','-Wl,--wrap=mprotect'])
    if m!='stub':cmd.append('-D'+macro+'='+str(m))
    if run(tag+' '+stem+' build '+str(m),cmd):
     for case in cases if m in [0,'stub'] else [mutants[m]]:run(tag+' '+case+' execute '+str(m),[exe,*([case] if stem in ['owned_stream','parser_follow'] else [])],None if m==0 else marker)
     if stem=='fiber_follow' and m==0 and 'sanitize' not in tag:run(tag+' guard page child',[exe,'guard'])
 if not args.parts or 'fiber_follow' in args.parts:
  for cc in ['gcc','clang']:
   exe=build/(cc+'-nested-tsan');cmd=[cc,'-std=c17','-O1','-g','-fsanitize=thread','-DCO_TSAN','-fno-pie','-no-pie','-fcf-protection=none','-I'+str(lab),'-I'+str(ROOT/'labs/coroutines/include'),ref/'fiber_follow.c',ROOT/'assessor/coroutines/fiber.c',ROOT/'assessor/coroutines/fiber_x86_64.S',lab/'fiber_tests.c','-o',exe]
   cmd.extend(['-Wl,--wrap=munmap','-Wl,--wrap=mprotect'])
   if run(cc+' nested fiber TSan build',cmd):run(cc+' nested fiber TSan execute',[exe])
  run('fiber ELF notes',['readelf','-n',build/'gcc-release-fiber_follow-0']);run('fiber context disassembly',['objdump','-d',build/'gcc-release-fiber_follow-0'])
 for cc in ['gcc','clang']:
  for stem in ['async','channel']:
   if args.parts and stem+'_follow' not in args.parts:continue
   exe=build/(cc+'-'+stem+'-tsan');imports=[ref/'async_follow.c']+([ref/'channel_follow.c',ROOT/'assessor/messaging/messaging.c'] if stem=='channel' else [])
   cmd=[cc,'-std=c17','-O1','-g','-pthread','-fsanitize=thread','-fno-pie','-no-pie','-I'+str(lab),'-I'+str(ROOT/'labs/coroutines/include'),'-I'+str(ROOT/'labs/messaging/include'),*imports,lab/(stem+'_tests.c'),'-o',exe]
   if run(cc+' '+stem+' TSan build',cmd):run(cc+' '+stem+' TSan execute',[exe])
 files=[Path(__file__),*lab.glob('*'),*ref.glob('*'),ROOT/'labs/coroutines/include/coroutines.h',ROOT/'labs/coroutines/include/fiber.h',ROOT/'assessor/coroutines/coroutines.c',ROOT/'assessor/coroutines/fiber.c',ROOT/'assessor/coroutines/fiber_x86_64.S',ROOT/'labs/messaging/include/messaging.h',ROOT/'assessor/messaging/messaging.c']
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),selected_parts=args.parts,checks=rows,input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files if p.is_file()},limitations=['Original author implementation variants only; independent learner, external review and physical evidence not inferred','Guard-page failure is an isolated child process property, not overflow recovery','Fiber backend remains Linux x86-64 SysV same-thread without CET, signal migration or foreign unwinding support','ThreadSanitizer covers executed schedules/annotations only'])
 (ROOT/'research/v5/modern/coroutines_followthrough_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
