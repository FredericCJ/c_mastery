"""Independent list/permutation oracle; never calls the C search recursively."""
import argparse,hashlib,itertools,json,subprocess
from pathlib import Path

def allowed(ops,spec):
 complete={i for i,o in enumerate(ops) if not o[8]}
 pending=[i for i,o in enumerate(ops) if o[8]]
 for bits in range(1<<len(pending)):
  chosen=complete|{i for j,i in enumerate(pending) if bits>>j&1}
  for order in itertools.permutations(chosen):
   queue=[];done=set();good=True
   for i in order:
    o=ops[i]
    if any(j not in done and not p[8] and p[3]<o[2] for j,p in enumerate(ops) if j in chosen):good=False;break
    done.add(i)
    if o[5]==0:
     result=1 if len(queue)==4 else 0;value=o[6]
     if result==0:queue.append(value)
    elif queue:result=0;value=queue.pop(-1 if spec else 0)
    else:result=2;value=o[6]
    if not o[8] and (o[7]!=result or (o[5]==1 and result==0 and o[6]!=value)):good=False;break
   if good:return True
 return False

def replay(ops,spec,words):
 status,visited,count,mask=map(int,words[:4]);assert status==0 and visited>0
 order=[];queue=[];complete={i for i,o in enumerate(ops) if not o[8]}
 chosen=complete|{i for i,o in enumerate(ops) if o[8] and mask>>i&1}
 assert len(words[4:])==count==len(chosen)
 for token in words[4:]:
  i,result,value=map(int,token.split(','));assert i in chosen and i not in order
  assert all(p[8] or p[3]>=ops[i][2] or j in order for j,p in enumerate(ops) if j in chosen)
  order.append(i);o=ops[i]
  if o[5]==0:
   expected=1 if len(queue)==4 else 0;v=o[6]
   if expected==0:queue.append(v)
  elif queue:expected=0;v=queue.pop(-1 if spec else 0)
  else:expected=2;v=o[6]
  assert result==expected and value==v
  if not o[8]:assert result==o[7] and (o[5]==0 or result!=0 or value==o[6])

def main():
 p=argparse.ArgumentParser();p.add_argument('--exe',required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--protocol',type=Path,action='append',default=[]);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=False)
 schedules=[]
 for order in itertools.permutations(range(6)):
  pos={event:i for i,event in enumerate(order)}
  if all(pos[2*i]<pos[2*i+1] for i in range(3)):schedules.append(pos)
 assert len(schedules)==90
 actions=[(0,7,0),(0,8,0),(1,7,0),(1,8,0),(1,0,2)]
 cases=[];inputs=[]
 for action in itertools.product(actions,repeat=3):
  for pos in schedules:
   for spec in (0,1):
    for pending in (0,1,3):
     ops=[[pos[2*i],pos[2*i+1],pos[2*i],pos[2*i+1],i,*action[i],int(bool(pending>>i&1))] for i in range(3)]
     expected=allowed(ops,spec);cases.append((ops,spec,expected));inputs.append('3 4 %d 1 1 100000\n'%spec+'\n'.join(' '.join(map(str,o)) for o in ops)+'\n')
 corpus=''.join(inputs);(a.output/'input.txt').write_text(corpus)
 r=subprocess.run([a.exe,'driver'],input=corpus,text=True,capture_output=True,timeout=120)
 (a.output/'actual.txt').write_text(r.stdout+r.stderr);assert r.returncode==0 and not r.stderr
 lines=r.stdout.splitlines();assert len(lines)==len(cases)==67500
 valid=0;visited=0;maximum=0
 for (ops,spec,expected),line in zip(cases,lines):
  words=line.split();status,steps=map(int,words[:2]);assert status==(0 if expected else 1),(ops,spec,expected,line)
  visited+=steps;maximum=max(maximum,steps)
  if expected:valid+=1;replay(ops,spec,words)
 protocol=[]
 for path in a.protocol:
  for line in path.read_text().splitlines():
   if line.startswith('CP_HISTORY '):
    fields=line.split();ops=[list(map(int,v.split(','))) for v in fields[2:]];assert allowed(ops,0);protocol.append({'path':str(path),'case':int(fields[1]),'calls':len(ops)})
 result={'passed':True,'cases':len(cases),'specifications':['FIFO','STACK'],'pending_masks':[0,1,3],'event_schedules':90,'accepted':valid,'rejected':len(cases)-valid,'visited':visited,'maximum_search_nodes':maximum,'actual_protocol_histories':protocol,'input_sha256':hashlib.sha256(corpus.encode()).hexdigest(),'limits':['Finite complete and pending prefixes with distinct participants; invalid admission and exhausted budget have separate C tests.','A witness is independently replayed; finite acceptance is not a proof of all queue executions.']}
 (a.output/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
if __name__=='__main__':main()
