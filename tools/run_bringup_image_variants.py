"""Actual vendor/original image, GC, placement and implicit-helper investigations."""
from pathlib import Path
import argparse,datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);p.add_argument('--sources',type=Path,required=True);a=p.parse_args();lab=ROOT/'labs/bringup_depth';ref=ROOT/'assessor/bringup_depth';build=ROOT/'build/bringup-images'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f');build.mkdir(parents=True);rows=[]
 expected={'vendor-f429.s':'4b235aec8ca2fece51748221ff3d2b5ca3a113ac59738f1fda1ffa44f1da3953','vendor.ld':'e21958e98c2dbfa63c5112859966a1f7fc16394d3547da42f3c8c81e37c5668e'}
 def pins_now():return {f.relative_to(ROOT).as_posix():hashlib.sha256(f.read_bytes()).hexdigest() for f in [Path(__file__),*lab.rglob('*'),*ref.rglob('*')] if f.is_file() and '__pycache__' not in str(f)}
 initial=pins_now()
 for n,h in expected.items():assert hashlib.sha256((a.sources/n).read_bytes()).hexdigest()==h,n
 def run(label,cmd,rc=0,needle=None):
  cmd=list(map(str,cmd));log=build/(str(len(rows))+'.txt')
  try:r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=30);out=r.stdout+r.stderr;code=r.returncode
  except (OSError,subprocess.TimeoutExpired) as e:out=str(e);code=None
  log.write_text(out,encoding='utf8');ok=code==rc and (needle is None or needle in out);rows.append(dict(label=label,command=cmd,returncode=code,expected_returncode=rc,required_text=needle,passed=ok,raw=log.relative_to(ROOT).as_posix(),raw_sha256=hashlib.sha256(log.read_bytes()).hexdigest()));print(label,ok,flush=True);return out,ok
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['-O0','-O3']:
   base=build/(cc+opt);base.mkdir();flags=['-std=c17',opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-I'+str(lab),'-isystem','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib']+(['--target=arm-none-eabi'] if cc=='clang' else [])
   obj=base/'core.o';_,ok=run(str(base.name)+' core compile',[cc,*flags,'-c',ref/'bringup.c','-o',obj])
   if ok:
    out,_=run(str(base.name)+' whole core closure',['arm-none-eabi-nm','-u',obj]);rows[-1]['passed']&=not out.strip()
   life=base/'lifecycle.o';_,ok=run(base.name+' lifecycle core compile',[cc,*flags,'-c',ref/'lifecycle.c','-o',life])
   if ok:
    out,_=run(base.name+' lifecycle core closure',['arm-none-eabi-nm','-u',life]);rows[-1]['passed']&=not out.strip()
   helper=base/'helpers.o';_,ok=run(str(base.name)+' helper specimen compile',[cc,*flags,'-c',ref/'target/helpers.c','-o',helper])
   if ok:
    run(str(base.name)+' actual implicit imports',['arm-none-eabi-nm','-u',helper],needle='__aeabi_uldivmod')
    run(str(base.name)+' forbidden helper link',['arm-none-eabi-ld','-e','wider',helper,'-o',base/'forbidden.elf'],1,'undefined reference')
   for mode in ['original','gc-missing','stack-overlap','stack-align','dma-bank']:
    script=base/(mode+'.ld');text=(lab/'target/image.ld.in').read_text().replace('@CODE@','0x00000000').replace('@BANK@','0x20010000')
    if mode=='gc-missing':text=text.replace('KEEP(*(.vectors))','*(.vectors)')
    elif mode=='stack-overlap':text=text.replace('_stack_top-2048','ORIGIN(RAM)+32')
    elif mode=='stack-align':text=text.replace('ORIGIN(RAM)+LENGTH(RAM);','ORIGIN(RAM)+LENGTH(RAM)-4;')
    elif mode=='dma-bank':text=text.replace('@BANK@','0x10000000').replace('ORIGIN = 0x20010000','ORIGIN = 0x10000000').replace('KEEP(*(.dma)) . = ALIGN(4); } >RAM','KEEP(*(.dma)) . = ALIGN(4); } >BANK')
    script.write_text(text);objects=[]
    for name in ['reset.S','guest.c']:
     o=base/(mode+name+'.o');_,ok=run(base.name+' '+mode+' compile '+name,[cc,*flags,'-c',ref/'target'/name,'-o',o]);objects.append(o)
    # Unretained vector refers to reset, but nothing live refers back after this
    # target-only specimen replaces that reference in reset/guest objects below.
    if mode=='gc-missing':
     asm=base/'unreferenced.S';asm.write_text('.syntax unified\n.thumb\n.section .vectors,"a"\n.word 0x20010000,bu_reset\n.section .text.reset,"ax"\n.global bu_reset\n.thumb_func\nbu_reset: b bu_reset\n');o=base/'unreferenced.o';run(base.name+' missing-vector specimen compile',[cc,*flags,'-c',asm,'-o',o]);objects=[o]
    elf=base/(mode+'.elf');reason={'gc-missing':'vector table missing','stack-overlap':'stack overlaps','stack-align':'stack alignment','dma-bank':'DMA inaccessible'}
    _,ok=run(base.name+' '+mode+' link',['arm-none-eabi-ld','--gc-sections','-T',script,*objects,'-Map='+str(base/(mode+'.map')),'-o',elf],0 if mode=='original' else 1,reason.get(mode))
    if mode=='original' and ok:
     run(base.name+' independent positive image',[sys.executable,lab/'check_image.py',elf,'--code','0','--bank','0x20010000'])
     run(base.name+' eight actual ELF mutations',[sys.executable,ref/'image_negatives.py',elf,base/'negative-images'])
   for mode in ['physical-cross','mps2-missing-copy','mps2-repaired-copy']:
    script=base/(mode+'.ld');text=(a.sources/'vendor.ld').read_text();mps=mode!='physical-cross'
    # Original source pair is untouched. Derived script changes are captured.
    text=text.replace('SECTIONS\n{','SECTIONS\n{\n .retained (NOLOAD) : { KEEP(*(.retained)) } >RAM')
    text=text.replace('    libc.a ( * )','').replace('    libm.a ( * )','').replace('    libgcc.a ( * )','')
    if mps:text=text.replace('ORIGIN = 0x08000000','ORIGIN = 0x00000000').replace('ORIGIN = 0x10000000','ORIGIN = 0x20040000')
    script.write_text(text);objects=[]
    for name,source in [('vendor',a.sources/'vendor-f429.s'),('adapter',ref/'target/vendor_adapter.c')]:
     o=base/(mode+name+'.o');_,ok=run(base.name+' '+mode+' compile '+name,[cc,*flags,'-DBU_VENDOR_MPS2='+str(int(mps)),'-DBU_VENDOR_SECOND='+str(int(mode=='mps2-repaired-copy')),'-c',source,'-o',o]);objects.append(o)
    elf=base/(mode+'.elf');_,ok=run(base.name+' '+mode+' link',['arm-none-eabi-ld','--gc-sections','-T',script,*objects,'-Map='+str(base/(mode+'.map')),'-o',elf])
    if not ok:continue
    out,_=run(base.name+' '+mode+' closure',['arm-none-eabi-nm','-u',elf]);rows[-1]['passed']&=not out.strip()
    run(base.name+' '+mode+' disassembly',['arm-none-eabi-objdump','-d',elf]);run(base.name+' '+mode+' sections',['arm-none-eabi-objdump','-h',elf])
    if mps:run(base.name+' '+mode+' execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf],1 if mode=='mps2-missing-copy' else 0,'BU_VENDOR_SECOND_UNCOPIED' if mode=='mps2-missing-copy' else 'BU_VENDOR_PASS')
 pins=pins_now();rows.append(dict(label='source stable during execution',passed=initial==pins))
 report=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,outputs_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in build.rglob('*') if p.is_file()},external_sources={str(a.sources/n):h for n,h in expected.items()},limits=['Unmodified vendor reset code with an explicitly original restricted SystemInit/runtime adapter; no ST clock implementation executed.','F429 original linker memory contract, F439 sibling comparison; MPS2 derived addresses do not become physical evidence.','Negative linker tests are not semantic mutants. GCC/Clang emitted helper calls inspected; no helper library linked.']);path=ROOT/a.report;path.write_text(json.dumps(report,indent=2)+'\n');print(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not report['passed'])
if __name__=='__main__':raise SystemExit(main())
