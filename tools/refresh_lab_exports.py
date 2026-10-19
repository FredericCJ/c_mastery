"""Run actual owned export producers, scoped guards and readiness builders.

This does not change C campaign observations or write global completion state.
Each producer already preserves its actual command/log contract. Failed attempts
are retained, and independent packages continue so a failure cannot disappear.
"""
import argparse
import concurrent.futures
import datetime
import json
import subprocess
import sys
from common import ROOT,sha

GROUPS={
 'b02':['arena','fixed_point','map','parse','region','result','sort','transaction'],
 'portability':['capability','context','declarations','facade','facilities','lexical','macros','migration','registry','replacement','resize','schema','wrappers'],
 'storage':['async','cache','codec','delta','durability','file','logical','lookup','readers','references','service','transactions','tree'],
}
def main():
 p=argparse.ArgumentParser();p.add_argument('--group',choices=GROUPS,required=True);p.add_argument('--only',nargs='*');a=p.parse_args()
 names=a.only or GROUPS[a.group];assert all(n in GROUPS[a.group] for n in names)
 base=ROOT/'reviews/v5/expansion/envelope_refresh_lab_01'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');base.mkdir(parents=True)
 def work(name):
  scope=a.group+'_'+name;commands=[['tools/run_'+scope+'_export.py']]
  if scope=='b02_fixed_point':commands.append(['tools/run_b02_fixed_point_legacy_export.py'])
  commands += [['tools/check_'+scope+'.py','--self-test'],['tools/build_'+scope+'_readiness.py']]
  records=[]
  for i,tail in enumerate(commands):
   cmd=[sys.executable,*tail];before=sha(ROOT/tail[0]);r=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=900);log=base/(scope+f'-{i}.log');log.write_text(r.stdout+r.stderr,encoding='utf8')
   ok=r.returncode==0 and sha(ROOT/tail[0])==before
   records.append(dict(command=cmd,exit_code=r.returncode,passed=ok,producer_sha256=before,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
   if not ok:break
  passed=len(records)==len(commands) and all(r['passed'] for r in records)
  print(scope+': '+('passed' if passed else 'FAILED'),flush=True)
  return dict(scope=scope,passed=passed,executions=records)
 with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:results=list(pool.map(work,names))
 d=dict(passed=all(r['passed'] for r in results),group=a.group,results=results,limits=['Actual export/guard/metadata refresh; preexisting C campaign bytes remain unchanged.','No global inventory writes, historical receipt replacement or independent assessment.'])
 out=base/'result.json';out.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=d['passed'],report=out.relative_to(ROOT).as_posix())))
 return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
