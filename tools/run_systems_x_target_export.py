"""Fresh isolated public target export; external assessor paths are explicit."""
import argparse,datetime,hashlib,json,shutil,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();dest=ROOT/a.report;assert not dest.exists()
 work=ROOT/'build/systems-x-target-export'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');export=work/'public';export.mkdir(parents=True);sources={Path(__file__)};checks=[]
 for part in ['systems_x_followthrough','modern_bridge/include','execution/include']:
  original=ROOT/'labs'/part;target=export/'labs'/part
  shutil.copytree(original,target,ignore=shutil.ignore_patterns('__pycache__','*.pyc'))
  sources.update(p for p in original.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 def run(label,cmd,marker):
  r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=240,cwd=export);raw=work/(str(len(checks))+'.log');raw.write_text(r.stdout+r.stderr)
  checks.append(dict(label=label,command=list(map(str,cmd)),returncode=r.returncode,expected=0,marker=marker,passed=r.returncode==0 and marker in raw.read_text(),raw=raw.relative_to(ROOT).as_posix(),raw_sha256=sha(raw)));print(label,checks[-1]['passed'],flush=True)
 before={p.relative_to(ROOT).as_posix():sha(p) for p in sources};target_reports=[];external={}
 for cc in ['arm-none-eabi-gcc','clang']:
  build=work/(cc+'-m4');run(cc+' isolated target export',[sys.executable,export/'labs/systems_x_followthrough/reclamation/target.py','--source',ROOT/'assessor/systems_x_followthrough/reclamation/list.c','--bridge',ROOT/'assessor/modern_bridge/bridge.c','--runtime',ROOT/'assessor/execution','--build',build,'--cc',cc,'--opt','2'],'LIST_TARGET_PASS modes=2 rounds=20 readers=2 actual_context_switches=1 no_archives=1')
  if (build/'target.json').is_file():
   target_reports.append((build/'target.json').relative_to(ROOT).as_posix());report=json.loads((build/'target.json').read_text())
   for path,h in report['source_dependencies'].items():
    pp=Path(path)
    if pp.is_relative_to(ROOT):
     # Generated export bytes are covered by outputs; actual provider bytes remain inputs.
     if not pp.is_relative_to(work):before[pp.relative_to(ROOT).as_posix()]=h
    else:external[path]=h
 checks.append(dict(label='isolated public tree contains no assessor',passed=not any('assessor' in p.relative_to(export).parts for p in export.rglob('*'))))
 checks.append(dict(label='source stable',passed=all(sha(ROOT/p)==h for p,h in before.items())))
 value=dict(passed=all(r['passed'] for r in checks),checks=checks,input_sha256=before,external_input_sha256=external,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},target_reports=target_reports,limits=['External private implementations are explicit test inputs; only public contracts, headers and workload were copied. Actual Cortex-M emulator execution, not a physical board observation.'])
 dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(value,indent=2)+'\n');print(json.dumps({'passed':value['passed'],'checks':len(checks)}));return int(not value['passed'])
if __name__=='__main__':raise SystemExit(main())
