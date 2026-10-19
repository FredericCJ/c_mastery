"""Independent list/stack-distance cache certificates and exact rational costs."""
import collections,json,pathlib,sys
from fractions import Fraction
def trace(rows,key):
 head=next(r for r in rows['head'] if r[0]==key);_,n,lines,blocksize,ways,fifo,*totals=head;sets=lines//ways;banks=[[] for _ in range(sets)];shadow=[];seen=set();history=[];counts=[0]*4
 steps=sorted((r for r in rows['step'] if r[0]==key),key=lambda r:r[1]);assert len(steps)==n
 for i,row in enumerate(steps):
  _,index,address,offset,bank,tag,category,used,*state=row;number,off=divmod(address,blocksize);tag0,bank0=divmod(number,sets);assert (index,offset,bank,tag)==(i,off,bank0,tag0)
  target=banks[bank];hit=number in target
  if not fifo and number in seen:
   previous=max(j for j,v in enumerate(history) if v==number);assert (number in shadow)==(len(set(history[previous+1:]))<lines)
  expected=0 if hit else 1 if number not in seen else 2 if number in shadow else 3;assert category==expected;counts[category]+=1
  if hit:target.remove(number)
  target.append(number)
  if len(target)>ways:target.pop(0)
  if number not in shadow:shadow.append(number)
  elif not fifo:shadow.remove(number);shadow.append(number)
  if len(shadow)>lines:shadow.pop(0)
  blocks=[];valid=[]
  for target in banks:blocks+=list(reversed(target))+[0]*(ways-len(target));valid+=[1]*len(target)+[0]*(ways-len(target))
  blocks+=[0]*(8-lines);valid+=[0]*(8-lines);shadow_state=list(reversed(shadow))+[0]*(8-len(shadow));assert used==len(shadow) and state==blocks+valid+shadow_state;seen.add(number);history.append(number)
 assert counts==totals
 if key==0:assert counts==[3,4,2,0] and [r[6] for r in steps]==[1,1,0,1,2,0,1,2,0]
 if key==1:assert steps[-1][6]==3
 if key==2:assert counts==[5,4,0,0]
 if key in [3,4]:assert steps[-1][6]==(2 if key==3 else 3)
 if key in [10,11,12,13]:assert counts[0]=={10:0,11:14,12:48,13:0}[key]
 return dict(key=key,accesses=n,hits=counts[0],compulsory=counts[1],conflict=counts[2],capacity=counts[3])
def overlap(rows):
 results=[]
 for mode in range(3):
  for slots in range(1,5):
   packet=sorted((r for r in rows['overlap'] if r[:2]==[mode,slots]),key=lambda r:r[2]);assert len(packet)==4;inputs=[(r[3],r[4],r[5]-1) for r in packet];waiting=set(range(4));finish={};start={};time=0
   while waiting:
    active=[t for t in finish.values() if t>time];ready=sorted(i for i in waiting if inputs[i][0]<=time and (inputs[i][2]<0 or inputs[i][2] in finish and finish[inputs[i][2]]<=time))
    if len(active)<slots and ready:
     i=ready[0];start[i]=time;finish[i]=time+inputs[i][1];waiting.remove(i);time+=1
    else:
     future=active+[inputs[i][0] for i in waiting if inputs[i][0]>time];assert future;time=min(future)
   makespan=max(finish.values());assert all(r[6:]==[start[i],finish[i],makespan] for i,r in enumerate(packet));assert sum(v[1] for v in inputs)==(80 if mode<2 else 38);results.append([mode,slots,makespan])
 return results
def main():
 path=pathlib.Path(sys.argv[1]);case=sys.argv[2];rows=collections.defaultdict(list)
 if case=='resource':
  entries={line.split('\t')[0].split(':')[-1]:(int(line.split('\t')[-2]),line.split('\t')[-1]) for line in path.read_text().splitlines()};assert all(n in entries and 0<entries[n][0]<32768 and entries[n][1] in ['static','dynamic,bounded'] for n in ['ach_trace','ach_cost','ach_overlap']);print(json.dumps(dict(status='pass',entries=entries,physical=False,limits='Bounded native compiler estimates for individual functions; not full call-stack or physical high-water.')));return
 for line in path.read_text().splitlines():
  if line.startswith('DATA '):_,kind,*values=line.split();rows[kind].append(list(map(int,values)))
 if case in ['ordinary','layouts','geometry']:
  keys=sorted(r[0] for r in rows['head']);assert keys==({'ordinary':list(range(5)),'layouts':list(range(10,14)),'geometry':list(range(100,160))}[case]);results=[trace(rows,k) for k in keys];result=dict(status='pass',packets=len(results),traces=results)
 elif case=='costs':
  expected=[[Fraction(x) for x in [21,120,120,240,120]],[Fraction(x) for x in [21,120,120,240,120]],[Fraction(47,14),Fraction(21),Fraction(10),Fraction(31),Fraction(31)]];assert len(rows['cost'])==3
  for row in rows['cost']:assert [Fraction(row[i],row[i+1]) for i in range(1,11,2)]==expected[row[0]]
  assert Fraction(1)+Fraction(6,9)*30==21 and (Fraction(100)*Fraction(6,5)+24*Fraction(1,4)*20)*Fraction(10**9,2*10**9)==120;result=dict(status='pass',packets=3,units=['ns','cycles','cycles','cycles','ns'])
 elif case=='overlap':result=dict(status='pass',packets=12,schedules=overlap(rows))
 else:raise AssertionError(case)
 print(json.dumps(result))
if __name__=='__main__':main()
