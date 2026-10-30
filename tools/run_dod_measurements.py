"""Bounded, equal-overflow-contract host layout measurements; no speed threshold."""
import argparse
from datetime import datetime,timezone
import hashlib,json,platform,statistics,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--report',required=True);args=parser.parse_args()
 if platform.system()!='Linux':raise SystemExit('Run in the declared Linux host environment')
 report=(ROOT/args.report).resolve()
 if not report.is_relative_to(ROOT/'reviews/v5/expansion'):raise ValueError('Use new expansion evidence path')
 paths=[ROOT/p for p in ['labs/dod/tests/measure.c','labs/dod/include/dod.h','labs/dod/include/dod_ext.h','assessor/dod/dod.c','assessor/dod/dod_ext.c','tools/run_dod_measurements.py']]
 pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
 directory=report.parent/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');directory.mkdir(parents=True,exist_ok=False)
 build=ROOT/'build/modern_completion/dod-measure';build.mkdir(parents=True,exist_ok=True)
 records=[];measurements=[]
 def execute(command,label):
  r=subprocess.run([str(x) for x in command],cwd=ROOT,capture_output=True,text=True,timeout=180)
  log=directory/(label+'.log');log.write_text(r.stdout+'\n'+r.stderr,encoding='utf-8')
  records.append(dict(command=[str(x) for x in command],exit_code=r.returncode,log=log.relative_to(ROOT).as_posix(),log_sha256=hashlib.sha256(log.read_bytes()).hexdigest()))
  if r.returncode:raise RuntimeError(label+' failed')
  return r.stdout
 for compiler in ['gcc','clang']:
  execute([compiler,'--version'],compiler+'-version')
  flags=['-std=c17','-O3','-Wall','-Wextra','-Wpedantic','-Werror','-fno-lto','-I','labs/dod/include']
  if compiler=='gcc':flags+=['-fopt-info-vec-all='+str(build/'gcc-vectorization.txt')]
  else:flags+=['-Rpass=loop-vectorize','-Rpass-missed=loop-vectorize']
  binary=build/(compiler+'-layout')
  execute([compiler,*flags,'labs/dod/tests/measure.c','assessor/dod/dod.c','assessor/dod/dod_ext.c','-o',binary],compiler+'-build')
  execute(['size',binary],compiler+'-size')
  execute(['objdump','-d',binary],compiler+'-disassembly')
  if compiler=='gcc':
   vector=(build/'gcc-vectorization.txt').read_text();log=directory/'gcc-vectorization.log';log.write_text(vector,encoding='utf-8');records.append(dict(command=['read compiler-produced vectorization report'],exit_code=0,log=log.relative_to(ROOT).as_posix(),log_sha256=hashlib.sha256(log.read_bytes()).hexdigest()))
  rows=[json.loads(line) for line in execute([binary],compiler+'-measure').splitlines()]
  if len(rows)!=336:raise ValueError('Incomplete measurement grid')
  for n in [1024,65536,1048576]:
   expected_hot=sum(i%1000+4 for i in range(n));expected_whole=sum(i%1000+2+i%251 for i in range(n))
   for row in rows:
    if row['n']==n and row['checksum']!=(expected_hot if row['kind']<2 else expected_whole):raise ValueError('Unequal correctness contract')
   for eviction in [0,1]:
    for kind in range(8):
     values=[r['nanoseconds'] for r in rows if (r['n'],r['eviction_conditioned'],r['kind'])==(n,eviction,kind)]
     measurements.append(dict(compiler=compiler,n=n,eviction_conditioned=eviction,kind=['row_update','column_update','row_whole','column_whole','row_indexed','column_indexed','row_to_columns','columns_to_rows'][kind],samples_ns=values,median_ns=statistics.median(values),min_ns=min(values),max_ns=max(values)))
 stable=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items())
 result=dict(schema_version=1,passed=stable,scope='T70.01 host layout, conversion and generated-instruction measurement support',recorded_at=datetime.now(timezone.utc).isoformat(),inspected_paths=pins,source_stable=stable,executions=records,measurements=measurements,environment=platform.uname()._asdict(),limits=['Seven samples per cell, no CPU affinity or frequency control; competing host load affects uncertainty.','64MiB stride touches are eviction-conditioned, not proof of a cold cache.','No performance pass threshold, universal speedup, hardware counter, second ABI or physical-board claim.','Whole-record consumers and updates are distinct workloads; compare layouts only within the same kind pair.','Compiler reports and disassembly are supplied artifacts requiring reasoned interpretation, not automatic proof of vectorization or constant-time behavior.'])
 report.parent.mkdir(parents=True,exist_ok=True);report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');print(json.dumps(dict(passed=stable,measurements=len(measurements),samples=672)))
 return 0 if stable else 1
if __name__=='__main__':raise SystemExit(main())
