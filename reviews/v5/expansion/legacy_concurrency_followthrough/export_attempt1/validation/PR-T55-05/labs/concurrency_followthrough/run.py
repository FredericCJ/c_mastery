"""Build an incomplete learner component, or explicitly selected reference tree."""
import argparse,json,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];PUBLIC=ROOT/'labs/concurrency_followthrough'
INCLUDES=['labs/concurrency_followthrough/core','labs/concurrency_followthrough/history','labs/concurrency_followthrough/measure','labs/advanced_concurrency_depth/reclamation','labs/advanced_concurrency_depth/grace','labs/execution/include','labs/api_depth/include']

def recipe(part,source,providers):
 p={k:ROOT/v for k,v in providers.items()}
 if part=='core':return [PUBLIC/'core/tests.c',source/'core/core.c']
 if part=='history':return [PUBLIC/'history/tests.c',source/'history/history.c']
 if part=='epoch':return [PUBLIC/'core/epoch_tests.c',source/'core/epoch.c',p['queue'],p['grace'],p['advanced'],p['concurrent'],p['registry']]
 if part=='protocol':return [PUBLIC/'core/protocol_tests.c',source/'history/history.c',p['queue'],p['advanced'],p['concurrent']]
 if part=='measure':return [PUBLIC/'measure/main.c',source/'measure/measure.c',source/'core/core.c',p['queue'],p['grace'],p['concurrent'],p['registry']]
 if part=='protection':return [ROOT/'labs/advanced_concurrency_depth/reclamation/tests.c',p['queue']]
 if part=='reservation':return [ROOT/'labs/advanced_concurrency_depth/reclamation/reservation_tests.c',p['reservation'],p['advanced']]
 if part=='versions':return [ROOT/'labs/advanced_concurrency_depth/grace/tests.c',p['grace'],p['advanced'],p['concurrent'],p['registry']]
 raise ValueError(part)

def compile_command(cc,flags,part,source,providers,exe):
 return [cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-pthread',*flags,*['-I'+str(ROOT/p) for p in INCLUDES],*map(str,recipe(part,source,providers)),'-o',str(exe)]

def main():
 p=argparse.ArgumentParser();p.add_argument('--part',required=True);p.add_argument('--source',type=Path,default=PUBLIC);p.add_argument('--providers',type=Path,default=PUBLIC/'providers.json');p.add_argument('--build',type=Path,required=True);p.add_argument('--cc',default='gcc');p.add_argument('--case',default='tests');p.add_argument('--compile-only',action='store_true');p.add_argument('--flag',action='append',default=[]);p.add_argument('args',nargs='*');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=False);exe=a.build.resolve()/'program';cmd=compile_command(a.cc,a.flag or ['-O2'],a.part,a.source.resolve(),json.loads(a.providers.read_text()),exe)
 r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True);(a.build/'compile.txt').write_text(r.stdout+r.stderr);print(r.stdout+r.stderr,end='')
 if r.returncode:return r.returncode
 if a.compile_only:return 0
 r=subprocess.run([str(exe),*(a.args or [a.case])],cwd=ROOT,capture_output=True,text=True,timeout=90);(a.build/'run.txt').write_text(r.stdout+r.stderr);print(r.stdout+r.stderr,end='');return r.returncode
if __name__=='__main__':raise SystemExit(main())
