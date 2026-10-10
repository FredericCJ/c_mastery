"""Build/run the SAME portable core on emulated MPS2 AN386 Cortex-M4.
Requires explicit --assessor-root, or exercises learner src/cvm.c by default.
This is emulated instruction/reset execution, not STM32/physical evidence.
"""
import argparse,json,pathlib,shutil,subprocess,tempfile
p=argparse.ArgumentParser();p.add_argument('--assessor-root',type=pathlib.Path);p.add_argument('--report',type=pathlib.Path);a=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[1];impl=a.assessor_root or root/'src'
required=['arm-none-eabi-gcc','qemu-system-arm']
missing=[x for x in required if not shutil.which(x)]
if missing:raise SystemExit('UNAVAILABLE: '+', '.join(missing))
with tempfile.TemporaryDirectory(prefix='cmastery-runtime-thumb-') as tmp:
 elf=pathlib.Path(tmp)/'cvm.elf'
 command=['arm-none-eabi-gcc','-std=c17','-mcpu=cortex-m4','-mthumb','-Os','-ffreestanding','-fno-builtin','-nostdlib','-I'+str(root/'include'),str(impl/'cvm.c'),str(root/'thumb/startup.c'),'-T'+str(root/'thumb/link.ld'),'-Wl,-Map='+str(pathlib.Path(tmp)/'cvm.map'),'-lgcc','-o',str(elf)]
 build=subprocess.run(command,text=True,capture_output=True);print(build.stdout+build.stderr,end='')
 if build.returncode:raise SystemExit(build.returncode)
 run=subprocess.run(['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',str(elf)],capture_output=True,text=True,timeout=15)
 output=run.stdout+run.stderr;print(output,end='')
 result={'status':'pass' if run.returncode==0 and 'CVM_THUMB_PASS' in output else 'fail','target':'QEMU mps2-an386 Cortex-M4','scope':'emulated portable VM load/verify/run and rooted-pair collection; no STM32 peripherals or physical timing','command':command,'output':output,'exit_code':run.returncode}
 if a.report:a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(result,indent=2)+'\n')
 raise SystemExit(0 if result['status']=='pass' else 1)
