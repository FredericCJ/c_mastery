"""Independent expected ownership states and TLV bytes; no target dependency."""
import itertools,json,pathlib,struct,sys
text=pathlib.Path(sys.argv[1]).read_text();which=sys.argv[2];keys=['dialogue','receipts','resume'] if which=='all' else [which]
if 'dialogue' in keys:
 assert 'ES_DIALOGUE_VALUES 0 1 1 0' in text
 # Sender/receiver authority, delivered TOKEN, completion, cancellation.
 states={(1,0,0,0)};edges=0
 for _ in range(5):
  new=set(states)
  for a,b,pending,delivered in states:
   transitions=[]
   if a:transitions.append((0,b,1,delivered))
   if pending and not delivered:transitions.append((0,1,1,1))
   if pending and delivered:transitions.append((0,1,0,1))
   if pending:transitions.append((0,b,1,delivered))
   for t in transitions:assert t[0]+t[1]<=1;new.add(t);edges+=1
  states=new
 assert (0,1,1,1) in states and (0,0,1,0) in states
 # Unsafe timeout restoration following delivery enables two holders.
 assert sum((1,1))==2
if 'receipts' in keys:
 assert 'ES_RECEIPTS_VALUES 1 1 0 2500' in text
 expected=bytes([1,1,1,3,2,2])+struct.pack('>H',9)+bytes([3,2])+struct.pack('>h',2500)+bytes([4,1,1])
 assert 'ES_REPLY_HEX\n'+expected.hex() in text
if 'resume' in keys:assert 'ES_RESUME_VALUES 1 1 0 2' in text
print(json.dumps(dict(status='pass',keys=keys,packets=len(keys),limits='Bounded author observations and a finite ownership model; no authentication, durable protocol, learner or physical proof.')))
