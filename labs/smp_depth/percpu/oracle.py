"""Independent numeric-layout expectations and actual per-CPU trace checks."""
import itertools,re
MAX=2**64-1
def cases():
 rows=[]
 for base,n,cpu,size,a in itertools.product((0,3,0x100003,MAX-15),(0,10000,300000,MAX),(0,1,2,4,5),(511,512,2048,8192),(7,8,16,64)):
  rows.append((0,base,n,cpu,size,a,0))
 for f0,f1,count,align,hardware in itertools.product((0,7,33,MAX),(0,48,MAX-15),(0,1,2,3),(0,8,16,32),(0,40,MAX)):
  rows.append((1,f0,f1,count,64,align,hardware))
 return rows
def inputs():return [' '.join(map(str,r)) for r in cases()]
def expected(row):
 kind,a,b,cpu,size,align,hw=row
 if kind==0:
  if not 1<=cpu<=4 or not 8<=align<=4096 or align&(align-1) or size<512 or size%align:return 'SP 1 0'
  if a+b>MAX:return 'SP 2 0'
  at=a;spans=[]
  for c in range(cpu):
   for k in range(9):
    stack=k<4 or k>=7;n=size if stack else 56 if k==4 else 104 if k==5 else 4096;al=align if stack else 16 if k==6 else 8
    begin=(at+al-1)//al*al;end=begin+n
    if at+al-1>MAX or end>MAX:return 'SP 2 0'
    if end>a+b:return 'SP 4 0'
    spans.append((begin,end));at=end
  return 'SP 0 '+str(at)+''.join(' '+str(lo)+':'+str(hi) for lo,hi in spans)
 if not align or align&(align-1) or align>4096:return 'SP 1 1193046'
 total=hw+120+align-1+512
 if total>MAX:return 'SP 2 1193046'
 for f in (a,b,size)[:cpu]:
  rounded=(f+align-1)//align*align
  if f+align-1>MAX or total+rounded>MAX:return 'SP 2 1193046'
  total+=rounded
 return 'SP 0 '+str(total)
def check_model(text):
 rows=cases();lines=text.splitlines()
 if len(lines)!=len(rows):raise ValueError('Missing/extra numeric row')
 for row,line in zip(rows,lines):
  if line!=expected(row):raise ValueError('Wrong checked layout/budget '+str(row))
 return {'rows':len(rows),'scope':'Finite numeric parameter grid and exact unbounded-integer oracle, not memory mapping or arbitrary stack proof.'}
def check_guest(text,cpus,simd):
 if text.count('SP_GUEST_PASS:')!=1 or 'SP_GUEST_CHECK:' in text or 'FAIL' in text or 'simd='+str(simd) not in text:raise ValueError('Wrong target completion')
 parsed=re.findall(r'SP_RANGE cpu=(\d+) kind=(\d+) low=(\d+):(\d+) high=(\d+):(\d+)',text);spans={}
 for c,k,lh,ll,hh,hl in parsed:
  key=(int(c),int(k));low=int(lh)*2**32+int(ll);high=int(hh)*2**32+int(hl)
  if key in spans or not low<high:raise ValueError('Duplicate/invalid actual extent')
  if any(low<h and l<high for l,h in spans.values()):raise ValueError('Actual cross-CPU/stack/table alias')
  spans[key]=(low,high)
 if set(spans)!={(c,k) for c in range(cpus) for k in range(9)}:raise ValueError('Missing actual storage extent')
 rows=re.findall(r'SP_CPU index=(\d+) apic=(\d+) irq=(\d+) ud=(\d+) gp=(\d+) progress=(\d+) peer_before=(\d+) peer_after=(\d+) irq_frame=(\d+):(\d+) ud_frame=(\d+):(\d+) gp_frame=(\d+):(\d+)',text)
 if len(rows)!=cpus:raise ValueError('Wrong CPU observations')
 fault=1 if cpus>1 else 0;ids=[0,1,2,4];seen=set();result=[]
 for row in rows:
  c,apic,irq,ud,gp,progress,before,after,ih,il,uh,ul,gh,gl=map(int,row)
  if c in seen or c>=cpus or apic!=ids[c] or irq<128 or progress!=irq or (ud,gp)!=((1,1) if c==fault else (0,0)):raise ValueError('Wrong CPU identity/register/fault accounting')
  seen.add(c)
  if c==fault and cpus>1 and after<before+4:raise ValueError('No actual peer progress during controlled fault')
  for addr,kind,active in ((ih*2**32+il,2,True),(uh*2**32+ul,8,c==fault),(gh*2**32+gl,3,c==fault)):
   low,high=spans[c,kind]
   if active and not low+32<=addr<=high-176 or not active and addr:raise ValueError('Wrong IST stack/frame ownership')
  result.append({'cpu':c,'apic':apic,'interrupts':irq,'progress':progress,'faults':[ud,gp],'peer_before':before,'peer_after':after})
 return {'cpus':result,'spans':[{'cpu':c,'kind':k,'low':lo,'high':hi} for (c,k),(lo,hi) in sorted(spans.items())]}
def check_cost(text):
 rows=re.findall(r'SP_COST pair=(\d+) padded=(\d+) cpus=(\d+),(\d+) counts=(\d+),(\d+) elapsed_ns=(\d+) stride=(\d+) line=(-?\d+)',text)
 if len(rows)!=20 or {(int(r[0]),int(r[1])) for r in rows}!={(p,a) for p in range(10) for a in (0,1)}:raise ValueError('Missing paired cost sample')
 for pair,pad,c0,c1,n0,n1,elapsed,stride,line in rows:
  if c0==c1 or (int(n0),int(n1))!=(500000,500000) or int(elapsed)<=0 or int(stride)!=(128 if int(pad) else 8):raise ValueError('Unequal correctness/workload cost contract')
 return {'pairs':10,'operations_per_counter':500000,'scope':'Two allowed logical CPUs, real host measurements; no required speed ratio, physical-core topology or universal performance claim.'}
