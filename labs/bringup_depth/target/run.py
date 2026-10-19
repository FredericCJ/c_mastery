"""Build learner reset against an explicitly supplied target harness; no implicit solution."""
import argparse,pathlib,subprocess,tempfile,importlib.util
HERE=pathlib.Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--reset',type=pathlib.Path,default=HERE/'reset.S');p.add_argument('--harness',type=pathlib.Path,required=True);p.add_argument('--include',type=pathlib.Path,default=HERE.parent);p.add_argument('--build',type=pathlib.Path,required=True);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
script=a.build/'image.ld';script.write_text((HERE/'image.ld.in').read_text().replace('@CODE@','0').replace('@BANK@','0x20010000'));elf=a.build/'image.elf'
cmd=['arm-none-eabi-gcc','-std=c17','-O2','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-I'+str(a.include),str(a.reset),str(a.harness),'-nostdlib','-Wl,--gc-sections','-Wl,-T,'+str(script),'-o',str(elf)]
r=subprocess.run(cmd)
if r.returncode:raise SystemExit(r.returncode)
r=subprocess.run(['arm-none-eabi-nm','-u',str(elf)],capture_output=True,text=True)
if r.returncode or r.stdout.strip():print('BU_CLOSURE_REJECT:'+r.stdout);raise SystemExit(1)
binary=a.build/'image.bin'
r=subprocess.run(['arm-none-eabi-objcopy','-O','binary',str(elf),str(binary)])
if r.returncode:raise SystemExit(r.returncode)
spec=importlib.util.spec_from_file_location('image',HERE.parent/'check_image.py');image=importlib.util.module_from_spec(spec);spec.loader.exec_module(image);result=image.inspect(elf,0,0x20010000);loaders=[]
for section in ['.bss','.bank_bss','.data','.bank_data']:
 s=result['sections'][section];seed=a.build/(section[1:]+'.seed');seed.write_bytes(bytes([165])*s['size']);loaders+=['-device',f'loader,file={seed},addr={s["address"]},force-raw=on']
raise SystemExit(subprocess.run(['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',str(binary),*loaders],timeout=20).returncode)
