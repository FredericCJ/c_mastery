"""Record actual higher-order compilation, binary, stack and representation evidence."""
from pathlib import Path
import argparse,hashlib,json,subprocess,time,sys,shutil
from datetime import datetime,timezone
ROOT=Path(__file__).resolve().parents[1]
SOURCES=['assessor/higher_order/free.c','assessor/higher_order/final.c','labs/higher_order/include/higher_order.h','labs/higher_order/CONTRACTS.md','tools/run_higher_order_costs.py']
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);args=parser.parse_args()
 report=(ROOT/args.report).resolve();assert report.is_relative_to(ROOT/'reviews/v5/expansion')
 stamp='higher-costs-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
 run=report.parent/'runs'/stamp;run.mkdir(parents=True)
 work=ROOT/'build'/'higher_order_costs'/stamp;work.mkdir(parents=True)
 pins={p:digest(ROOT/p) for p in SOURCES};records=[];results=[]
 probe=run/'layout.c';probe.write_text('#include "higher_order.h"\n#include <stdio.h>\nint main(void){printf("%zu %zu %zu %zu %zu %zu %zu %zu\\n",sizeof(HONode),sizeof(HOA),sizeof(HEInt),sizeof(HEFun),sizeof(HPInt),sizeof(HPFun),sizeof(HEOps),sizeof(HPOps));return 0;}\n')
 def execute(command,label):
  start=time.monotonic_ns();p=subprocess.run(command,cwd=ROOT,capture_output=True,text=True);elapsed=time.monotonic_ns()-start
  log=run/(label+'.log');log.write_text(p.stdout+'\n'+p.stderr,encoding='utf8');records.append({'command':list(map(str,command)),'exit_code':p.returncode,'elapsed_ns':elapsed,'log':log.relative_to(ROOT).as_posix(),'log_sha256':digest(log)});assert p.returncode==0,(label,p.stderr);return p.stdout,elapsed
 for compiler in ('gcc','clang'):
  execute([compiler,'--version'],compiler+'-version');samples=[]
  for sample in range(3):
   objs=[];duration=0
   for source in SOURCES[:2]:
    obj=work/f'{compiler}-{sample}-{Path(source).stem}.o';objs.append(obj)
    _,elapsed=execute([compiler,'-std=c17','-O2','-g','-Wall','-Wextra','-Wpedantic','-Werror','-fstack-usage','-I'+str(ROOT/'labs/higher_order/include'),'-c',str(ROOT/source),'-o',str(obj)],f'{compiler}-{sample}-{Path(source).stem}-compile');duration+=elapsed
   exe=work/f'{compiler}-{sample}-layout';_,elapsed=execute([compiler,'-std=c17','-O2','-I'+str(ROOT/'labs/higher_order/include'),str(probe),*map(str,objs),'-o',str(exe)],f'{compiler}-{sample}-link');duration+=elapsed
   output,_=execute([str(exe)],f'{compiler}-{sample}-layout');sizes=list(map(int,output.split()));assert len(sizes)==8 and all(n>0 for n in sizes)
   samples.append(duration)
   if sample==0:
    execute(['size',*map(str,objs),str(exe)],compiler+'-size')
    for obj in objs:execute(['objdump','-dr',str(obj)],compiler+'-'+obj.stem+'-assembly')
  copied=work/(compiler+'-incremental-source');copied.mkdir()
  for source in SOURCES[:3]:shutil.copyfile(ROOT/source,copied/Path(source).name)
  shutil.copyfile(probe,copied/'layout.c')
  (copied/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(higher_cost C)\nset(CMAKE_C_STANDARD 17)\nadd_compile_options(-O2 -Wall -Wextra -Wpedantic -Werror)\nadd_executable(layout layout.c free.c final.c)\ntarget_include_directories(layout PRIVATE .)\n')
  built=work/(compiler+'-incremental-build')
  execute(['cmake','-S',str(copied),'-B',str(built),'-G','Ninja','-DCMAKE_C_COMPILER='+compiler],compiler+'-incremental-configure')
  execute(['cmake','--build',str(built)],compiler+'-incremental-first')
  text,no_change=execute(['cmake','--build',str(built)],compiler+'-incremental-no-change');assert 'no work to do' in text
  with (copied/'final.c').open('a') as f:f.write('\n/* Local copied-source invalidation probe; mechanism unchanged. */\n')
  text,changed=execute(['cmake','--build',str(built)],compiler+'-incremental-one-source');assert 'final.c.o' in text and 'free.c.o' not in text
  results.append({'compiler':compiler,'clean_translation_and_link_ns':samples,'incremental_no_change_ns':no_change,'incremental_copied_source_comment_change_ns':changed,'layout_bytes':dict(zip(['HONode','HOA','HEInt','HEFun','HPInt','HPFun','HEOps','HPOps'],sizes))})
 for usage in work.glob('*.su'):(run/usage.name).write_bytes(usage.read_bytes())
 artifacts={p.relative_to(ROOT).as_posix():digest(p) for p in run.iterdir() if p.suffix in ('.su','.log','.c')}
 result={'schema_version':1,'status':'passed','passed':True,'inspected_paths':pins,'scope_source_stable':pins=={p:digest(ROOT/p) for p in SOURCES},'results':results,'executions':records,'artifacts':artifacts,'limits':['Actual Linux x86-64 observations; no second ABI or physical evidence.','Three independent translation/link commands per compiler; filesystem/toolchain caches not cleared; no timing superiority inference.','Stack usage files report compiler frames, not automatic whole-call-graph proofs. Bind callback stack and arbitrary callback effects remain caller obligations.','Incremental observations use isolated copies and one appended comment: they verify dependency invalidation, not semantic-change compilation cost or broad speedup.','This cost report is author observation, not independent learner or external review.']}
 assert result['scope_source_stable'];report.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(results))
if __name__=='__main__':main()
