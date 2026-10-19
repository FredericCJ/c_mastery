"""Actual original CMX tasks on Cortex-M4; explicit sources, no runtime archive."""
import argparse,hashlib,json,re,shlex,shutil,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent;LABS=HERE.parents[1]
p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--bridge',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--build',type=Path,required=True);p.add_argument('--cc',default='arm-none-eabi-gcc',choices=['arm-none-eabi-gcc','clang']);p.add_argument('--opt',default='2',choices=['0','2']);p.add_argument('--mutant',default=0,type=int);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
rows=[];deps={Path(__file__).resolve(),HERE/'target.ld'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(name,cmd):
 cmd=list(map(str,cmd));r=subprocess.run(cmd,capture_output=True,text=True,timeout=90);body=r.stdout+r.stderr;log=a.build/(name+'.log');log.write_text(body);rows.append(dict(name=name,command=cmd,returncode=r.returncode,log=str(log),sha256=sha(log)));assert r.returncode==0,(name,body);return body
sources=[a.runtime/n for n in ['startup.S','context.S','cortex_port.c','board.c','support.c','cmx.c','concurrent.c','advanced.c']]+[a.source,a.bridge,HERE/'target_workload.c'];objects=[]
for i,s in enumerate(sources):
 obj=a.build/(str(i)+'.o');objects.append(obj);d=a.build/(str(i)+'.d');flags=['-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-O'+a.opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-fstack-usage','-g','-DCMX_QEMU','-DCMX_STAGE=2','-DCMX_STAGE_NAME="systems-x"','-DSX_LIST_MUTANT='+str(a.mutant),'-I'+str(HERE),'-I'+str(LABS/'modern_bridge/include'),'-I'+str(LABS/'execution/include'),'-idirafter','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib']
 if a.cc=='clang':flags+=['--target=arm-none-eabi']
 if s.suffix=='.S':flags+=['-mfpu=fpv4-sp-d16']
 else:flags+=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic']
 run(str(i)+'-compile',[a.cc,*flags,'-MD','-MF',d,'-c',s,'-o',obj]);run(str(i)+'-imports',['arm-none-eabi-nm','-u',obj]);deps.update(Path(x) for x in shlex.split(d.read_text().replace('\\\n',' ').split(':',1)[1]))
elf=a.build/'list.elf';run('link',['arm-none-eabi-ld','--gc-sections','-T',HERE/'target.ld',*objects,'-Map='+str(a.build/'list.map'),'-o',elf]);assert not run('undefined',['arm-none-eabi-nm','-u',elf]).strip();symbols=run('symbols',['arm-none-eabi-nm','-n',elf]);dis=run('disassembly',['arm-none-eabi-objdump','-drwC',elf]);run('size',['arm-none-eabi-size',elf]);assert not re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|libclang_rt', (a.build/'list.map').read_text())
assert all(x in symbols for x in ['sx_collect','sx_protect','mb_epoch_collect','PendSV_Handler','SysTick_Handler','SVC_Handler']);assert 'ldrex' in dis and 'strex' in dis and 'dmb' in dis
for tool in [a.cc,'arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']:deps.add(Path(shutil.which(tool)).resolve())
output=run('qemu',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',elf]);assert 'PASS CMX Cortex-M systems-x' in output
report={'passed':True,'rows':rows,'source_dependencies':{str(p.resolve()):sha(p) for p in sorted(deps)},'image_sha256':sha(elf),'scope':'Actual M4 CMX SVC/PendSV/SysTick, two reader contexts and writer,20 equal-pool rounds; dedicated emulator only, not board or multi-core.'};(a.build/'target.json').write_text(json.dumps(report,indent=2)+'\n');print('LIST_TARGET_PASS modes=2 rounds=20 readers=2 actual_context_switches=1 no_archives=1')
