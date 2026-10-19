"""Independent mathematical lower bound and explicit physical queue replay."""
import json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
def check(path,case):
 lines=[line.split() for line in path.read_text().splitlines() if not line.startswith('OBS ')];rows={}
 for line in lines:rows.setdefault(line[0],[]).append(list(map(int,line[1:])))
 if case=='kernel':
  packets=rows['KERNEL'];assert len(packets)==7560;seen=set()
  for code,direction,n,key,rank,got in packets:
   identity=(code,direction,n,key,rank);assert identity not in seen;seen.add(identity)
   values=[(i+1,(code//(3**i))%3-1) for i in range(3)];order=lambda r:((-r[1] if direction else r[1]),r[0]);values=sorted(values,key=order)[:n];assert got==sum(order(v)<order((key,rank)) for v in values)
  return dict(status='pass',packets=len(packets),meaning='exact mathematical order over emitted queries')
 if case=='queues':
  ring=[0]*3;head=count=0;nodes=[[0,-1,0] for _ in range(3)];qh=qt=-1;qc=scans=0;expected=[]
  for step,op in enumerate([11,22,0,33,44,55,0]):
   if op and count<3:
    ring[(head+count)%3]=op;count+=1;slot=next(i for i,x in enumerate(nodes) if not x[2]);scans+=slot+1;nodes[slot]=[op,-1,1]
    if qt<0:qh=slot
    else:nodes[qt][1]=slot
    qt=slot;qc+=1
   elif not op:
    head=(head+1)%3;count-=1;old=qh;qh=nodes[old][1];nodes[old][2]=0;qc-=1
    if not qc:qt=-1
   expected.append([step,op,head,count,*ring,qh,qt,qc,*sum(nodes,[]),scans])
  assert rows['QUEUE']==expected
  return dict(status='pass',packets=7,meaning='all physical ring slots and typed node links, seven free-slot inspections')
 if case=='fixture':
  fixture=json.loads((ROOT/'fixtures/algorithms/PR-T36-08.json').read_text());abstract={};states=[];statuses=[]
  for op,*args in fixture['input']['operations']:
   if op=='insert':
    identity,rank=args;status='duplicate' if identity in abstract else 'full' if len(abstract)==3 else 'ok'
    if status=='ok':abstract[identity]=rank
   else:status='ok';del abstract[args[0]]
   statuses.append(status);states.append(sorted([[k,v] for k,v in abstract.items()],key=lambda r:(r[1],r[0])))
  assert states==fixture['expected']['visible_states'] and statuses==fixture['expected']['status']
  assert rows['ORDER']==[[len(state),*sum(state,[])] for state in states]*4
  assert rows['FIXTURE']==[[0,0,4,4,0,0],[0,1,4,4,23,0],[1,0,4,4,0,0],[1,1,4,4,23,0]]
  return dict(status='pass',packets=28,meaning='four copy/borrow and callback/finite combinations, all six retained fixture states')
 if case=='backing':
  assert rows['BACKING']==[[kind,fault,6,5] for kind in range(2) for fault in range(5)];assert rows['ORDER']==[[2,7,2,9,2]]*10
  return dict(status='pass',packets=20,meaning='each update boundary fails once then retries without lost ownership')
 raise AssertionError(case)
if __name__=='__main__':print(json.dumps(check(pathlib.Path(sys.argv[1]),sys.argv[2])))
