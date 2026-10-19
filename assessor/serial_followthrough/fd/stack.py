"""Nonrecursive retained-frame overbound; distinct public test selectors never nest."""
import json,pathlib,subprocess,sys
d=pathlib.Path(sys.argv[1]);live={s.split()[-1] for s in subprocess.check_output(['arm-none-eabi-nm','--defined-only',str(d/'image.elf')],text=True).splitlines() if len(s.split())==3 and s.split()[1] in ['t','T']};rows=[]
for p in sorted(d.glob('*.su')):
 for line in p.read_text().splitlines():
  f=line.split('\t');name=f[0].split(':')[-1]
  if name in live:
   assert f[-1] in ['static','dynamic,bounded'];rows.append(dict(function=name,bytes=int(f[1]),kind=f[-1]))
bound=sum(r['bytes'] for r in rows)+32
assert rows and bound<8192,(bound,rows)
print(json.dumps(dict(passed=True,bound=bound,reserve=8192,frames=rows,limits='Sum of all retained frames plus basic exception reserve; no recursion/concurrent ISR; not physical high-water measurement.')))
