"""Independent integer-interval, source graph and service-bound certificates."""
import collections,json,pathlib,sys
def regions(part):return [(0x10000000,65536,1,1),(0x20000000,114688,1,54),(0x2001c000,16384,1,54),(0x20020000,65536,1,54) if part==0 else (0,0,0,0),(0x40000000,32768,1,42),(0x40010000,32768,1,34)]
def main():
 path=pathlib.Path(sys.argv[1]);case=sys.argv[2];rows=collections.defaultdict(list)
 for line in path.read_text().splitlines():
  if line.startswith('DATA '):_,kind,*v=line.split();rows[kind].append(list(map(int,v)))
 if case=='topology':
  assert len(rows['map'])==12
  for part,reg,*rest in rows['map']:assert rest==[*regions(part)[reg],196608 if part==0 else 131072]
  count=12
 elif case=='placement':
  assert len(rows['extent'])==960
  for part,reg,master,address,size,status,offset in rows['extent']:
   base,capacity,present,mask=regions(part)[reg];expected=1 if not present else 2 if not mask&(1<<master) else 3 if not (base<=address and address+size<=base+capacity) else 0;assert status==expected;assert offset==(address-base if not status else 0xfeed)
  assert rows['linked']==[[0x20000010,0x20020000,0x20000000,0x10000000]];count=961
 elif case=='lease':
  assert rows['lease']==[[1,1,1,2,16,42,77]];assert rows['lease_state']==[[0,0,0,0,0,0,0,0,42,0,0,0],[1,1,1,1,1,16,0,1,42,0,1,0],[2,1,1,1,0,0,32,0,42,0,1,0],[3,1,1,2,0,0,32,1,42,0,1,0],[4,3,1,1,0,0,32,1,42,42,1,1],[5,0,1,1,0,0,32,1,77,42,1,2]];count=6
 elif case=='arbitration':
  for mask in range(1,8):
   for mode in [0,1]:
    members=[i for i in range(3) if mask&(1<<i)];cost=[2,8,3] if mode else [1,1,1];grants=[r for r in rows['grant'] if r[:2]==[mask,mode]];assert len(grants)==len(members)*3;end=0;last={i:0 for i in members}
    for ordinal,row in enumerate(grants):
     _,_,index,master,start,finish,wait,bound=row;assert index==ordinal and master==members[ordinal%len(members)] and start==end and finish-start==cost[master] and wait==start-last[master] and bound==(24 if mode else 2) and wait<=bound;end=finish;last[master]=finish
    # The exact recurrent wait is the sum of other admitted burst costs.
    for row in grants[len(members):]:assert row[6]==sum(cost[i] for i in members if i!=row[3])
  count=14
 elif case=='bridge':
  assert len(rows['bridge'])==15
  for width,value,status,word,allowed in rows['bridge']:
   invalid=value>=1<<width;assert status==int(invalid) and allowed==int(width in [16,32]);expected=0xfeed if invalid else sum(value<<(i*width) for i in range(32//width));assert word==expected
  count=15
 else:raise AssertionError(case)
 print(json.dumps(dict(status='pass',packets=count,physical=False)))
if __name__=='__main__':main()
