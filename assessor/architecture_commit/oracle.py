"""Transition oracle uses immutable tuples/dictionaries, not C control logic."""
import json,pathlib,sys
def main():
 p=pathlib.Path(sys.argv[1]);case=sys.argv[2]
 if case=='evidence':
  d=json.loads(p.read_text());assert len(d['packets'])==6 and d['changed_investigation']['physical_observation']=='not-run';assert 'NOT a captured board trace' in d['packets'][-1]['origin'];assert len(d['changed_investigation']['reinspect'])==5;print(json.dumps(dict(status='pass',packets=6,physical=False)));return
 rows=[[int(v) for v in line.split()[2:]] for line in p.read_text().splitlines() if line.startswith('DATA ')];expected=[]
 if case=='baseline':
  program=[('li',1,3),('li',2,1),('sub',1,2),('bnz',1,2),('st',1,5),('ld',3,5),('halt',0,0)];regs=[0]*4;memory=[0]*16;pc=0;halt=0
  while not halt:
   before=pc;op,a,b=program[pc];rw=mw=taken=alu=0;pc+=1
   if op=='li':regs[a]=b;rw=1<<a;alu=b
   elif op=='sub':regs[a]=(regs[a]-regs[b])%256;rw=1<<a;alu=regs[a]
   elif op=='bnz':taken=int(regs[a]!=0);pc=b if taken else pc
   elif op=='st':memory[b]=regs[a];mw=1;alu=regs[a]
   elif op=='ld':regs[a]=memory[b];rw=1<<a;alu=regs[a]
   else:halt=1;pc=before
   i=len(expected);expected.append([i,before,pc,rw,mw,taken,alu,*regs,i+1,halt])
  expected.append([2,2,2,3,1])
 elif case=='clocked':
  # Explicit hand-derived control table, independently represented by rows.
  controls=[(0,1,2,0,0,3),(1,2,4,0,0,1),(2,3,2,0,0,2),(3,2,0,0,1,0),(2,3,2,0,0,1),(3,2,0,0,1,0),(2,3,2,0,0,0),(3,4,0,0,0,0),(4,5,0,1,0,0),(5,6,8,0,0,0),(6,6,0,0,0,0)]
  operands=[(0,3),(0,1),(3,1),(2,0),(2,1),(1,0),(1,1),(0,0),(0,0),(0,0),(0,0)];zero=0;cycle=0
  for i,control in enumerate(controls):
   left,right=operands[i]
   if i in [2,4,6]:zero=int(control[-1]==0)
   if i==9:
    expected.append([cycle,1,0,left,right,*control,5,9,0,zero,cycle+1]);cycle+=1
   expected.append([cycle,0,1,left,right,*control,control[1],i+1,int(i==10),zero,cycle+1]);cycle+=1
 elif case=='input':expected=[[4,0,4,4]]
 elif case=='status':expected=[[a,b,(a-b)%256,int(a==b),int(a==b)] for a in range(256) for b in range(256) if a==b or a==0 or b==0]+[[65536,256]]
 elif case=='load':expected=[[stalls,cancel,99 if cancel else 42,0 if cancel else 1,stalls+2,1] for stalls in range(4) for cancel in range(2)]
 elif case=='speculation':
  for condition in range(2):
   for prediction in [1,2]:
    for op1 in range(3):
     for op2 in range(3):
      selected=2 if condition else 1;kind=op2 if condition else op1;status=6 if prediction!=selected else 8 if kind==2 else 0;reg,m5,m6=9,55,66;commits=ticks=1;pc=selected
      if status==0:
       commits=ticks=2;pc+=1
       if kind==0:reg=88 if condition else 77
       elif condition:m6=9
       else:m5=9
      expected.append([condition,prediction,op1,op2,status,pc,reg,m5,m6,commits,ticks])
 else:raise AssertionError(case)
 assert rows==expected,(case,rows[:3],expected[:3]);print(json.dumps(dict(status='pass',packets=len(rows))))
if __name__=='__main__':main()
