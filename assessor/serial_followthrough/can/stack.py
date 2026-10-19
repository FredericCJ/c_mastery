"""Conservative sum of retained nonrecursive compiler frames plus exception reserve."""
import json,pathlib,re,subprocess,sys
d=pathlib.Path(sys.argv[1]);live={s.split()[-1] for s in subprocess.check_output(['arm-none-eabi-nm','--defined-only',str(d/'image.elf')],text=True).splitlines() if len(s.split())==3 and s.split()[1] in ['t','T']};rows=[]
for p in sorted(d.glob('*.su')):
 for line in p.read_text().splitlines():
  f=line.split('\t');name=f[0].split(':')[-1]
  if name in live:
   assert f[-1] in ['static','dynamic,bounded'];rows.append(dict(function=name,bytes=int(f[1]),kind=f[-1]))
alternatives={'arbitration','codec','stream','fault','timing','fifo','mailbox','recovery','port'}
# These are distinct selectors, invoked sequentially by xf_target_main. Verify
# retained branch bodies never directly call another selector; all remaining
# frames are still summed, including helper/model temporaries and dispatch.
asm=subprocess.check_output(['arm-none-eabi-objdump','-d',str(d/'image.elf')],text=True)
current=None
for line in asm.splitlines():
 m=re.match(r'^[0-9a-f]+ <([^>]+)>:',line)
 if m:current=m.group(1)
 if current in alternatives and re.search(r'\bblx?\b',line):
  target=re.search(r'<([^>+]+)',line)
  assert not target or target.group(1) not in alternatives,(current,line)
sizes=[r['bytes'] for r in rows if r['function'] in alternatives]
bound=sum(r['bytes'] for r in rows)-sum(sizes)+max(sizes,default=0)+32
assert rows and bound<8192,(bound,rows)
print(json.dumps(dict(passed=True,bound=bound,reserve=8192,frames=rows,exclusive_selectors=sorted(alternatives),limits='Maximum of nonnested selectors plus all other retained frames; no recursion/reentrant callbacks/interrupt activity; not physical high-water measurement.')))
