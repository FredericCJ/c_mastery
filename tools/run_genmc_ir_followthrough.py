"""Preserve failed optimized IR attempts and test explicit supported profiles."""
import argparse,datetime,hashlib,json,os,resource,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];GENMC=Path('/home/frede/cmastery-modern-tools/genmc/build/bin/genmc')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();out=ROOT/a.report;assert not out.exists();work=ROOT/'build/genmc-ir-followthrough'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True);rows=[]
 sources=[Path(__file__),ROOT/'assessor/interrupt_depth/handoff/queue.c',ROOT/'assessor/interrupt_depth/handoff/genmc.c',ROOT/'labs/interrupt_depth/handoff/handoff.h',ROOT/'labs/interrupt_depth/frame/frame.h'];before={p.relative_to(ROOT).as_posix():sha(p) for p in sources}
 model=work/'model.c';model.write_text('#include "'+str(sources[1])+'"\n#include "'+str(sources[2])+'"\n')
 def limits():resource.setrlimit(resource.RLIMIT_CORE,(0,0))
 def run(name,cmd):
  try:r=subprocess.run(list(map(str,cmd)),cwd=ROOT,text=True,capture_output=True,timeout=90,preexec_fn=limits);body=r.stdout+r.stderr;rc=r.returncode
  except subprocess.TimeoutExpired as e:body=str(e);rc=None
  log=work/(name+'.log');log.write_text(body);rows.append(dict(label=name,command=list(map(str,cmd)),returncode=rc,raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(name,rc,flush=True)
 run('help',[GENMC,'--help']);run('version',[GENMC,'--version'])
 flags=['-std=c17','-I'+str(ROOT/'labs/interrupt_depth/frame'),'-I'+str(ROOT/'labs/interrupt_depth/handoff')]
 profiles={'o1':['-O1'],'o1-no-builtin':['-O1','-fno-builtin'],'o1-scalar':['-O1','-fno-builtin','-fno-vectorize','-fno-slp-vectorize'],'o1-no-idiom':['-O1','-fno-builtin','-fno-vectorize','-fno-slp-vectorize','-mllvm','-disable-loop-idiom-all']}
 for name,opts in profiles.items():
  run(name,[GENMC,'-rc11','-mode=verify','-disable-mm-detector','-disable-estimation','--output-llvm-before='+str(work/(name+'-before.ll')),'--output-llvm-after='+str(work/(name+'-after.ll')),'--',*flags,*opts,model])
 value=dict(attempts=rows,input_sha256=before,external_input_sha256={str(GENMC):sha(GENMC)},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},source_stable=all(sha(ROOT/p)==h for p,h in before.items()),scope='Actual original queue and original finite harness, RC11 exhaustive mode with race detection enabled. Optimization choices explicit; failures receive no proof credit.')
 out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(value,indent=2)+'\n')
if __name__=='__main__':main()
