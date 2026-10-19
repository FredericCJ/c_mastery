"""Independent Python-list model and deletion minimizer; no assessor imports."""
import argparse,json,random,subprocess

def expected(history):
 values=[];cap=0;rows=[]
 for op,at,value,fail in history:
  status=0
  if op=='z':values=[];cap=0
  elif op=='i':
   if at>len(values):status=1
   elif len(values)==cap and fail:status=4
   else:
    if len(values)==cap:cap=2*cap if cap else 1
    values.insert(at,value)
  elif op=='e':
   if at>=len(values):status=1
   else:values.pop(at)
  elif op=='r':
   if at>cap:
    if fail:status=4
    else:cap=at
  elif op=='s':
   if len(values)<cap:
    if values and fail:status=4
    else:cap=len(values)
  rows.append([status,len(values),cap,int(cap!=0),*values])
 return rows
def execute(binary,history):
 text=''.join('%s %d %d %d\n'%row for row in history)
 result=subprocess.run([binary],input=text,text=True,capture_output=True,timeout=15)
 if result.returncode:raise RuntimeError(result.stderr)
 return [list(map(int,row.split())) for row in result.stdout.splitlines()]
def mismatch(binary,history):return execute(binary,history)!=expected(history)
def main():
 p=argparse.ArgumentParser();p.add_argument('binary');p.add_argument('--mutant',action='store_true');a=p.parse_args();rng=random.Random(1415)
 history=[]
 for group in range(128):
  history.append(('z',0,0,0));size=0
  for step in range(64):
   op=rng.choice('iiiers');at=rng.randrange(0,20) if op=='r' else rng.randrange(0,size+3)
   history.append((op,at,rng.randrange(-3,4),rng.randrange(5)==0))
   size=expected(history[-(step+2):])[-1][1]
 # Ensure a small deletion witness is in the generated corpus independently of PRNG evolution.
 history += [('z',0,0,0),('i',0,1,0),('i',1,2,0),('e',0,0,0)]
 bad=mismatch(a.binary,history)
 if not a.mutant:
  assert not bad,'CONTAINERS_MODEL_ASSERT: implementation disagrees with independent list'
  print(json.dumps({'passed':True,'histories':129,'operations':len(history),'seed':1415}));return
 assert bad,'intended deletion mutant survived'
 witness=[('z',0,0,0),('i',0,1,0),('i',1,2,0),('e',0,0,0)]
 assert mismatch(a.binary,witness)
 changed=True
 while changed:
  changed=False
  for i in range(len(witness)):
   candidate=witness[:i]+witness[i+1:]
   if mismatch(a.binary,candidate):witness=candidate;changed=True;break
 assert all(not mismatch(a.binary,witness[:i]+witness[i+1:]) for i in range(len(witness)))
 print(json.dumps({'passed':True,'intended_mutant_rejected':True,'deletion_minimal_witness':witness,'scope':'one-operation deletion minimal; not global shortest'}))
if __name__=='__main__':main()
