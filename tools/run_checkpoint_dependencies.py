"""Audit native LTO output and stack-protector/callback dependencies without executing unbootstrapped images."""
import argparse,hashlib,json,re,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def run(report):
 base=ROOT/'reviews/v5/expansion/legacy_completion';destination=(ROOT/report).resolve()
 if not destination.is_relative_to(base.resolve()):raise ValueError('completion evidence only')
 stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ');logs=base/'dependency_runs'/stamp;logs.mkdir(parents=True)
 paths=[Path(__file__).resolve(),ROOT/'labs/checkpoint_depth/transfer/dependency_probe.c',ROOT/'assessor/checkpoint_depth/checkpoint_memory.c',ROOT/'labs/checkpoint_depth/include/checkpoint_memory.h']
 pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths};rows=[];error=None
 try:
  with tempfile.TemporaryDirectory(prefix='checkpoint-dependencies-') as temporary:
   for compiler in ('gcc','clang'):
    for lto in (False,True):
     for case in ('closed','stack-protected','hidden-allocation','hidden-blocking'):
      name=f'{compiler}-lto{int(lto)}-{case}';image=Path(temporary)/(name+'.so')
      flags=['-O2','-ffreestanding','-fno-builtin','-fPIC','-shared','-nostdlib','-fno-stack-protector']
      if lto:flags+=['-flto']
      if case=='stack-protected':flags+=['-fstack-protector-all','-mstack-protector-guard=global']
      if case=='hidden-allocation':flags+=['-DCPX_SECRET_ALLOC']
      if case=='hidden-blocking':flags+=['-DCPX_SECRET_BLOCK']
      command=[compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(paths[3].parent),str(paths[1]),str(paths[2]),'-Wl,-Map,'+str(logs/(name+'.map')),'-o',str(image)]
      built=subprocess.run(command,capture_output=True,text=True,timeout=60)
      if built.returncode:raise RuntimeError(name+': '+built.stderr)
      undefined=subprocess.check_output(['nm','-u',str(image)],text=True);defined=subprocess.check_output(['nm','--defined-only',str(image)],text=True)
      assembly=subprocess.check_output(['objdump','-d',str(image)],text=True)
      (logs/(name+'-symbols.txt')).write_text('UNDEFINED\n'+undefined+'DEFINED\n'+defined,encoding='utf-8',newline='\n');(logs/(name+'-disassembly.txt')).write_text(assembly,encoding='utf-8',newline='\n')
      if not re.search(r'\bcpx_effect_probe$',defined,re.M) or not re.search(r'\bcpm_prefix$',defined,re.M):raise ValueError('LTO removed required inspected mechanism')
      rejected=bool(undefined.strip()) or bool(re.search(r'\bmalloc$',defined,re.M))
      if case=='closed' and rejected:raise ValueError('Unexpected dependency '+undefined)
      if case!='closed' and not rejected:raise ValueError('Missed dependency control '+name)
      if case=='stack-protected' and '__stack_chk_fail' not in undefined:raise ValueError('Stack protector did not materialize')
      if case=='hidden-allocation' and (undefined.strip() or not re.search(r'\bmalloc$',defined,re.M)):raise ValueError('Resolved-provider control did not materialize')
      if case=='hidden-blocking' and not re.search(r'\bpoll$',undefined,re.M):raise ValueError('Blocking callback did not materialize')
      rows.append({'name':name,'command':command,'undefined':undefined,'rejected_under_strict_contract':rejected,'passed':True})
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as exc:error=str(exc)
 stable=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items())
 result={'passed':error is None and stable and len(rows)==16,'error':error,'source_stable':stable,'inspected_paths':pins,'observations':rows,
         'scope':'Actual native shared inspection images, GCC/Clang with/without LTO. No execution of images lacking startup/TLS/guard initialization. Provider and callback effects are inspected separately from undefined-symbol closure.',
         'log_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}}
 payload=json.dumps(result,indent=2)+'\n';destination.write_text(payload,encoding='utf-8',newline='\n');(logs/'report.json').write_text(payload,encoding='utf-8',newline='\n')
 print(json.dumps({'passed':result['passed'],'checks':len(rows),'error':error}));return 0 if result['passed'] else 1

if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(run(p.parse_args().report))
