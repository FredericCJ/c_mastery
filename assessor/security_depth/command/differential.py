"""Independent grammar-to-value oracle; no implementation extraction or C call reuse."""
import itertools,json,subprocess,sys
OK,LENGTH,OPCODE,COUNT,DENIED,BUDGET=range(6)
INVALID=8
class Reject(Exception): pass
def need(condition,status):
 if not condition:raise Reject(status)
def oracle(packet,extended,allowed,generation):
 commands=[];work=0
 def node(p,depth):
  nonlocal work
  need(len(p)>=2,LENGTH);op,count=p[:2]
  if extended and op==3:
   need(depth<2,BUDGET);need(count<=4,COUNT);children=[];tail=p[2:]
   for _ in range(count):
    need(bool(tail),LENGTH);length=tail[0];tail=tail[1:];need(length<=len(tail),LENGTH)
    # Process in wire order: earlier child's error precedes malformed later child.
    child,tail=tail[:length],tail[length:];node(child,depth+1)
   need(not tail,LENGTH);return
  if extended and op==5:
   need(len(p)==4,LENGTH);need(count<=4,BUDGET);cost=max(1,count)
   need(work+cost<=4 and len(commands)<4,BUDGET)
   commands.append((1,count,0,[int.from_bytes(bytes(p[2:]),'big')]*count));work+=cost;return
  need(op in (1,2) or (extended and op==4),OPCODE);need(count<=4,COUNT)
  opt=0;offset=2
  if op==4:
   need(len(p)>=4,LENGTH);need(p[3] in (0,1),INVALID);opt=p[3];offset=4;op=2
  need(len(p)==offset+2*(count+opt),LENGTH);cost=max(1,count)+opt
  need(work+cost<=4 and len(commands)<4,BUDGET)
  need(op!=2 or (allowed and generation!=2**32-1),DENIED)
  values=[int.from_bytes(bytes(p[j:j+2]),'big') for j in range(offset,offset+2*count,2)]
  commands.append((op,count,generation if op==2 else 0,values));work+=cost
 try:
  need(generation!=0 and allowed in (0,1),INVALID)
  need(len(packet)<=32,BUDGET if extended else LENGTH);node(packet,0)
  return [OK,len(commands),work]+[x for op,count,gen,v in commands for x in [op,count,gen,*v]]
 except Reject as e:return [e.args[0]]
def cases():
 rows=[]
 for ext,allowed,op,count,n in itertools.product(range(2),range(2),range(7),range(7),range(13)):
  rows.append((ext,allowed,1,([op,count]+list(range(2,13)))[:n]))
 leaves=[[op,count]+[x for i in range(count) for x in [0,i+7]] for op,count in itertools.product([1,2],range(5))]
 def wrap(parts):return [3,len(parts)]+[x for p in parts for x in [len(p),*p]]
 trees=leaves+[wrap([p]) for p in leaves]+[wrap([p,q]) for p,q in itertools.product(leaves,repeat=2)]+[wrap([wrap([p])]) for p in leaves]+[wrap([wrap([wrap([p])])]) for p in leaves]
 trees += [[4,c,role,opt]+[x for _ in range(c+opt) for x in [255,255]] for c,role,opt in itertools.product(range(6),[0,1,255],[0,1,2])]
 trees += [[5,c,255,255] for c in [0,1,4,5,255]]
 for p in trees:
  for allowed in [0,1]:
   for n in range(len(p)+1):rows.append((1,allowed,7,p[:n]))
   rows.append((1,allowed,7,p+[0]));rows.append((1,allowed,2**32-1,p))
   for i in range(len(p)):
    q=p.copy();q[i]=255;rows.append((1,allowed,7,q))
 rows += [(1,1,1,[1]*33),(0,1,1,[1]*33),(1,2,1,[1,0]),(1,1,0,[1,0])]
 return list(dict.fromkeys((e,a,g,tuple(p)) for e,a,g,p in rows))
def main():
 rows=cases();wire=''.join(' '.join(map(str,[e,a,g,len(p),*p]))+'\n' for e,a,g,p in rows)
 r=subprocess.run([sys.argv[1]],input=wire,text=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=30)
 assert r.returncode==0,(r.returncode,r.stderr)
 output=[list(map(int,s.split())) for s in r.stdout.splitlines()];assert len(output)==len(rows)
 for row,actual in zip(rows,output):
  e,a,g,p=row;wanted=oracle(list(p),e,a,g);assert actual==wanted,(row,actual,wanted)
 print(json.dumps(dict(status='pass',cases=len(rows),requests=[1,2,3,4,5],oracle='Independent recursive grammar and integer byte decoding',limits=['Finite enumeration and structured mutations, not exhaustive arbitrary packets.','Harness uses 32-bit unsigned on the selected Linux ABI; UINT_MAX boundary asserted by native tests.'])))
if __name__=='__main__':main()
