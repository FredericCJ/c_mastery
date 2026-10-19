"""Selected optimized original queue RC11 campaign, with unchanged failure controls."""
import argparse,datetime,hashlib,json,resource,shutil,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];GENMC=Path('/home/frede/cmastery-modern-tools/genmc/build/bin/genmc');OPT=Path('/home/frede/cmastery-modern-tools/usr/lib/llvm-21/bin/opt')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();out=ROOT/a.report;assert not out.exists();work=ROOT/'build/genmc-ir-validation'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True);rows=[]
 files=[Path(__file__),ROOT/'assessor/interrupt_depth/handoff/queue.c',ROOT/'assessor/interrupt_depth/handoff/genmc.c',ROOT/'labs/interrupt_depth/handoff/handoff.h',ROOT/'labs/interrupt_depth/frame/frame.h'];inputs={p.relative_to(ROOT).as_posix():sha(p) for p in files}
 external={p:sha(p) for p in [GENMC,Path(shutil.which('clang')).resolve(),OPT,GENMC.parents[2]/'doc/manual/usage.md',GENMC.parents[2]/'doc/manual/cli.md',GENMC.parents[2]/'passes/passes/Transforms/PromoteMemIntrinsicPass.cpp']}
 model=work/'model.c';model.write_text('#include "'+str(files[1])+'"\n#include "'+str(files[2])+'"\n')
 def limit():resource.setrlimit(resource.RLIMIT_CORE,(0,0))
 def run(name,cmd,expected=0,marker=None):
  try:r=subprocess.run(list(map(str,cmd)),cwd=ROOT,capture_output=True,text=True,timeout=90,preexec_fn=limit);body=r.stdout+r.stderr;rc=r.returncode
  except subprocess.TimeoutExpired as e:body=str(e);rc=None
  log=work/(name+'.log');log.write_text(body);passed=rc==expected and (not marker or marker in body);rows.append(dict(label=name,command=list(map(str,cmd)),returncode=rc,expected=expected,marker=marker,passed=passed,raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(name,passed,flush=True)
 run('clang-help',['clang','--help'],marker='-fno-builtin');run('genmc-help',[GENMC,'--help'],marker='-rc11');run('genmc-version',[GENMC,'--version'],marker='GenMC v0.19.0 (commit #9f6c4c0)')
 for opt in ['0','1']:
  for mutant in [0,1,7,8]:
   name='O'+opt+'-mutant'+str(mutant);before=work/(name+'-before.ll');after=work/(name+'-after.ll');flags=['-std=c17','-O'+opt,'-I'+str(ROOT/'labs/interrupt_depth/frame'),'-I'+str(ROOT/'labs/interrupt_depth/handoff'),'-DXI_MUTANT='+str(mutant)]
   if opt=='1':flags+=['-fno-builtin']
   run(name,[GENMC,'-rc11','-mode=verify','-disable-mm-detector','-disable-estimation','--output-llvm-before='+str(before),'--output-llvm-after='+str(after),'--',*flags,model],42 if mutant else 0,'Error: Non-atomic race!' if mutant else 'Number of complete executions explored: 20')
   run(name+'-verify-before',[OPT,'-passes=verify','-disable-output',before]);run(name+'-verify-after',[OPT,'-passes=verify','-disable-output',after])
 rows.append(dict(label='original sources stable',passed=all(sha(ROOT/p)==h for p,h in inputs.items())))
 value=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=inputs,external_input_sha256={str(p):h for p,h in external.items()},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},limits=['Only this finite one-producer/one-consumer queue harness: three attempted operations each and final drain, capacity two with one spare slot.','RC11 verify mode and race detection remain enabled. -fno-builtin is an explicit compiler optimization selection, not an atomic strengthening, source rewrite or intrinsic deletion.','Default -O1 remains unsupported in the preserved attempt; no full service/frame translation unit, arbitrary optimization pipeline, all-C17, target machine or liveness proof.'])
 out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(value,indent=2)+'\n');print(json.dumps({'passed':value['passed'],'checks':len(rows)}));return int(not value['passed'])
if __name__=='__main__':raise SystemExit(main())
