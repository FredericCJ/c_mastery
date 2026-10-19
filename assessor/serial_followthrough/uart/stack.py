"""Conservative sum of all nonrecursive emitted C frame sizes, plus vector wrapper."""
import json,pathlib,subprocess,sys
d=pathlib.Path(sys.argv[1]);rows=[]
symbols=subprocess.check_output(['arm-none-eabi-nm','--defined-only',str(d/'image.elf')],text=True)
live={s.split()[-1] for s in symbols.splitlines() if len(s.split())==3 and s.split()[1] in ['t','T']}
for p in sorted(d.glob('*.su')):
 for line in p.read_text().splitlines():
  fields=line.split('\t');name=fields[0].split(':')[-1]
  if name not in live:continue
  assert fields[-1] in ['static','dynamic,bounded'],line;rows.append(dict(location=fields[0],function=name,bytes=int(fields[1]),kind=fields[-1]))
# Summing all compiler frames overbounds every simple C call path here. There are
# no recursive callbacks: test callbacks only access model storage. IRQs masked.
# The five ua_cases alternatives and ua_mapped are sequential, never nested.
# Optimizer-inlined alternatives are already accounted in the retained caller.
alternatives=[x['bytes'] for x in rows if x['function'] in ['frames','baud','transmission','receive','deadline','ua_mapped']]
bound=sum(x['bytes'] for x in rows)-sum(alternatives)+max(alternatives,default=0)+32
assert rows and bound<8192,(bound,rows)
print(json.dumps(dict(passed=True,bound=bound,reserve=8192,frames=rows,limits='No recursion, IRQ nesting or physical stack watermark claim')))
