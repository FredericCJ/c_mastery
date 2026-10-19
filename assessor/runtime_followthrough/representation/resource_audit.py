"""Conservative nonrecursive selected-entry stack sum, plus exact live closure."""
from pathlib import Path
import json,re,subprocess,sys
p=Path(sys.argv[1]);elf=p/'image.elf'
symbols=subprocess.check_output(['arm-none-eabi-nm','--defined-only',str(elf)],text=True)
live={line.split()[-1] for line in symbols.splitlines() if len(line.split())>=3 and line.split()[-2].lower()=='t'}
frames={};dynamic=[]
for file in p.glob('*.su'):
 for row in file.read_text().splitlines():
  parts=row.split('\t')
  if len(parts)!=3:continue
  name=parts[0].rsplit(':',1)[-1]
  if name not in live:continue
  if parts[2]!='static':dynamic.append(row)
  frames[name]=max(frames.get(name,0),int(parts[1]))
# No recursion, dynamic alloca, interrupt or arbitrary callback is reached by
# these representation tests. Summing every live C frame overcounts siblings;
# add1024 for startup/ABI bridge and exception reserve. This is no WCET proof.
bound=sum(frames.values())+1024
undefined=subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True)
required={'rv_decode','rv_encode','rv_arithmetic','cvm_decode','cvm_encode'}
result=dict(passed=required<=live and not dynamic and not undefined.strip() and bound<32768,frames=frames,conservative_bytes=bound,reserve=32768,dynamic=dynamic,undefined=undefined,scope='Selected nonrecursive representation entry only; all live C frames plus1024; IRQ masked; no physical watermark or arbitrary callbacks.')
(p/'resource-audit.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));raise SystemExit(not result['passed'])
