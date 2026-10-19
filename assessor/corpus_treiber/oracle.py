"""Independent list/slot-state machine and finite real-time LIFO histories."""
import itertools,json,random,subprocess
def traces(driver):
 total=0
 for capacity in [1,2,8]:
  for seed in range(8):
   rng=random.Random(seed);stack=[];free=capacity;retired=0;ops=[];expected=[]
   for _ in range(512):
    op=rng.choice('PPOOC');v=rng.randrange(2**32);ops.append((op,v));out=999
    if op=='P':
     if free:free-=1;stack.append(v);status=0
     else:status=2
    elif op=='O':
     if stack:out=stack.pop();retired+=1;status=0
     else:status=1
    else:out=retired;free+=retired;retired=0;status=0
    expected.append((status,out))
   data=f'{capacity} {len(ops)}\n'+''.join(f'{op} {v}\n' for op,v in ops)
   p=subprocess.run([str(driver),'driver'],input=data,text=True,capture_output=True,timeout=30)
   assert p.returncode==0,p.stderr
   actual=[tuple(map(int,line.split())) for line in p.stdout.splitlines()]
   assert actual==expected,(capacity,seed,next((i,a,e) for i,(a,e) in enumerate(zip(actual,expected)) if a!=e))
   total+=len(ops)
 return total
def legal(rows):
 for order in itertools.permutations(range(4)):
  positions={n:i for i,n in enumerate(order)}
  if any(a[1]<b[0] and positions[i]>positions[j] for i,a in enumerate(rows) for j,b in enumerate(rows)):continue
  stack=[];ok=True
  for i in order:
   _,_,status,value=rows[i]
   if i%2==0:
    if status:ok=False;break
    stack.append(value)
   elif not stack or status or stack.pop()!=value:ok=False;break
  if ok:return True
 return False
def run(driver):
 count=traces(driver)
 p=subprocess.run([str(driver),'history'],text=True,capture_output=True,timeout=60);assert p.returncode==0,p.stderr
 histories=[]
 for line in p.stdout.splitlines():
  values=list(map(int,line.split()));assert len(values)==16
  rows=[values[i:i+4] for i in range(0,16,4)];assert legal(rows),rows;histories.append(rows)
 assert len(histories)==256
 assert not legal([[0,1,0,10],[4,5,0,10],[2,3,0,11],[6,7,0,11]]) # FIFO result violates real-time LIFO
 return dict(sequential_transitions=count,actual_concurrent_histories=len(histories),linearization_candidates_per_history=24,scope='Finite list oracle and actual real-time histories; not a general linearizability or progress proof.')
