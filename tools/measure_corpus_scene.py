"""Raw actual cache work and typed-region storage under the supplied workloads."""
import datetime
import json
import subprocess
from common import ROOT,sha
def main():
 paths=['labs/corpus_scene/tests.c','labs/corpus_scene/scene.h','assessor/corpus_scene/scene.c','tools/measure_corpus_scene.py']
 pins={p:sha(ROOT/p) for p in paths};build=ROOT/'build/scene-measure'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S');build.mkdir(parents=True)
 records=[];observations=[]
 def run(args):
  args=[str(a) for a in args];r=subprocess.run(args,cwd=ROOT,capture_output=True,text=True,timeout=90)
  records.append(dict(command=args,exit_code=r.returncode,stdout=r.stdout+r.stderr,passed=r.returncode==0));assert r.returncode==0,r.stderr
  return r.stdout
 for cc in ['gcc','clang']:
  exe=build/cc;run([cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I',ROOT/'labs/corpus_scene',ROOT/'labs/corpus_scene/tests.c',ROOT/'assessor/corpus_scene/scene.c','-o',exe])
  rows=run([exe,'1']).splitlines()+run([exe,'2']).splitlines()
  for line in rows:
   r=line.split()
   if r[:2]==['COST','scene']:observations.append(dict(compiler=cc,kind='actual cache workload',matrix_multiplications=int(r[2]),zero_work_repeated_queries=int(r[3]),scene_bytes=int(r[4]),query_matrix_staging_bytes=int(r[5]),limits='Work counters, not machine latency; keep-world also uses a full staged scene.'))
   if r[:2]==['COST','nav']:observations.append(dict(compiler=cc,kind='actual hosted ABI storage',scratch_bytes=int(r[2]),session_bytes=int(r[3]),copied_path_bytes=int(r[4]),limits='Persistent polygon inputs are additional; target per-function frame audit is separate.'))
 assert all(sha(ROOT/p)==h for p,h in pins.items())
 out=ROOT/'reviews/v5/expansion/corpus_scene/cost_validation.json';out.write_text(json.dumps(dict(passed=True,source_stable=True,input_sha256=pins,executions=records,observations=observations,limits=['No physical timing, cache traffic, whole-path stack, rendering or navigation-device observation.']),indent=2)+'\n',encoding='utf8')
 print(json.dumps(dict(passed=True,commands=len(records),observations=len(observations))))
if __name__=='__main__':main()
