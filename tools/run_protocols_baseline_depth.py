"""Execute retained anchors, independent oracle and newly explicit boundary obligations."""
from pathlib import Path
import datetime, hashlib, json, subprocess, sys
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/protocols';src=ROOT/'assessor/protocols/protocols.c';build=ROOT/'build/protocols-baseline-depth';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(name,cmd,negative=None):
  try:
   p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=90);out=p.stdout+p.stderr;ok=p.returncode==0 if negative is None else p.returncode!=0 and negative in out;r=dict(label=name,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=ok)
  except (OSError,subprocess.TimeoutExpired) as e:r=dict(label=name,command=list(map(str,cmd)),passed=False,error=str(e))
  rows.append(r);print(name,r['passed'],flush=True);return r['passed']
 cases=['typestate','builder','session','hsm','active','thread','behavior','driver']
 profiles=[('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O0'],'clang-debug'),('clang',['-O3'],'clang-release'),('gcc',['-O1','-g','-fsanitize=address,undefined'],'gcc-sanitize'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]
 for cc,flags,tag in profiles:
  base=[cc,'-std=c17','-Wall','-Wextra','-Werror','-I'+str(lab/'include'),*flags]
  exe=build/(tag+'-boundaries')
  if run(tag+' supplemental boundaries build',[*base,src,lab/'followthrough/baseline_probes.c','-o',exe]):run(tag+' supplemental boundaries execute',[exe])
  for m in [None,*[(case,variant) for case in cases for variant in ['', '2']]]:
   name='reference' if m is None else '-'.join(m);exe=build/(tag+'-'+name)
   cmd=[*base,src,lab/'tests/tests.c','-o',exe]
   if m:cmd.append('-DTS_MUTATE_'+m[0].upper()+m[1])
   if run(tag+' anchor build '+name,cmd):
    for case in cases if m is None else [m[0]]:run(tag+' '+case+' '+name,[exe,case],None if m is None else 'TS_CHECK:')
 # Oracle uses a separately derived Python table and actual C shared library.
 for cc in ['gcc','clang']:
  so=build/(cc+'.so')
  if run(cc+' oracle library',[cc,'-std=c17','-O3','-shared','-fPIC','-I'+str(lab/'include'),src,'-o',so]):run(cc+' independent finite table',[sys.executable,ROOT/'tools/verify_protocols_fixtures.py','--library',so])
  run(cc+' duplicate suspension label diagnostic',[cc,'-std=c17','-Wall','-Wextra','-Werror','-I'+str(lab/'include'),'-c',lab/'tests/negative/duplicate_label.c','-o',build/(cc+'-bad.o')],negative='duplicate case')
 files=[Path(__file__),ROOT/'tools/verify_protocols_fixtures.py',src,lab/'include/protocols.h',lab/'CONTRACTS.md',lab/'tests/tests.c',lab/'tests/negative/duplicate_label.c',lab/'followthrough/baseline_probes.c',*[ROOT/f'fixtures/protocols/PR-T71-{i:02}.json' for i in range(1,9)]]
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),checks=rows,input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},limits=['Retained original implementations; no historical receipt overwritten','Independent oracle is separately derived source, not an external human reviewer','Finite enumerations and mutation sensitivity do not establish absolute mastery; owning/changed-condition mechanisms have separate follow-through reports'])
 (ROOT/'research/v5/modern/protocols_baseline_depth_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
