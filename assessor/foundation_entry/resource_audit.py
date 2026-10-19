"""Audit every actual object and linked image in the selected M4 reference driver."""
from pathlib import Path
import json,subprocess,sys
p=Path(sys.argv[1]);elf=p/'image.elf'
def run(*cmd):return subprocess.check_output(list(map(str,cmd)),text=True)
defined=run('arm-none-eabi-nm','--defined-only',elf);undefined=run('arm-none-eabi-nm','-u',elf)
live={x.split()[-1] for x in defined.splitlines() if len(x.split())>=3 and x.split()[-2].lower()=='t'}
allowed={'f_prefix','memcpy','memset','memmove','__aeabi_memclr','__aeabi_memclr4','__aeabi_memclr8','__aeabi_memcpy','__aeabi_memcpy4','__aeabi_memcpy8','fe_target_main','fe_fault','__stack_top','__data_load','__data_start','__data_end','__bss_start','__bss_end'}|{n for n in live if n.startswith('fe_')}
object_undefined={str(i):run('arm-none-eabi-nm','-u',p/(str(i)+'.o')).splitlines() for i in range(5)}
bad=[line for lines in object_undefined.values() for line in lines if line.split()[-1] not in allowed]
frames={};dynamic=[]
for f in p.glob('*.su'):
 for row in f.read_text().splitlines():
  parts=row.split('\t')
  if len(parts)!=3:continue
  name=parts[0].rsplit(':',1)[-1]
  if name not in live:continue
  if parts[2]!='static':dynamic.append(row)
  frames[name]=max(frames.get(name,0),int(parts[1]))
# Only recursive() recurses, depth at most17. Every other selected frame is
# counted once, including callbacks and all unused provider functions. IRQs are
# masked by startup. Inlining/clones are included by their emitted .su names.
bound=sum(v*(17 if k.startswith('recursive') else 1) for k,v in frames.items())+1024
build=json.loads((p/'build.json').read_text());link=build['commands'][-1]
maptext=(p/'image.map').read_text();required={'fe_fixed','fe_recursive','fe_explicit_stack','fe_two_pass','fe_staged','fe_maximum','fe_encode24','fe_any','fe_all','fe_forbidden_clear','fe_saturating_add','f_prefix'}
ok=not undefined.strip() and not bad and required<=live and bool(frames) and not dynamic and bound<32768 and link[0]=='arm-none-eabi-ld' and sum(x.endswith('.o') for x in link)==5 and not any(x.endswith('.a') or x.startswith('-l') for x in link) and '.a(' not in maptext
d=dict(passed=ok,object_undefined=object_undefined,unexpected=bad,final_undefined=undefined,frames=frames,conservative_bytes=bound,reserve=32768,dynamic=dynamic,scope='Selected IRQ-masked M4 driver: only recursive() depth17; all other live C frames counted once plus1024. Explicit original memory helpers only, no archive. No physical watermark or arbitrary callbacks.')
(p/'resource-audit.json').write_text(json.dumps(d,indent=2)+'\n');print(json.dumps(d));raise SystemExit(not ok)
