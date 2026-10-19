"""Conservative sum bound for foreground plus one nonnested IRQ0 invocation."""
import json,pathlib,subprocess,sys
d=pathlib.Path(sys.argv[1]);symbols=subprocess.check_output(['arm-none-eabi-nm','--defined-only',str(d/'image.elf')],text=True);live={s.split()[-1] for s in symbols.splitlines() if len(s.split())==3 and s.split()[1] in ['t','T']};rows=[]
for p in sorted(d.glob('*.su')):
 for line in p.read_text().splitlines():
  f=line.split('\t');name=f[0].split(':')[-1]
  if name not in live:continue
  assert f[-1] in ['static','dynamic,bounded'],line
  rows.append(dict(function=name,bytes=int(f[1]),kind=f[-1]))
# These test roots are sequential children of xf_target_main, never call each
# other, and are never entered from IRQ0. Bound the maximum one root plus every
# other retained frame twice for interrupted foreground and one IRQ call chain.
roots={'ss_edges','ss_queue','ss_quiescence','ss_irq_prepare','ss_irq_verify','ss_device','ss_mapped'}
root_frames=[x for x in rows if x['function'] in roots]
other_frames=[x for x in rows if x['function'] not in roots]
assert root_frames
bound=max(x['bytes'] for x in root_frames)+2*sum(x['bytes'] for x in other_frames)+44
assert rows and bound<8192,(bound,rows)
print(json.dumps(dict(passed=True,bound=bound,reserve=8192,frames=rows,sequential_roots=sorted(roots),limits='One basic exception; no floating point, recursion, nested IRQ, physical watermark or timing claim')))
