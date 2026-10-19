"""Exact packet comparison from the peer and ownership contracts, not C output."""
import json,pathlib,sys
path,key=sys.argv[1:];actual={}
for line in pathlib.Path(path).read_text().splitlines():
 if line.startswith('OBS '):
  _,name,*values=line.split();actual.setdefault(name,[]).append(list(map(int,values)))
expected=dict(edges=[[4*256*256,0x12*256+0x34,0xc0+3,2]],queue=[[n,n,n,2] for n in range(1,17)],send_only=[[0x11,64,0,2]],quiescence=[[order,4,3,1] for order in range(2)],irq_stale=[[3,1,0,1]],irq_exception=[[1,16,1,0]],device=[[n,n,int(n==3),0x1234 if n==3 else 0x7777] for n in range(1,4)],mapped=[[3*2**25+2**13+2**11+2**10+1,3*2**25+2**13+2**11+2**10+2**6+1,3,6]])
expected['mapped_scheduler']=[[int(key=='all'),6,2,1]]
keys={'edges':['edges'],'queue':['queue','send_only'],'quiescence':['quiescence'],'irq':['irq_stale'],'device':['device'],'mapped':['mapped','mapped_scheduler'],'all':list(expected)}[key]
wanted={k:expected[k] for k in keys};assert actual==wanted,(actual,wanted)
print(json.dumps(dict(status='pass',keys=keys,packets=sum(map(len,wanted.values())),limits='Exact bounded semantic packets; no physical or all-history proof')))
