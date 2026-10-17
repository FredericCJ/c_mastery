"""Mathematical expectations; does not import the reference C implementation."""
from pathlib import Path
import json,itertools
root=Path(__file__).resolve().parents[3]
c=json.loads((root/'fixtures/modern_bridge/cases.json').read_text(encoding='utf8'))['cases']
cursor=0;payload=0;offsets=[]
for n,a in c['region']['requests']:
 offset=next(x for x in range(cursor,33) if x%a==0)
 offsets.append(offset);cursor=offset+n;payload+=n
assert c['region']['expected']==dict(offsets=offsets,cursor=cursor,payload=payload,high=cursor,cleanup_order=list(reversed(range(1,5))),old_span_after_reset='STALE')
assert c['once']['permanent'][-1]=='FAILED' and c['once']['retry'][-1]=='OK'
assert c['once']['expected_values']==[17,34]
# Logical identities distinguish reuse independently of physical address.
old=(0,1);current=(0,2)
assert c['hazard']['expected']==dict(collect=1,protect_old='RETRY' if old!=current else 'OK',output_unchanged=old!=current,held_reader1_collect=0)
assert c['epoch']['expected']==['OK','OK','OK','BUSY','OK','OK',1] and c['epoch']['distance1_collect']==0
alignment_cases=0
for cursor,n,a in itertools.product(range(33),range(1,36),[1,2,4,8]):
 candidates=[x for x in range(cursor,33) if x%a==0 and x+n<=32]
 padding=(-cursor)%a
 admitted=padding<=32-cursor and n<=32-cursor-padding
 assert admitted==bool(candidates)
 if admitted:assert cursor+padding==candidates[0]
 alignment_cases+=1
# All total orders preserving reader's publish < validate < access and
# writer's remove < scan. If validation still sees source, scan cannot miss
# an uncleared hazard; source never republishes the same logical token.
valid_orders=0
for order in itertools.permutations('PVARS'):
 pos={x:order.index(x) for x in order}
 if not(pos['P']<pos['V']<pos['A'] and pos['R']<pos['S']):continue
 valid_orders+=1
 validation_ok=pos['V']<pos['R'];scan_misses=pos['S']<pos['P']
 assert not(validation_ok and scan_misses)
assert valid_orders==10
print(json.dumps(dict(checked=4,failed=0,independent_alignment_cases=alignment_cases,hazard_SC_orders=valid_orders,scope='finite mathematical expectations; no weak-memory or machine-execution proof')))
