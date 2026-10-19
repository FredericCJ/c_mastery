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
# main calls these four entries sequentially; none calls any other entry.
# Keep all their callees in the common sum (an overestimate), but use the
# maximum of their own frames. If one is inlined its frame is already in main;
# summing that main frame plus the maximum remaining entry still overcounts.
# This is a selected-call-tree bound, not a general call-graph analyzer.
exclusive={n:frames[n] for n in ['arithmetic','failures','pending','calls','interleave'] if n in frames}
bound=sum(v for n,v in frames.items() if n not in exclusive)+max(exclusive.values(),default=0)+1024
undefined=subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True)
required={'vs_start','vs_step','vs_complete','vv_verify','cvm_run'}
result=dict(passed=required<=live and not dynamic and not undefined.strip() and bound<32768,frames=frames,mutually_exclusive_entry_frames=exclusive,conservative_bytes=bound,reserve=32768,dynamic=dynamic,undefined=undefined,scope='Selected nonrecursive execution entry; sum all common live C frames plus maximum of five sequential test entries plus1024; IRQ masked; no physical watermark or arbitrary callbacks.')
(p/'resource-audit.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));raise SystemExit(not result['passed'])
