"""Nonrecursive test call-path overbound using actual retained compiler frames."""
import json,pathlib,subprocess,sys
d=pathlib.Path(sys.argv[1]);rows=[]
symbols=subprocess.check_output(['arm-none-eabi-nm','--defined-only',str(d/'image.elf')],text=True)
live={s.split()[-1] for s in symbols.splitlines() if len(s.split())==3 and s.split()[1] in ['t','T']}
for p in sorted(d.glob('*.su')):
 for line in p.read_text().splitlines():
  fields=line.split('\t');name=fields[0].split(':')[-1]
  if name not in live:continue
  assert fields[-1] in ['static','dynamic,bounded'],line
  rows.append(dict(location=fields[0],function=name,bytes=int(fields[1]),kind=fields[-1]))
# Independent selectors never nest; callbacks only access their local RAM model.
alternatives=[r['bytes'] for r in rows if r['function'] in ['config','timing','events','cancellation','link_case','r5_mapped']]
bound=sum(r['bytes'] for r in rows)-sum(alternatives)+max(alternatives,default=0)+32
assert rows and bound<8192,(bound,rows)
print(json.dumps(dict(passed=True,bound=bound,reserve=8192,frames=rows,limits='IRQs masked, no recursion or physical high-water mark')))
