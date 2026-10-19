"""Build an explicit completed startup adapter in a private owned CMK image."""
import argparse,hashlib,importlib.util,json,os,shutil,subprocess
from pathlib import Path
PUBLIC=Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def cpu_features(model,props=None):
 command=['qemu-system-x86_64','-machine','pc-i440fx-10.2','-accel','tcg','-display','none','-nodefaults','-nic','none','-S','-qmp','stdio']
 messages=[{'execute':'qmp_capabilities','id':'caps'},{'execute':'query-cpu-model-expansion','arguments':{'type':'full','model':{'name':model,'props':props or {}}},'id':'model'},{'execute':'quit','id':'quit'}]
 result=subprocess.run(command,input='\n'.join(json.dumps(m) for m in messages)+'\n',capture_output=True,text=True,timeout=15)
 if result.returncode:raise ValueError('CPU preflight command failed: '+result.stderr)
 replies=[json.loads(line) for line in result.stdout.splitlines() if line.startswith('{')];reply=next((r for r in replies if r.get('id')=='model'),{})
 if 'return' not in reply:raise ValueError('CPU model cannot be expanded: '+json.dumps(reply))
 expanded=reply['return']['model'];features=expanded.get('props',{});missing=[name for name in ('apic','pae','lm','nx') if features.get(name) is not True]
 return {'model':model,'properties':props or {},'available':not missing,'missing':missing,'expanded':expanded,'command':command,'exit_code':result.returncode,'stdout':result.stdout,'stderr':result.stderr}
def prepare(lab,source,implementation,work,base=0x8000,mode=0,mutant=0,compiler='gcc',optimization='-O2',observer=None):
 if base not in (0x8000,0x9000) or mode not in (0,1,2) or mutant not in (0,1,2):raise ValueError('Unsupported startup experiment')
 work.mkdir(parents=True,exist_ok=False);copied=work/'source';shutil.copytree(source,copied)
 old=copied/'smp.c';text=old.read_text(encoding='utf-8');anchor='void cmk2_smp_run(unsigned info){'
 if text.count(anchor)!=1:raise ValueError('Retained completed entry changed; inspect before adapting')
 text=text.replace(anchor,'void cmk2_retained_smp_run(unsigned info){')+'\n'+(implementation/'startup_guest.c').read_text(encoding='utf-8');old.write_text(text,encoding='utf-8',newline='\n')
 assembly=copied/'ap.S';assembly.write_text(assembly.read_text(encoding='utf-8')+'\n'+(implementation/'startup_trampoline.S').read_text(encoding='utf-8'),encoding='utf-8',newline='\n')
 shutil.copyfile(implementation/'startup.c',copied/'startup.c')
 spec=importlib.util.spec_from_file_location('ss_cmk_guest',lab/'tools/guest.py');guest=importlib.util.module_from_spec(spec);spec.loader.exec_module(guest)
 guest.FLAGS=[p for p in guest.FLAGS if p!='-O2']+[optimization,'-I'+str(PUBLIC),'-DSS_BASE='+hex(base),'-DSS_MODE='+str(mode),'-DSS_GUEST_MUTANT='+str(mutant)]
 original=guest.run
 def run(command,**kwargs):
  command=list(command)
  # Retained CMK contains GCC-only combinations of _Atomic and GNU atomics.
  # The changed-compiler experiment compiles the entire adapted smp.c and ap.S
  # translation units with the selected compiler; the frozen boot/runtime/user
  # prerequisite remains GCC-built. Preserve that dependency contract explicitly.
  if command[0]=='gcc' and any(str(p).endswith(('/smp.c','/ap.S')) for p in command):command[0]=compiler
  # Retained Multiboot assembly uses GNU as integer truncation. Clang's C
  # frontend remains selected, with the declared external GNU assembler for
  # these GNU assembly sources; do not silently rewrite retained boot bytes.
  if command[0]=='clang' and any(str(p).endswith('.S') for p in command):command.insert(1,'-fno-integrated-as')
  return observer(command,**kwargs) if observer is not None else original(command,**kwargs)
 guest.run=run
 for key in tuple(os.environ):
  if key.startswith(('CMASTERY_STORAGE_','CMASTERY_RUNTIME_')):os.environ.pop(key)
 guest.build(lab,copied,work/'guest')
 return guest,{'base':base,'mode':mode,'mutant':mutant,'compiler':compiler,'compiler_scope':'Selected compiler for adapted smp.c and ap.S; GCC for frozen boot/kernel/user prerequisite, GNU as for assembly.','optimization':optimization,'source_root':str(source),'implementation_root':str(implementation),'modified_copy_sha256':{p.name:sha(p) for p in (old,assembly,copied/'startup.c')},'scope':'Retained discovery/APIC/PIT/per-CPU table routines imported unchanged; only old test entry renamed and explicit new original adapter/trampoline appended.'}
def command(guest,work,cpus,model):
 if cpus not in (1,2,4) or model not in ('qemu64','max'):raise ValueError('Use declared CPU/model matrix')
 os.environ['CMASTERY_CPUS']=str(cpus);cmd=guest.command(work/'guest','write');cmd[cmd.index('-cpu')+1]=model
 if cpus==4:cmd[cmd.index('-smp')+1]='4,sockets=2,cores=3,threads=1,maxcpus=6'
 disk=work/'guest/disk.raw'
 with disk.open('wb') as stream:stream.truncate(4*1024*1024)
 return cmd
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--lab',type=Path,required=True);p.add_argument('--source',type=Path,required=True);p.add_argument('--implementation',type=Path,required=True);p.add_argument('--work',type=Path,required=True);p.add_argument('--base',type=lambda s:int(s,0),default=0x8000);p.add_argument('--mode',type=int,choices=(0,1,2),default=0);p.add_argument('--cpus',type=int,choices=(1,2,4),default=4);p.add_argument('--cpu-model',choices=('qemu64','max'),default='qemu64');a=p.parse_args()
 features=cpu_features(a.cpu_model)
 if not features['available']:raise SystemExit('Required features unavailable: '+','.join(features['missing']))
 g,info=prepare(a.lab.resolve(),a.source.resolve(),a.implementation.resolve(),a.work.resolve(),a.base,a.mode)
 r=subprocess.run(command(g,a.work.resolve(),a.cpus,a.cpu_model),capture_output=True,text=True,timeout=45);(a.work/'execution.log').write_text(r.stdout+r.stderr,encoding='utf-8',newline='\n');print(r.stdout+r.stderr);raise SystemExit(0 if r.returncode==33 and 'SS_GUEST_PASS:' in r.stdout and 'SS_GUEST_CHECK:' not in r.stdout else 1)
