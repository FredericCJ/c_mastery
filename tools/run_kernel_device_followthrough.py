"""Device ownership, capability, geometry and explicit VM workspace campaign."""
import argparse,hashlib,importlib.util,json,os,subprocess
from pathlib import Path
from c_source import replace_unique
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main(work):
 work=work.resolve();assert work.is_relative_to(ROOT/'reviews/v5/expansion/legacy_kernel_followthrough');work.mkdir(parents=True,exist_ok=False);commands=[];checks=[]
 lab=ROOT/'labs/kernel_followthrough/device';ref=ROOT/'assessor/kernel_followthrough/device/device.c';runtime=ROOT/'labs/kernel_followthrough/runtime';port=ROOT/'assessor/kernel_followthrough/runtime'
 paths=[*lab.glob('*'),ref,*runtime.glob('*'),*port.glob('*'),ROOT/'assessor/runtime/cvm.c',ROOT/'labs/runtime/include/cvm.h',Path(__file__)]
 before={p.relative_to(ROOT).as_posix():sha(p) for p in paths if p.is_file()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:exitcode=86',UBSAN_OPTIONS='halt_on_error=1:exitcode=86')
 def run(label,cmd,expected=0):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,env=env,timeout=60);p=work/(label+'.txt');p.write_text(r.stdout+r.stderr);commands.append({'label':label,'command':cmd,'exit_code':r.returncode,'expected_exit_code':expected,'output':p.relative_to(ROOT).as_posix()});assert r.returncode==expected,(label,r.stdout,r.stderr);return r.stdout+r.stderr
 def check(name,condition):checks.append({'name':name,'passed':bool(condition)});assert condition,name
 try:
  spec=importlib.util.spec_from_file_location('kf_port_registration',port/'port_registration.py');registration=importlib.util.module_from_spec(spec);spec.loader.exec_module(registration);check('exact-source-registered-port',registration.render((ROOT/'assessor/runtime/cvm.c').read_bytes())==(port/'cvm_scratch.c').read_text())
  for cc in ('gcc','clang'):
   run(cc+'-version',[cc,'--version'])
   for profile,flags in [('O0',['-O0','-g']),('O2',['-O2','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-no-pie'])]:
    label=cc+'-'+profile;exe=work/label;run(label+'-build',[cc,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(lab),lab/'tests.c',ref,'-o',exe])
    for group in ('pci','queue','geometry','ordering','durability'):check(label+'-'+group,'KF_DEVICE_PASS:'+group in run(label+'-'+group,[exe,group]))
    outputs=[]
    for scratch in (False,True):
     name=label+('-scratch' if scratch else '-baseline');exe=work/name;core=port/'cvm_scratch.c' if scratch else ROOT/'assessor/runtime/cvm.c'
     includes=['-I'+str(runtime),'-I'+str(ROOT/'labs/runtime/include')]
     run(name+'-build',[cc,'-std=c17','-Wall','-Wextra','-Werror',*flags,*includes,*(['-DKF_SCRATCH'] if scratch else []),runtime/'hosted.c',runtime/'probe.c',core,'-o',exe]);outputs.append(run(name,[exe]));check(name+'-six-distinct-outcomes',outputs[-1].count('KF_VM:')==6 and ('KF_VM_TAIL:1:6:7:3216:1076:'+('5120' if scratch else '0')) in outputs[-1])
    check(label+'-baseline-scratch-semantic-rows-identical',outputs[0].splitlines()[:6]==outputs[1].splitlines()[:6]);exe=work/(label+'-reentry');run(label+'-reentry-build',[cc,'-std=c17','-Wall','-Wextra','-Werror',*flags,*includes,runtime/'reentry.c',port/'cvm_scratch.c','-o',exe]);check(label+'-reentry-atomic','SCRATCH_REENTRY_REFUSED_UNCHANGED_RELEASED' in run(label+'-reentry',[exe]))
   for group in ('pci','queue','geometry','ordering','durability'):check(cc+'-memcheck-'+group,'ERROR SUMMARY: 0 errors' in run(cc+'-memcheck-'+group,['valgrind','--leak-check=full','--error-exitcode=89',work/(cc+'-O0'),group]))
   original=ref.read_text();mutations=[('duplicate-cap','pci','(kinds & (1u << kind)) ||','0 ||'),('aperture','pci','size > extent - off ||','0 ||'),('timeout-recycle','queue','r->state = KF_UNKNOWN;','r->state = KF_SUCCESS;'),('write-stable-too-early','queue','++q->completed_writes;','q->stable_writes = ++q->completed_writes;'),('slice-widen','geometry','logical >= count ||','0 ||'),('changed-sector','geometry','bytes != 512 ||','bytes == 0 ||')]
   for name,group,old,new in mutations:
    source=work/(cc+'-'+name+'.c');source.write_text(replace_unique(original,old,new));exe=work/(cc+'-'+name);run(cc+'-'+name+'-build',[cc,'-std=c17','-O2','-I'+str(lab),lab/'tests.c',source,'-o',exe]);check(cc+'-'+name+'-killed','KF_DEVICE_CHECK:' in run(cc+'-'+name,[exe,group],1))
  exe=work/'stub';run('stub-build',['gcc','-std=c17','-I'+str(lab),lab/'tests.c',lab/'device.c','-o',exe]);check('stub-incomplete','KF_DEVICE_CHECK:' in run('stub',[exe,'pci'],1));check('source-stable',all(sha(ROOT/p)==h for p,h in before.items()))
 except Exception as e:passed=False;error=repr(e)
 else:passed=True;error=None
 result={'passed':passed,'error':error,'commands':commands,'checks':checks,'inspected_paths':before,'artifact_sha256':{p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},'scope':'Hosted synthetic PCI/queue/DMA premise models and native VM workspace checks; real guest/MMIO/flush evidence remains separate.'};(work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':passed,'error':error,'commands':len(commands),'checks':len(checks)}));return int(not passed)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);raise SystemExit(main(p.parse_args().work))
