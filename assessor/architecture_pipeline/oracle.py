"""Independent stage certificates and renamed-dataflow/retirement checks."""
import collections,json,pathlib,sys
PROGRAM=[(0,1,0,0,0,0),(0,2,1,0,0,0),(1,3,2,0,0,0),(0,4,3,0,0,0),(2,0,4,0,6,1),(0,5,4,0,0,0),(0,6,4,0,0,0)]
def setup(key):
 if key<32:return PROGRAM,(key//16,(key//8)%2+1,(key//4)%2,(key//2)%2,key%2)
 table={100:(PROGRAM,(1,1,1,0,0)),101:(PROGRAM[:4],(0,1,1,0,0)),102:(PROGRAM[:4],(1,1,1,0,0)),103:(PROGRAM[:4],(0,1,0,0,0)),104:(PROGRAM,(1,2,1,0,0)),105:(PROGRAM,(1,1,1,1,0)),200:([(1,1,0,0,0,0),(0,2,0,0,0,0),(0,3,0,0,0,0),(0,4,0,0,0,0)],(1,1,1,0,1))}
 return table[key]
def schedule(rows,key):
 data={kind:[r[1:] for r in values if r[0]==key] for kind,values in rows.items()};head=data['head'];assert len(head)==1;cycles,count,retired_count,flushed_count,*_=head[0];instances={r[0]:r[1:] for r in data['instance']};assert set(instances)==set(range(count));grid={r[0]:r[1:] for r in data['cycle']};assert set(grid)==set(range(1,cycles+1));program,config=setup(key);forward,latency,writefirst,inid,unified=config;interval={i:[[] for _ in range(5)] for i in instances}
 for c,stages in grid.items():
  live=[i for i in stages if i>=0];assert len(live)==len(set(live))
  for s,i in enumerate(stages):
   if i>=0:assert i in interval;interval[i][s].append(c)
 for i,(pc,flush,resolve) in instances.items():
  assert 0<=pc<len(program);stage=interval[i];assert stage[0]
  for s,times in enumerate(stage):
   if times:assert times==list(range(times[0],times[-1]+1))
   if s and times:assert stage[s-1] and times[0]==stage[s-1][-1]+1
  if flush:assert not stage[2] and max(t for values in stage for t in values)==flush
  else:
   assert all(stage) and len(stage[4])==1;assert len(stage[3])==(latency if program[pc][0]==1 else 1)
   if len(stage[2])>1:
    for c in stage[2][:-1]:assert grid[c][3]>=0 and c<interval[grid[c][3]][3][-1]
  op,dst,left,right,target,taken=program[pc]
  if flush:continue
  for reg in {left,right}-{0}:
   writers=[j for j,(q,f,_) in instances.items() if j<i and not f and program[q][0]!=2 and program[q][1]==reg]
   if not writers:continue
   producer=max(writers);previous=interval[producer];load=program[instances[producer][0]][0]==1
   if op==2 and inid:assert resolve>=previous[4][0]+(not writefirst)
   elif forward:assert stage[2][0]>=(previous[3][-1] if load else previous[2][0])+1
   else:assert stage[1][-1]>=previous[4][0]+(not writefirst)
  if op==2:
   assert resolve in stage[1 if inid else 2]
   if not inid:assert resolve==stage[2][0]
   if taken:
    younger=[j for j in grid[resolve][:1 if inid else 2] if j>=0];assert all(instances[j][1]==resolve for j in younger)
    new=[j for j in instances if interval[j][0][0]>resolve];assert new and instances[min(new)][0]==target
    if not unified:assert interval[min(new)][0][0]==resolve+1
 for c,stages in grid.items():
  if unified and stages[3]>=0 and program[instances[stages[3]][0]][0]==1:
   assert stages[0]<0 or interval[stages[0]][0][0]<c
 retired=data['retire'];assert len(retired)==retired_count and [r[0] for r in retired]==list(range(retired_count));assert sum(bool(r[1]) for r in instances.values())==flushed_count
 assert [r[1] for r in retired]==[i for i in sorted(instances) if not instances[i][1]]
 for _,i,c in retired:assert interval[i][4]==[c]
 assert retired[-1][2]==cycles and retired_count==(6 if len(program)==7 else 4)
 expected={100:(13,2),101:(14,0),102:(9,0),103:(17,0),104:(14,2),105:(14,1),200:(9,0)}
 if key in expected:assert (cycles,flushed_count)==expected[key]
 if key==100:
  table={0:[[1],[2],[3],[4],[5]],1:[[2],[3],[4],[5],[6]],2:[[3],[4],[5],[6],[7]],3:[[4],[5,6],[7],[8],[9]],4:[[5,6],[7],[8],[9],[10]],5:[[7],[8],[],[],[]],6:[[8],[],[],[],[]],7:[[9],[10],[11],[12],[13]]};assert interval==table
 return dict(key=key,cycles=cycles,instances=count,retired=retired_count,flushed=flushed_count)
def ooo(rows):
 cases=[[(1,1,0,0),(0,2,1,0),(0,3,0,0),(0,4,2,3)],[(1,1,0,0),(0,2,1,0),(0,1,0,0),(0,3,1,0)],[(1,0,0,0),(0,1,0,0),(1,2,0,0),(0,3,1,2)],[(0,1,6,7),(1,2,1,0),(0,3,1,2),(0,1,3,7)]];initial=[0]*6+[250,10];memory=[(37*i+5)%256 for i in range(16)];packets=[]
 for kind,program in enumerate(cases):
  for latency in range(1,5):
   actual=[r[2:] for r in rows['ooo'] if r[:2]==[kind,latency]];assert len(actual)==4;records={r[0]:r[1:] for r in actual};tags=[];latest={};values=[]
   for i,(op,dst,left,right) in enumerate(program):
    lt=latest.get(left,-1);rt=latest.get(right,-1);tags.append((lt,rt));a=initial[left] if lt<0 else values[lt];b=initial[right] if rt<0 else values[rt];values.append(memory[(a+b)%16] if op else (a+b)%256)
    if dst:latest[dst]=i
   for i,(issue,complete,retire,value,lt,rt) in records.items():
    assert (lt,rt)==tags[i] and value==values[i];assert complete==issue+(latency if program[i][0] else 1)-1
    for t in [lt,rt]:assert t<0 or issue>records[t][1]
    assert retire==max(complete+1,records[i-1][2]+1 if i else 1)
   assert len({r[0] for r in records.values()})==4
   loads=[(r[0],r[1]) for i,r in records.items() if program[i][0]==1]
   for a in range(len(loads)):
    for b in range(a):assert max(loads[a][0],loads[b][0])>min(loads[a][1],loads[b][1])
   for cycle in range(1,max(r[0] for r in records.values())+1):
    issued=[i for i,r in records.items() if r[0]==cycle];ready=[]
    for i,r in records.items():
     if r[0]<cycle:continue
     if any(t>=0 and records[t][1]>=cycle for t in tags[i]):continue
     if program[i][0]==1 and any(other!=i and records[other][0]<cycle<=records[other][1] for other in records if program[other][0]==1):continue
     ready.append(i)
    assert issued==([min(ready)] if ready else [])
   final=initial[:]
   for i,(_,dst,_,_) in enumerate(program):
    if dst:final[dst]=values[i]
   assert [r[2:] for r in rows['ooo_final'] if r[:2]==[kind,latency]]==[[records[3][2],*final]]
   packets.append([kind,latency,records[3][2]])
 return packets
def main():
 path=pathlib.Path(sys.argv[1]);case=sys.argv[2];rows=collections.defaultdict(list)
 if case=='distinctions':
  d=json.loads(path.read_text());assert d['c_data_race']['classification']=='undefined behavior' and len(d['c_data_race']['premises'])==4 and d['c_data_race']['execution']=='analysis-only, not run';assert d['out_of_order']['physical_observation']=='not-run' and len(d['out_of_order']['changed_assumptions'])==4;print(json.dumps(dict(status='pass',packets=5,physical=False)));return
 if case=='cost':
  entries={line.split('\t')[0].split(':')[-1]:(int(line.split('\t')[-2]),line.split('\t')[-1]) for line in path.read_text().splitlines()};assert all(n in entries for n in ['ap_schedule','ap_ooo']);assert all(0<entries[n][0]<8192 and entries[n][1] in ['static','dynamic,bounded'] for n in ['ap_schedule','ap_ooo']);print(json.dumps(dict(status='pass',entries=entries,limits='Compiler-estimated bounded native function usage, including bounded body adjustments where reported; not total call-stack, target stack or physical high-water observation.')));return
 for line in path.read_text().splitlines():
  if line.startswith('DATA '):_,kind,*values=line.split();rows[kind].append(list(map(int,values)))
 if case in ['schedule','matrix']:
  keys=sorted(r[0] for r in rows['head']);assert keys==([100,101,102,103,104,105] if case=='schedule' else [*range(32),200]);results=[schedule(rows,key) for key in keys];result=dict(status='pass',packets=len(results),schedules=results)
 elif case=='boundaries':assert rows=={'boundaries':[[6,0]]};result=dict(status='pass',packets=1)
 elif case=='ooo':results=ooo(rows);result=dict(status='pass',packets=len(results),workloads=results)
 else:raise AssertionError(case)
 print(json.dumps(result))
if __name__=='__main__':main()
