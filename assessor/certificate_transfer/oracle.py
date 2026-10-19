"""Independent all-descendant/leaf-elimination tree oracle, seed 360021."""
import copy,json,pathlib,random,subprocess,sys
def expected(c):
 policy,cap,count,root,nodes=c
 if not 1<=cap<=64 or count>cap or policy>2:return None
 live={i for i,n in enumerate(nodes) if n[5]}
 if len(live)!=count:return None
 if root==-1:return 0 if not live else None
 if root not in live or nodes[root][3]!=-1:return None
 children={i:set() for i in live};incoming={i:0 for i in live}
 for i in live:
  key,left,right,parent,height,on,red=nodes[i]
  if red not in (0,1):return None
  for child in [left,right]:
   if child==-1:continue
   if child not in live or nodes[child][3]!=i:return None
   incoming[child]+=1;children[i].add(child)
 if any(incoming[i]!=(0 if i==root else 1) for i in live):return None
 descendants={i:set(children[i]) for i in live}
 for _ in range(cap):
  for i in live:
   descendants[i]|={j for k in tuple(descendants[i]) for j in descendants[k]}
 if any(i in descendants[i] for i in live) or descendants[root]|{root}!=live:return None
 for i in live:
  for side in [1,2]:
   child=nodes[i][side]
   if child!=-1:
    for j in descendants[child]|{child}:
     if (nodes[j][0]>=nodes[i][0] if side==1 else nodes[j][0]<=nodes[i][0]):return None
 heights={-1:0};black={-1:1};pending=set(live)
 while pending:
  ready={i for i in pending if children[i]<=heights.keys()}
  if not ready:return None
  for i in ready:
   _,l,r,_,h,_,red=nodes[i];actual=1+max(heights[l],heights[r])
   if policy!=2 and h!=actual:return None
   if policy==1 and abs(heights[l]-heights[r])>1:return None
   if policy==2 and (black[l]!=black[r] or(red and any(j!=-1 and nodes[j][6] for j in [l,r]))):return None
   heights[i]=actual;black[i]=black[l]+(not red)
  pending-=ready
 if policy==2 and nodes[root][6]:return None
 return heights[root]
def packets():
 rng=random.Random(360021);out=[]
 for policy in range(3):
  for depth in range(1,6):
   count=2**depth-1;nodes=[]
   # Complete heap shape; inorder rank supplies strict search ordering.
   ranks={};order=[]
   def walk(i):
    if i>=count:return
    walk(2*i+1);order.append(i);walk(2*i+2)
   walk(0);ranks={i:k*2-count for k,i in enumerate(order)}
   for i in range(count):nodes.append([ranks[i],2*i+1 if 2*i+1<count else -1,2*i+2 if 2*i+2<count else -1,(i-1)//2 if i else -1,depth-(i+1).bit_length()+1,1,0])
   base=[policy,count,count,0,nodes];out.append(base)
   for _ in range(160):
    c=copy.deepcopy(base);i=rng.randrange(count);field=rng.randrange(7)
    c[4][i][field]=rng.choice([-2,-1,0,1,2,count-1,count,count+1,2147483647]) if field<5 else rng.randrange(4)
    out.append(c)
 for policy in range(3):out.extend([[policy,1,0,-1,[[0,-1,-1,-1,0,0,0]]],[policy,1,1,0,[[-2147483648,-1,-1,-1,1,1,0]]]])
 return out
def main():
 values=packets();text=''.join(' '.join(map(str,c[:4]))+' '+' '.join(str(x) for n in c[4] for x in n)+'\n' for c in values)
 p=subprocess.run([sys.argv[1]],input=text,text=True,capture_output=True,timeout=30);lines=p.stdout.splitlines();errors=[];accepted=0
 if p.returncode or len(lines)!=len(values):errors.append('transport/count mismatch')
 for i,(c,line) in enumerate(zip(values,lines)):
  e=expected(c);accepted+=e is not None;status,h=map(int,line.split())
  if (status==0)!=(e is not None) or h!=(e if e is not None else 777):errors.append(i)
 result=dict(passed=not errors,seed=360021,cases=len(values),accepted=accepted,rejected=len(values)-accepted,errors=errors,limits='Bounded synthetic certificates; separate all-descendant closure and leaf elimination, not an insertion implementation.')
 pathlib.Path(sys.argv[2]).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
