"""Separate exhaustive permutation specification for actual four-call logs."""
import itertools,json,pathlib,sys
def allowed(ops):
 for order in itertools.permutations(range(4)):
  prefix=[];fifo=[];good=True
  for i in order:
   invoke,respond,participant,kind,value,result=ops[i]
   if any(o[1]<invoke and j not in prefix for j,o in enumerate(ops)):good=False;break
   prefix.append(i)
   if kind==0:
    if result!=0 or len(fifo)==4:good=False;break
    fifo.append(value)
   elif fifo:
    if result!=0 or fifo.pop(0)!=value:good=False;break
   elif result!=2:good=False;break
  if good:return list(order)
 return None
def main():
 p=pathlib.Path(sys.argv[1]);cases=[]
 for line in p.read_text(encoding='utf8').splitlines():
  if not line.startswith('TRACE '):continue
  words=line.split();ops=[list(map(int,x.split(','))) for x in words[2:]]
  assert len(ops)==4 and sorted(x for o in ops for x in o[:2])==list(range(8))
  witness=allowed(ops);assert witness is not None
  cases.append(dict(run=int(words[1]),order=witness))
 assert len(cases)==64 and [q['run'] for q in cases]==list(range(64))
 assert allowed([[0,1,0,0,11,0],[2,3,1,0,22,0],[4,5,2,1,22,0],[6,7,3,1,11,0]]) is None
 print(json.dumps(dict(status='pass',histories=64,actual_calls=256,witnesses=cases,reversed_fifo_rejected=True)))
if __name__=='__main__':main()
