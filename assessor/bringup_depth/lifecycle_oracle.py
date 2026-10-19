"""Independent event ledger: outstanding callback identities and explicit deadline."""
import sys,json
rows=0
with open(sys.argv[1]) as f:
 for n in range(4):
  for limit in range(1,4):
   for sequence in range(243):
    pending=set(range(n));deadline=limit;ack=False;expired=False;transferred=False;x=sequence
    for i in range(5):
     event=x%3;x//=3
     if transferred or (event==2 and not pending): result=1
     elif expired: result=3
     else:
      if event==2:pending.remove(min(pending))
      ack=ack or event==1
      if ack and not pending:transferred=True;result=0
      else:
       deadline-=1;expired=deadline==0;result=3 if expired else 4
     expected=[n,limit,sequence,i,result,len(pending),deadline,int(ack),int(expired),int(transferred)]
     actual=list(map(int,next(f).split()));assert actual==expected,(actual,expected)
     rows+=1
 assert not f.read().strip()
print(json.dumps(dict(passed=True,rows=rows,scope='2916 bounded event words, old owner persists after timeout; no device timing claim')))
