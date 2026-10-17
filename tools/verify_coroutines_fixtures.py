"""Independent materialized, grouping and language-shape oracles for T72."""
import argparse,ctypes as c,itertools,json,math
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
class Range(c.Structure):_fields_=[(x,c.c_uint32) for x in ('current','end','step')]+[('done',c.c_uint)]
class Flow(c.Structure):_fields_=[('source',Range),('remaining',c.c_size_t),('pulls',c.c_size_t)]
class Row(c.Structure):_fields_=[('key',c.c_uint),('value',c.c_int32)]
class Query(c.Structure):_fields_=[('rows',c.POINTER(Row)),('count',c.c_size_t),('index',c.c_size_t),('closed',c.c_uint)]
class Memo(c.Structure):_fields_=[('state',c.c_uint),('end',c.c_size_t)]
class Trampoline(c.Structure):_fields_=[('n',c.c_uint),('depth',c.c_uint),('phase',c.c_uint),('stack',c.c_uint*12),('value',c.c_uint32)]
class Effect(c.Structure):_fields_=[('pc',c.c_uint),('generation',c.c_uint32),('first',c.c_uint32)]
class Continuation(c.Structure):_fields_=[('owner',c.POINTER(Effect)),('generation',c.c_uint32),('question',c.c_uint)]
def main():
 p=argparse.ArgumentParser();p.add_argument('--library');args=p.parse_args();lib=c.CDLL(args.library) if args.library else None;counts={k:0 for k in ('iterator','dataflow','cps','packrat','effect')}
 if lib:
  lib.co_flow_next.argtypes=[c.POINTER(Flow),c.POINTER(c.c_uint32)]
  lib.co_query_open.argtypes=[c.POINTER(Query),c.POINTER(Row),c.c_size_t];lib.co_query_next.argtypes=[c.POINTER(Query),c.POINTER(c.c_uint),c.POINTER(c.c_uint32)]
  lib.co_trampoline_init.argtypes=[c.POINTER(Trampoline),c.c_uint];lib.co_trampoline_step.argtypes=[c.POINTER(Trampoline),c.c_uint,c.POINTER(c.c_uint32)]
  lib.co_packrat.argtypes=[c.c_char_p,c.c_size_t,c.POINTER(Memo),c.c_size_t,c.POINTER(c.c_size_t),c.POINTER(c.c_uint)]
  lib.co_effect_start.argtypes=[c.POINTER(Effect),c.POINTER(Continuation)];lib.co_effect_resume.argtypes=[c.POINTER(Continuation),c.c_uint32,c.POINTER(Continuation),c.POINTER(c.c_uint32)]
 for end,step,take in itertools.product(range(33),range(1,9),range(6)):
  expected=[x*x for x in range(0,end,step) if x%2==0][:take]
  if lib:
   f=Flow(Range(0,end,step,0),take,0);out=c.c_uint32(91);got=[]
   while True:
    result=lib.co_flow_next(c.byref(f),c.byref(out))
    if result==7:break
    assert result==0;got.append(out.value);assert len(got)<=take
   assert got==expected
   if take==0:assert f.pulls==0
  counts['iterator']+=1
 possibilities=[(key,value) for key in (0,1) for value in (-1,0,2)]
 for n in range(6):
  for rows in itertools.product(possibilities,repeat=n):
   expected=[]
   for key,group in itertools.groupby(rows,key=lambda r:r[0]):
    values=[r[1] for r in group if r[1]>=0]
    if values:expected.append((key,sum(values)))
   if lib:
    data=(Row*n)(*(Row(*r) for r in rows));q=Query();assert lib.co_query_open(c.byref(q),data,n)==0;k=c.c_uint(91);v=c.c_uint32(92);got=[]
    while True:
     result=lib.co_query_next(c.byref(q),c.byref(k),c.byref(v))
     if result==7:break
     assert result==0;got.append((k.value,v.value));assert len(got)<=n
    assert got==expected
   counts['dataflow']+=1
 for n,budget in itertools.product(range(13),range(1,29)):
  if lib:
   t=Trampoline();assert lib.co_trampoline_init(c.byref(t),n)==0;out=c.c_uint32(91)
   for _ in range(30):
    result=lib.co_trampoline_step(c.byref(t),budget,c.byref(out))
    if result==7:break
    assert result==4 and out.value==91
   assert result==7 and out.value==math.factorial(n)
  counts['cps']+=1
 for n in range(8):
  for letters in itertools.product(b'abcx',repeat=n):
   s=bytes(letters);k=0
   while k<n and s[k]==ord('a'):k+=1
   expected=n==2*k+1 and s[k:k+1]==b'x' and all(ch in b'bc' for ch in s[k+1:])
   if lib:
    memo=(Memo*65)();end=c.c_size_t(91);calls=c.c_uint(0);result=lib.co_packrat(s,n,memo,65,c.byref(end),c.byref(calls))
    assert (result==0)==expected and result in (0,6) and calls.value<=n+1
    assert end.value==(n if expected else 91)
   counts['packrat']+=1
 for a,b in itertools.product([0,1,17,2**31,2**32-1],repeat=2):
  if lib:
   frame=Effect();k=Continuation();next_k=Continuation();out=c.c_uint32(91)
   assert lib.co_effect_start(c.byref(frame),c.byref(k))==8
   assert lib.co_effect_resume(c.byref(k),a,c.byref(next_k),c.byref(out))==8
   result=lib.co_effect_resume(c.byref(next_k),b,c.byref(k),c.byref(out))
   assert result==(7 if a+b<2**32 else 2)
   assert out.value==(a+b if result==7 else 91)
  counts['effect']+=1
 for i in range(1,10):assert json.loads((ROOT/f'fixtures/coroutines/PR-T72-{i:02}.json').read_text(encoding='utf-8'))['exercise']==f'X-T72-{i:03}'
 print(json.dumps(dict(passed=True,executed_against_c=bool(lib),cases=counts,total=sum(counts.values()),limits='Independent finite oracles cover five kernels; actual ABI/allocator/readiness/parser traces have separate concrete assertions and are not exhaustive.')))
if __name__=='__main__':main()
