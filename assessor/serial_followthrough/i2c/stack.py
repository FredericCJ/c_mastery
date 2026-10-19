"""All retained nonrecursive frames overbound every masked-IRQ call chain."""
import json,pathlib,subprocess,sys
d=pathlib.Path(sys.argv[1]);symbols=subprocess.check_output(['arm-none-eabi-nm','--defined-only',str(d/'image.elf')],text=True);live={s.split()[-1] for s in symbols.splitlines() if len(s.split())==3 and s.split()[1] in ['t','T']};rows=[]
for p in sorted(d.glob('*.su')):
 for line in p.read_text().splitlines():
  f=line.split('\t');name=f[0].split(':')[-1]
  if name not in live:continue
  assert f[-1] in ['static','dynamic,bounded'],line
  rows.append(dict(function=name,bytes=int(f[1]),kind=f[-1]))
bound=sum(x['bytes'] for x in rows)+32
assert rows and bound<8192,(bound,rows)
print(json.dumps(dict(passed=True,bound=bound,reserve=8192,frames=rows,limits='No recursion, IRQ nesting, physical watermark or timing claim')))
