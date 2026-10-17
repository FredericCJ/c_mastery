"""Independent finite mathematical/identity oracles; ctypes is host verification only."""
import argparse,ctypes as C,itertools,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
U8=C.c_uint8;U16=C.c_uint16;U32=C.c_uint32;I32=C.c_int32
class Intern(C.Structure):_fields_=[('bytes',(U8*15)*8),('length',U8*8),('used',U8)]
class Shape(C.Structure):_fields_=[('count',U8),('parent',U8),('added',U8),('keys',U8*4)]
class Shapes(C.Structure):_fields_=[('shape',Shape*16),('used',U8)]
class Object(C.Structure):_fields_=[('shape',U8),('fields',I32*4)]
class Cache(C.Structure):_fields_=[('owner',C.c_void_p),('shape',U8*2),('key',U8*2),('offset',U8*2),('count',U8),('mega',U8),('hits',U32),('misses',U32)]
class Graph(C.Structure):_fields_=[('live',U16),('roots',U16),('edge',(U8*2)*16),('color',U8*16),('active',U8)]
class Cell(C.Structure):_fields_=[('value',I32),('edge',U8*2),('age',U8)]
class Nursery(C.Structure):_fields_=[('young',(Cell*8)*2),('old',Cell*8),('active',U8),('used',U8),('old_used',U8),('roots',U8*4)]
class Cards(C.Structure):_fields_=[('edge',(U8*2)*8),('dirty',U8)]
class Payload(C.Structure):_fields_=[('value',I32),('edge',U8*2)]
class Compact(C.Structure):_fields_=[('cell',Payload*16),('live',U16),('handle',U8*16),('borrows',C.c_uint)]
class Weak(C.Structure):_fields_=[('graph',Graph),('key',U8*4),('value',U8*4),('weak',U8*4),('finalizable',U16),('finalized',U16),('pending',U16)]
class Counts(C.Structure):_fields_=[('live',U16),('refs',U16*16),('edge',(U8*2)*16)]
class Slot(C.Structure):_fields_=[('tag',U8),('base',U8),('offset',U8),('scalar',U32)]
class Frame(C.Structure):_fields_=[('slot',Slot*8),('initialized',U8),('root_map',U8),('at_safepoint',U8)]
def reachable(edges,roots):
 seen=set(roots);todo=list(roots)
 while todo:
  for target in edges[todo.pop()]:
   if target and target not in seen:seen.add(target);todo.append(target)
 return seen
def main():
 p=argparse.ArgumentParser();p.add_argument('--library',type=Path);a=p.parse_args()
 fixtures=sorted((ROOT/'fixtures/object_runtime').glob('PR-*.json'))
 for fixture in fixtures:assert json.loads(fixture.read_text(encoding='utf-8'))['project']==fixture.stem
 if not a.library:print(json.dumps({'fixtures':len(fixtures),'executed':False,'scope':'Metadata only; pass --library for C differential execution.'}));return
 lib=C.CDLL(str(a.library.resolve()));counts={};ref=C.byref
 lib.or_number_bits.argtypes=[C.c_uint64];lib.or_number_bits.restype=C.c_uint64
 lib.or_integer.argtypes=[I32];lib.or_integer.restype=C.c_uint64
 lib.or_integer_get.argtypes=[C.c_uint64,C.POINTER(I32)]
 n=0
 for value in list(range(-4096,4097))+[-2**31,2**31-1]:
  out=I32();word=lib.or_integer(value);assert word==0x7ff9000000000000+(value%2**32);assert lib.or_integer_get(word,ref(out))==0 and out.value==value;n+=1
 for sign,exp,fraction in itertools.product([0,1],[0,1,1023,2046,2047],[0,1,2**51,2**52-1]):
  word=sign*2**63+exp*2**52+fraction;expected=0x7ff8000000000000 if exp==2047 and fraction else word
  assert lib.or_number_bits(word)==expected;n+=1
 counts['value']=n
 n=0
 for seq in itertools.product([b'',b'a',b'b',b'a\x00',b'\x00b'],repeat=4):
  t=Intern();model={}
  for text in seq:
   output=U8(91);buf=(U8*len(text)).from_buffer_copy(text)
   assert lib.or_intern(ref(t),buf,len(text),0,ref(output))==0
   if text not in model:model[text]=len(model)+1
   assert output.value==model[text] and t.used==len(model);n+=1
 counts['intern']=n
 n=0
 for order in itertools.permutations(range(1,5)):
  s=Shapes();lib.or_shapes_init(ref(s));obj=Object();cache=Cache();expected={}
  for key in order:
   expected[key]=key*19;assert lib.or_property_set(ref(s),ref(obj),key,expected[key])==0
   for lookup in range(1,6):
    output=I32(91);status=lib.or_property_get(ref(s),ref(obj),lookup,ref(cache),ref(output));assert status==(0 if lookup in expected else 5)
    assert output.value==expected.get(lookup,91);n+=1
  assert list(s.shape[obj.shape].keys)==list(order)
 counts['shapes_and_cache']=n
 n=0
 for targets in itertools.product(range(4),repeat=3):
  edges={i+1:[targets[i]] for i in range(3)}
  for mask in range(8):
   g=Graph();g.live=7;g.roots=mask
   for i,t in enumerate(targets):g.edge[i][0]=t
   assert lib.or_mark_begin(ref(g))==0
   while True:
    scanned=C.c_uint();result=lib.or_mark_step(ref(g),1,ref(scanned));assert scanned.value<=1
    if result==4:break
    assert result==0
   expected=reachable(edges,[i+1 for i in range(3) if mask>>i&1]);assert {i+1 for i in range(3) if g.color[i]==2}==expected;n+=1
 counts['mark']=n
 n=0
 for targets in itertools.product(range(4),repeat=3):
  for root in range(4):
   h=Nursery();h.used=3;h.roots[0]=root
   for i,t in enumerate(targets):h.young[0][i].value=100+i;h.young[0][i].edge[0]=t
   expected=reachable({i+1:[targets[i]] for i in range(3)},[root] if root else [])
   copied=C.c_uint();assert lib.or_minor(ref(h),8,8,ref(copied))==0 and copied.value==len(expected)
   assert {h.young[h.active][i].value for i in range(h.used)}=={99+i for i in expected}
   for i in range(h.used):
    cell=h.young[h.active][i];original=cell.value-100;target=targets[original]
    assert (h.young[h.active][cell.edge[0]-1].value if cell.edge[0] else 0)==(99+target if target else 0)
   assert lib.or_minor(ref(h),8,8,ref(copied))==0 and h.used==0 and h.old_used==len(expected);n+=1
 counts['nursery']=n
 n=0
 for object_id,field,target in itertools.product(range(8),range(2),range(17)):
  c=Cards();assert lib.or_card_store(ref(c),object_id,field,target)==0
  roots=U16();scanned=C.c_uint();assert lib.or_card_scan(ref(c),ref(roots),ref(scanned))==0
  assert roots.value==(1<<(target-1) if 1<=target<=8 else 0)
  assert scanned.value==(4 if 1<=target<=8 else 0);n+=1
 counts['cards']=n
 n=0
 for mask in range(256):
  h=Compact();h.live=mask
  for i in range(8):
   if mask>>i&1:h.cell[i].value=100+i;h.handle[i]=i+1
  forward=(U8*16)();assert lib.or_compact(ref(h),forward)==0
  for i in range(8):
   if mask>>i&1:assert h.cell[h.handle[i]-1].value==100+i and forward[i]==(mask&((1<<(i+1))-1)).bit_count()
  assert h.live==(1<<mask.bit_count())-1;n+=1
 counts['compact']=n
 n=0
 for keys in itertools.product(range(1,4),repeat=2):
  for values in itertools.product(range(1,4),repeat=2):
   for roots in range(8):
    h=Weak();h.graph.live=7;h.graph.roots=roots
    for i in range(2):h.key[i]=keys[i];h.value[i]=values[i]
    seen={i+1 for i in range(3) if roots>>i&1}
    while True:
     after=seen|{v for k,v in zip(keys,values) if k in seen}
     if after==seen:break
     seen=after
    dead=U16();assert lib.or_weak_collect(ref(h),ref(dead))==0
    assert dead.value==sum(1<<(i-1) for i in {1,2,3}-seen);n+=1
 counts['weak']=n
 n=0
 for targets in itertools.product(range(4),repeat=3):
  edges={i+1:[targets[i]] for i in range(3)}
  for roots in range(8):
   for candidates in range(8):
    h=Counts();h.live=7
    for i,t in enumerate(targets):
     h.edge[i][0]=t
     if t:h.refs[t-1]+=1
    for i in range(3):h.refs[i]+=(roots>>i)&1
    external={i+1 for i in range(3) if roots>>i&1 or not(candidates>>i&1)}
    keep=reachable(edges,external);expected={i+1 for i in range(3) if candidates>>i&1}-keep
    dead=U16();assert lib.or_trial_delete(ref(h),candidates,ref(dead))==0
    assert dead.value==sum(1<<(i-1) for i in expected);n+=1
 counts['trial']=n
 n=0
 for source,target,offset in itertools.product(range(1,17),range(1,17),range(10)):
  f=Frame();f.initialized=3;f.root_map=1;f.at_safepoint=1;f.slot[0]=Slot(1,source,offset,0);f.slot[1]=Slot(0,0,0,source)
  forward=(U8*16)();length=(U8*16)();forward[source-1]=target;length[source-1]=8;before=bytes(f)
  result=lib.or_safepoint(ref(f),forward,length)
  if offset<=8:assert result==0 and f.slot[0].base==target and f.slot[0].offset==offset and f.slot[1].scalar==source
  else:assert result==1 and bytes(f)==before
  n+=1
 counts['safepoint']=n
 print(json.dumps({'executed':True,'cases':sum(counts.values()),'by_mechanism':counts,'limits':'Finite independent models; does not validate real VM adapters, compiler stack maps, hardware or learner mastery.'}))
if __name__=='__main__':main()
