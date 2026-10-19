"""List/Fraction and explicit original wire authority, independent of C layouts."""
from fractions import Fraction
from itertools import product,permutations
import hashlib,io,subprocess
GENERATOR='lcg32-high16-v1'
def seeded(seed,n=64):
 state=seed;result=[]
 for _ in range(n):state=(state*1664525+1013904223)&0xffffffff;result.append((state>>16)%4)
 return result
def words(radix,n):
 for word in range(radix**n):
  code=word;ops=[]
  for _ in range(n):ops.append(code%radix);code//=radix
  yield ops
def check(text):
 lines=iter(io.StringIO(text));histories=steps=0
 def expect(value):
  line=next(lines,None)
  if line is None or line.split()!=list(map(str,value)):raise ValueError('Independent model mismatch: '+str(value))
 def sequence(ops,family,identity):
  nonlocal histories,steps
  expect(['BEGIN',family,identity,len(ops)]);values=[];histories+=1;accepted=removed=discarded=0
  for index,op in enumerate(ops):
   out=99
   if op<2:
    status=1 if len(values)==2 else 0
    if not status:values.append(op);accepted+=1
   elif op==2:
    status=0 if values else 2
    if not status:out=values.pop(0);removed+=1
   else:status=0;discarded+=len(values);values=[]
   if accepted!=removed+discarded+len(values):raise ValueError('Oracle conservation with clear')
   expect(['Q',index,op,status,out,len(values),1]);steps+=1
 identity=0
 for radix in (3,4):
  for n in range(7):
   for ops in words(radix,n):sequence(ops,radix,identity);identity+=1
 for seed in range(1,33):sequence(seeded(seed),5,seed)
 decoders=0
 for count in range(6):
  for n in range(13):
   for opcode in range(2):
    data=bytes([opcode,count,*[(i*17+3)&255 for i in range(2,12)]])[:n]
    status=1 if n<2 else 2 if opcode!=1 else 3 if count>4 else 1 if n<2+2*count else 4 if n>2+2*count else 0
    values=[int.from_bytes(data[i:i+2],'big') for i in range(2,n,2)] if not status else []
    fields=['D',count,n,opcode,status,status,count if not status else 0,count if not status else 0]
    for i in range(4):fields += [values[i] if i<len(values) else 0]*2
    expect(fields);decoders+=1
 filters=0
 for values in product(range(-3,4),repeat=3):
  exact=[sum(Fraction(1,2)*values[i-j] for j in range(2) if i>=j) for i in range(3)]
  for split in range(4):
   for saturated in range(2):
    output=[max(Fraction(-2),min(Fraction(2),v)) if saturated else v for v in exact]
    expect(['F',*values,split,saturated,0,0,*[int(2*v) for v in output]]);filters+=1
 if next(lines,None) is not None:raise ValueError('Unexpected extra observation')
 return {'histories':histories,'queue_operations':steps,'decoder_cases':decoders,'FIR_partitions':filters,'generator':GENERATOR,'seeds':list(range(1,33)),'scope':'All0..6 words on3/4 operations,32 retained64-step seeds, explicit wire/Fraction domains only'}
def minimize(original,predicate):
 """1-deletion-minimal for the same semantic class, never merely any failure."""
 wanted=predicate(original)
 if wanted=='none':raise ValueError('No target failure')
 current=original;attempts=[];again=True
 while again:
  again=False
  for index in range(len(current)):
   candidate=current[:index]+current[index+1:];observed=predicate(candidate);attempts.append({'candidate':candidate,'observed':observed})
   if observed==wanted:current=candidate;again=True;break
 return {'original':original,'minimized':current,'property':wanted,'attempts':attempts,'scope':'Deletion-minimal under the recorded predicate and generator; not globally minimum'}
def investigate(trace,schedule_bad,schedule_good,correlated,run,identities=None):
 def trace_predicate(value):
  out,code=run([trace,value])
  if code or not out.startswith('RESULT '):return 'invalid-execution'
  return out.split()[1]
 original=''.join(map(str,seeded(7)));reduction=minimize(original,trace_predicate)
 if reduction['property']!='storage-preservation' or len(reduction['minimized'])!=3:raise ValueError('Wrong reduced queue failure')
 schedules=[]
 def schedule_predicate(value):
  out,code=run([schedule_bad,value])
  if code==2:return 'invalid-schedule'
  if code or not out.startswith('SCHEDULE '):raise ValueError('Wrong scheduler observation')
  return out.split()[1]
 for order in permutations('aAbB'):
  s=''.join(order)
  if s.index('a')>s.index('A') or s.index('b')>s.index('B'):continue
  snapshots={};value=0
  for token in s:
   if token.islower():snapshots[token]=value
   else:value=snapshots[token.lower()]+1
  out,code=run([schedule_bad,s]);good,gcode=run([schedule_good,s])
  if code or out.strip()!=f'SCHEDULE {"none" if value==2 else "lost-update"} {value}' or gcode or good.strip()!='SCHEDULE none 2':raise ValueError('Wrong actual controlled-thread schedule')
  schedules.append({'schedule':s,'flawed_result':value,'repaired_result':2})
 reduced=minimize('.a.b..A.B.',schedule_predicate)
 if reduced['minimized']!='abAB' or reduced['property']!='lost-update':raise ValueError('Lost schedule/class during reduction')
 # Two implementations may agree under a shared wrong authority.
 wire=bytes([1,2,1,2,254,220]);wrong=[int.from_bytes(wire[i:i+2],'little') for i in (2,4)];right=[258,65244]
 out,code=run([correlated])
 if wrong==right or code or out.strip()!=f'CORRELATED {wrong[0]} {wrong[1]} {wrong[0]} {wrong[1]} 0 0':raise ValueError('Asymmetric vector/actual correlated implementations failed')
 length_counterexample={'wire_hex':wire.hex(),'normative_extent':2+2*wire[1],'both_wrong_extent':2*wire[1],'both_wrong_reject':True,'correct_result':right}
 return {'queue':reduction,'generator':GENERATOR,'seed':7,'complete_trace':original,'thread_schedule':reduced,'schedule_matrix':schedules,'binary_sha256':identities if identities is not None else {str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in (trace,schedule_bad,schedule_good,correlated)},'shared_wrong_endian':{'same_helper_twice':wrong,'correct_external_vector':right,'shared_dependency':'actual C cursor/fold both call wrong_endian in correlated.c'},'shared_wrong_length':length_counterexample,'limitations':['Controlled pthread schedule, not arbitrary stress or all-memory-model proof.','Generator version plus complete trace and binary identities retained; seed alone is insufficient.','Original wire contract is explicit external authority relative to both decoders, not a network standard.']}
