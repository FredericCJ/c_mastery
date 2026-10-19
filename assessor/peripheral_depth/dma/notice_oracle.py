from pathlib import Path
import json,sys

def check(path):
 states={};seen=set();count=0
 # Rows are operation PUBLISH/TAKE/ACK/QUERY; columns state FREE/READY/READING/ACKED.
 transitions=[[(0,1),(2,1),(2,2),(0,1)],[(2,0),(0,2),(2,2),(2,3)],[(2,0),(2,1),(0,3),(2,3)],[(2,0),(2,1),(2,2),(0,3)]]
 for line in Path(path).read_text().splitlines():
  kind,h,k,op,r,state,seq=line.split();h,k,op,r,state,seq=map(int,[h,k,op,r,state,seq]);assert kind=='N' and (h,k) not in seen;seen.add((h,k));old,oldseq=states.get(h,(0,0));assert op==(h//4**k)%4
  expected,new=transitions[op][old]
  if oldseq==0 and op!=0:expected,new=3,old
  newseq=oldseq+1 if op==0 and expected==0 else oldseq
  assert (r,state,seq)==(expected,new,newseq),(h,k,op,(r,state,seq),(expected,new,newseq));states[h]=(new,newseq);count+=1
 assert count==5120 and len(states)==1024
 return dict(passed=True,rows=count,histories=len(states),limits='Finite serialized mailbox histories, not real cross-core timing.')
if __name__=='__main__':print(json.dumps(check(sys.argv[1])))
