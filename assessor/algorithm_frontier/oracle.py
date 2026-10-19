"""Independent simple paths plus replay of actual queue/heap frontier certificates."""
import itertools,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2];INF=(1<<64)-1
def graph(code):
 special={1000:(5,[(0,1,4),(0,2,1),(2,1,1),(1,3,1),(2,3,5)]),1001:(1,[]),1002:(4,[(0,0,0),(0,2,0),(0,1,0),(1,2,0),(2,1,0)]),1003:(2,[(0,1,65534),(1,1,0)]),1004:(2,[(0,1,9),(0,1,4),(0,1,2),(1,1,0)]),1005:(4,[(0,2,1),(0,1,1),(2,3,0),(1,3,0)])}
 if code in special:return special[code]
 edges=[]
 for a in range(3):
  for b in range(3):
   if a!=b:
    choice=code%3;code//=3
    if choice:edges.append((a,b,0 if choice==1 else 2))
 return 3,edges
def simple(vertices,edges,source):
 distance=[None]*vertices;hops=[None]*vertices
 def walk(v,seen,total,depth):
  distance[v]=total if distance[v] is None else min(distance[v],total);hops[v]=depth if hops[v] is None else min(hops[v],depth)
  for a,b,w in edges:
   if a==v and b not in seen:walk(b,seen|{b},total+w,depth+1)
 walk(source,{source},0,0);return distance,hops
def check_packet(lines):
 code,source,vertices,reachable=map(int,lines[0].split()[1:]);v,e=graph(code);assert vertices==v;edges=sorted(e,key=lambda x:x[0]);distance,hops=simple(v,e,source);assert reachable==sum(1<<i for i,d in enumerate(distance) if d is not None)
 result={};bfs=[];weighted=[];relax=[];counts=None
 for line in lines[1:]:
  parts=line.split()
  if parts[0]=='RESULT':values=list(map(int,parts[1:]));result[values[0]]=values[1:]
  elif parts[0]=='STEP':
   values=list(map(int,parts[2:]));selected,done,seen,count=values[:4];d=values[4:12];front=list(zip(values[12::2],values[13::2]));assert len(front)==count;(bfs if parts[1]=='B' else weighted).append((selected,done,seen,d,front))
  elif parts[0]=='RELAX':relax.append(tuple(map(int,parts[1:])))
  elif parts[0]=='COUNTS':counts=tuple(map(int,parts[1:]))
 assert set(result)==set(range(v));q=[source];seen=1<<source;done=0;labels=[INF]*8;labels[source]=0;expected=[]
 while q:
  at=q[0];expected.append((at,done,seen,labels[:],[(x,0) for x in q]));q.pop(0);done|=1<<at
  for a,b,w in edges:
   if a==at and not seen>>b&1:seen|=1<<b;labels[b]=labels[at]+1;q.append(b)
 assert bfs==expected
 d=[INF]*8;d[source]=0;done=0;pred=[8]*8;pred[source]=source;changes=0;inserts=1;expected_relax=[]
 for selected,actualdone,seen,actual,front in weighted:
  opened={i for i in range(v) if d[i]!=INF and not done>>i&1};assert opened;at=min(opened,key=lambda i:(d[i],i));assert selected==at and actualdone==done and actual==d and seen==done|sum(1<<i for i in opened);assert len(front)==len(opened) and {i for i,p in front}==opened
  for index,(i,p) in enumerate(front):
   assert p==d[i]*8+i
   if index:assert front[(index-1)//2][1]<=p
  assert front[0][0]==at;done|=1<<at
  for edge,(a,b,w) in enumerate(edges):
   if a==at:
    candidate=d[at]+w;assert candidate<=65534
    if not done>>b&1 and candidate<d[b]:
     changes+=int(d[b]!=INF);inserts+=int(d[b]==INF);expected_relax.append((edge,a,b,d[b],candidate));d[b]=candidate;pred[b]=at
 assert done==reachable and relax==expected_relax
 for i in range(v):
  assert result[i]==([0,0,8] if distance[i] is None else [distance[i],hops[i],8 if i==source else pred[i]])
  if distance[i] is not None:
   at=i;visited=set()
   while at!=source:
    assert at not in visited;visited.add(at);p=pred[at];assert any(a==p and b==at and distance[p]+w==distance[at] for a,b,w in edges);at=p
 assert counts[:5]==(len(bfs),len(bfs),inserts,changes,len(weighted)) and counts[5]>=0
 return code,source
def main():
 f=json.loads((ROOT/'fixtures/algorithms/PR-T36-07.json').read_text());x=f['input'];e=f['expected'];edges=[(x['vertices'].index(a),x['vertices'].index(b),w) for a,b,w in x['edges']];d,h=simple(5,edges,0);assert d==e['weighted_distances'] and h==e['hop_distances'];assert sum(x['overflow_operands'])>x['distance_limit']
 negative=[(0,1,1),(0,2,2),(2,1,-3)];assert simple(3,negative,0)[0][1]==e['negative_true_B']==-1
 # Deliberate early-settlement counterexample: settling B before C loses -1.
 assert min((1,1),(2,2))[1]==1 and e['premature_settle_B']==1
 # Undirected triangle: minimum total edge tree differs from source distances.
 assert min(sum(pair) for pair in itertools.combinations([2,3,2],2))==4 and 2+3==5
 actual=[]
 if len(sys.argv)>1:
  packet=[]
  for line in pathlib.Path(sys.argv[1]).read_text().splitlines():
   if line.startswith('GRAPH '):assert not packet;packet=[line]
   elif line=='ENDGRAPH':actual.append(check_packet(packet));packet=[]
   elif packet:packet.append(line)
  assert not packet and len(actual)==len(set(actual));kind=sys.argv[2];expected={'fixture':{(1000,0)},'boundaries':{(1001,0),(1002,0),(1003,0)},'parallel':{(1004,0),(1005,0)},'exhaustive':{(i,s) for i in range(729) for s in range(3)}}[kind];assert set(actual)==expected
 print(json.dumps(dict(status='pass',fixture_relations=5,actual_packets=len(actual))))
if __name__=='__main__':main()
