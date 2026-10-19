"""Independent finite FIFO history oracle: explicit permutations, not C DFS."""
import ctypes,itertools,json,sys
class Op(ctypes.Structure):_fields_=[(n,ctypes.c_uint) for n in ['invocation','response','thread','kind','value','result']]
class Witness(ctypes.Structure):_fields_=[('length',ctypes.c_uint),('operation',ctypes.c_uint*6),('completed_pending_mask',ctypes.c_uint),('result',ctypes.c_uint*6),('value',ctypes.c_uint*6)]
def oracle(ops,extend):
 pending={i for i,o in enumerate(ops) if o.response==0}
 if pending and not extend:return 5
 for bits in range(1<<len(ops)):
  selected=[i for i in range(len(ops)) if bits&(1<<i)]
  if any(i not in selected for i in range(len(ops)) if i not in pending):continue
  for order in itertools.permutations(selected):
   positions={j:i for i,j in enumerate(order)}
   if any(a.response and a.response<b.invocation and positions[i]>positions[j] for i,a in enumerate(ops) for j,b in enumerate(ops) if i in positions and j in positions):continue
   queue=[];good=True
   for i in order:
    o=ops[i]
    if o.kind==0:
     result=2 if len(queue)==3 else 0;value=0
     if result==0:queue.append(o.value)
    else:
     result=0 if queue else 1;value=queue.pop(0) if queue else 0
    if o.response and (o.result!=result or (o.kind==1 and result==0 and o.value!=value)):good=False;break
   if good:return 0
 return 4
def main():
 lib=ctypes.CDLL(sys.argv[1]);f=lib.tp_history;f.argtypes=[ctypes.POINTER(Op),ctypes.c_size_t,ctypes.c_uint,ctypes.POINTER(Witness)];f.restype=ctypes.c_int
 total=0;accepted=0
 # Four distinct threads, each one operation. All8 invocation/response events
 # are permuted; reject malformed intervals before this valid-history corpus.
 for times in itertools.permutations(range(1,9)):
  if any(times[2*i]>=times[2*i+1] for i in range(4)):continue
  for a,b in itertools.product(range(3),repeat=2):
   rows=[Op(times[0],times[1],0,0,1,0),Op(times[2],times[3],1,0,2,0),Op(times[4],times[5],2,1,a,1 if a==0 else 0),Op(times[6],times[7],3,1,b,1 if b==0 else 0)]
   expected=oracle(rows,False);out=Witness();actual=f((Op*4)(*rows),4,0,ctypes.byref(out));assert actual==expected,(times,a,b,actual,expected);total+=1;accepted+=actual==0
 # Pending producer before/after the completed consumer, including the
 # observable case that cannot be explained by simply dropping the producer.
 pending_count=0
 for invocation in [1,4]:
  for value in [0,7,8]:
   rows=[Op(invocation,0,0,0,7,0),Op(2,3,1,1,value,1 if value==0 else 0)]
   for policy in [0,1]:
    out=Witness();actual=f((Op*2)(*rows),2,policy,ctypes.byref(out));assert actual==oracle(rows,policy), (invocation,value,policy,actual);pending_count+=1
 print(json.dumps(dict(status='pass',complete_histories=total,linearizable=accepted,pending_cases=pending_count,limits='Four complete operations,capacity3,values1/2 plus explicit pending-producer cases; no liveness or general execution proof.')))
if __name__=='__main__':main()
