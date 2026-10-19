"""Build only explicit original release teaching sources; no hardware flashing."""
import hashlib,re,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent
def build(work,compiler,opt,source,driver,helpers,variant,run):
 work.mkdir(parents=True,exist_ok=True);cc=['arm-none-eabi-gcc'] if compiler=='gcc' else ['clang','--target=arm-none-eabi']
 flags=['-std=c17',opt,'-g','-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fstack-usage','-DDT_TARGET_MUTANT='+str(variant if isinstance(variant,int) else 0),'-I'+str(HERE)]
 if compiler=='gcc':flags+=['-fno-tree-loop-distribute-patterns']
 sources=[HERE/'m4.c',source,driver,helpers,HERE/'entry.S'];objects=[]
 for i,p in enumerate(sources):
  obj=work/(str(i)+'.o');run([*cc,*(['-mcpu=cortex-m4','-mthumb','-g'] if p.suffix=='.S' else flags),'-c',p,'-o',obj]);objects.append(obj)
 image=work/'release.elf';mapping=work/'release.map';link=['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nostartfiles','-nodefaultlibs',*objects,'-T',HERE/'m4.ld','-Wl,--gc-sections','-Wl,-Map='+str(mapping),'-o',image];run(link)
 outputs={name:run(cmd) for name,cmd in [('symbols.txt',['arm-none-eabi-nm','-n',image]),('undefined.txt',['arm-none-eabi-nm','-u',image]),('assembly.txt',['arm-none-eabi-objdump','-d','-S',image]),('headers.txt',['arm-none-eabi-readelf','-l',image]),('size.txt',['arm-none-eabi-size',image])]}
 if outputs['undefined.txt'].strip() or re.search(r'lib(?:gcc|c|atomic)\.a|compiler-rt|crt\w*\.o',mapping.read_text(encoding='utf-8')):raise ValueError('Undeclared runtime dependency')
 for name,text in outputs.items():(work/name).write_text(text,encoding='utf-8',newline='\n')
 return image,{'image_sha256':hashlib.sha256(image.read_bytes()).hexdigest(),'sources':list(map(str,sources)),'link':list(map(str,link))}
def command(image):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-nic','none','-semihosting-config','enable=on,target=native','-kernel',str(image)]
def observations(text):
 found=re.findall(r'DT_DWT:([0-9a-f]{8}):([0-9a-f]{8}):([0-9a-f]{8})',text)
 if len(found)!=1:raise ValueError('Missing exact DWT observation')
 ctrl,before,after=map(lambda s:int(s,16),found[0]);rows=[tuple(int(x,16) for x in r) for r in re.findall(r'DT_COST:([0-9a-f]{8}):([0-9a-f]{8}):([0-9a-f]{8}):([0-9a-f]{8})',text)]
 if len(rows)!=21 or {(r[0],r[1]) for r in rows}!={(s,m) for s in range(7) for m in range(3)} or any(r[2]<=0 or r[3]!=(297 if r[1]==1 else 0) for r in rows):raise ValueError('Wrong trace workload/cost observations')
 if 'DT_RESET:retained-v1-current-token data42 bss0' not in text:raise ValueError('Missing actual reset retention')
 return {'dwt_ctrl':ctrl,'dwt_before':before,'dwt_after':after,'dwt_classification':'unsupported-capability' if ctrl&(1<<25) else 'no-observed-movement' if before==after else 'observed-movement-frequency-unverified','timer':'CMSDK timer0 at selected AN38625MHz virtual clock, not CPU CYCCNT','rows':[{'sample':s,'mode':m,'virtual_timer_ticks':ticks,'dropped':drop} for s,m,ticks,drop in rows],'physical_observation':False,'reset_scope':'Actual QEMU SYSRESETREQ with deliberately poisoned data/bss and retained noinit; not physical power cycle.'}
if __name__=='__main__':
 import argparse,json
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('source','driver','helpers','output'):p.add_argument('--'+name,type=Path,required=True)
 p.add_argument('--compiler',choices=['gcc','clang'],default='gcc');p.add_argument('--opt',choices=['0','2'],default='0');p.add_argument('--run',action='store_true');a=p.parse_args()
 def execute(cmd):return subprocess.check_output(list(map(str,cmd)),stderr=subprocess.STDOUT,text=True,timeout=60)
 image,audit=build(a.output,a.compiler,'-O'+a.opt,a.source,a.driver,a.helpers,0,execute)
 result={'audit':audit}
 if a.run:
  text=execute(command(image));result['observed']=observations(text);result['output']=text
 print(json.dumps(result,indent=2))
