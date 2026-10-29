"""Independent finite traces and offline DP; never a native device oracle."""
import ctypes as C
from functools import lru_cache
import itertools
import json
from pathlib import Path
import statistics
import sys
U=C.c_uint
Z=C.c_size_t
B=C.c_ubyte
P=C.POINTER

def main():
 lib=C.CDLL(str(Path(sys.argv[1]).resolve()));count=0
 class Checksum(C.Structure):_fields_=[('sum',C.c_uint32),('pending',U),('octet',U)]
 lib.s_checksum_add.argtypes=[P(Checksum),P(B),Z]
 lib.s_checksum_finish.argtypes=[P(Checksum),P(C.c_uint16)]
 for n in range(7):
  for values in itertools.product((0,1,128,255),repeat=n):
   words=list(values)+([0] if n%2 else [])
   total=sum(256*words[i]+words[i+1] for i in range(0,len(words),2))
   while total>65535:total=(total&65535)+(total>>16)
   expected=65535-total
   for split in range(n+1):
    s=Checksum();a=(B*split)(*values[:split]);b=(B*(n-split))(*values[split:]);out=C.c_uint16()
    assert lib.s_checksum_add(C.byref(s),a,split)==0
    assert lib.s_checksum_add(C.byref(s),b,n-split)==0
    assert lib.s_checksum_finish(C.byref(s),C.byref(out))==0 and out.value==expected,(values,split)
    count+=1
 class Replacement(C.Structure):_fields_=[('faults',U),('evicted',C.c_int*32)]
 lib.s_replace.argtypes=[P(B),Z,U,U,P(Replacement)]
 # DP enumerates all legal victim choices; no furthest-future greedy rule.
 for n in range(7):
  for values in itertools.product(range(3),repeat=n):
   for frames in (1,2,3):
    @lru_cache(None)
    def optimum(at,resident):
     if at==n:return 0
     page=values[at]
     if page in resident:return optimum(at+1,resident)
     if len(resident)<frames:return 1+optimum(at+1,tuple(sorted((*resident,page))))
     return 1+min(optimum(at+1,tuple(sorted([x for x in resident if x!=victim]+[page]))) for victim in resident)
    # Normalize all states before use; sorting affects memoization only.
    expected=optimum(0,());r=Replacement();a=(B*n)(*values)
    assert lib.s_replace(a,n,frames,1,C.byref(r))==0 and r.faults==expected,(values,frames)
    resident=set()
    for i,page in enumerate(values):
     victim=r.evicted[i]
     if victim>=0:assert victim in resident;resident.remove(victim)
     resident.add(page);assert len(resident)<=frames
    count+=1
 lib.s_paired.argtypes=[P(C.c_uint32),P(C.c_uint32),Z,P(C.c_int64),P(U)]
 for n in range(1,33):
  for seed in range(16):
   a=[(i*7919+seed*104729)%(2**32) for i in range(n)]
   b=[((i+seed)*65537+17)%(2**32) for i in range(n)]
   out=C.c_int64();wins=U()
   assert lib.s_paired((C.c_uint32*n)(*a),(C.c_uint32*n)(*b),n,C.byref(out),C.byref(wins))==0
   assert out.value==statistics.median_low([y-x for x,y in zip(a,b)]) and wins.value==sum(y<x for x,y in zip(a,b))
   count+=1
 class Shoot(C.Structure):_fields_=[('generation',C.c_uint32),('active',U),('pending',U),('targets',U),('reclaimed',U)]
 lib.s_shoot_begin.argtypes=[P(Shoot),U];lib.s_shoot_ack.argtypes=[P(Shoot),U,C.c_uint32];lib.s_shoot_reclaim.argtypes=[P(Shoot)]
 for mask in range(256):
  for order in (range(8),reversed(range(8))):
   s=Shoot();assert lib.s_shoot_begin(C.byref(s),mask)==0
   remaining={i for i in range(8) if mask&(1<<i)}
   for cpu in order:
    if cpu not in remaining:continue
    assert lib.s_shoot_reclaim(C.byref(s))==4
    assert lib.s_shoot_ack(C.byref(s),cpu,0)==3
    assert lib.s_shoot_ack(C.byref(s),cpu,1)==0;remaining.remove(cpu)
    assert s.pending==sum(1<<i for i in remaining)
   assert lib.s_shoot_reclaim(C.byref(s))==0 and s.reclaimed
   count+=1
 print(json.dumps({'passed':True,'independent_cases':count,'scope':['all alphabet4 checksum inputs length0..6 at every two-chunk split','offline replacement DP alphabet3 length0..6','paired statistics32 lengths16 seeds','all8-struct lr_cpu masks in two acknowledgement orders'],
                   'limits':'Finite serialized host models; no independent human examiner, timing observation or hardware execution.'}))
if __name__=='__main__':main()
