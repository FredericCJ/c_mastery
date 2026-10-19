"""Snapshot/unsigned-order oracle and unbounded arithmetic differential model."""
import ctypes,json,random

def run(c,root,args):
 c.group='freestanding-core';public=root/'labs/machine_image/freestanding_depth';private=root/'assessor/legacy_systems/freestanding_depth';sources=[private/'bytes.c',private/'divide.c']
 for cc in ('gcc','clang'):
  for name,flags in [('debug',['-O0']),('release',['-O2']),('size',['-Os']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
   exe=c.root/(cc+'-'+name);c.run([cc,'-std=c17','-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin',*flags,'-I'+str(public),public/'tests.c',*sources,'-o',exe]);c.run([exe],contains='FREE-CORE-PASS')
 for name,flags,inputs in [('forward',['-DFR_MUTANT_FORWARD'],sources),('signed',['-DFR_MUTANT_SIGNED'],sources),('carry',['-DFR_MUTANT_CARRY'],sources),('learner',[],[public/'free.c'])]:
  exe=c.root/name;c.run(['gcc','-std=c17','-O2','-ffreestanding','-fno-builtin','-I'+str(public),*flags,public/'tests.c',*inputs,'-o',exe]);c.run([exe],expected=1,contains='FREE-CHECK:')
 class Wide(ctypes.Structure):_fields_=[('limb',ctypes.c_uint32*4)]
 def wide(n):return Wide((ctypes.c_uint32*4)(*[(n>>(32*i))&0xffffffff for i in range(4)]))
 def integer(n):return sum(int(n.limb[i])<<(32*i) for i in range(4))
 rng=random.Random(110417);mask=(1<<128)-1;cases=[(rng.getrandbits(128),rng.getrandbits(128) or 1) for _ in range(4096)]
 cases += [(n,d) for n in (0,1,2**63,2**64-1,2**64,2**127-1,2**127,mask) for d in (1,2,3,2**32-1,2**64+1,2**127-1,2**127,mask)]
 steps=[(rng.randrange(d),d,rng.randrange(2)) for d in [rng.getrandbits(128) or 1 for _ in range(4096)]]
 steps += [(2**127+3,2**127+7,bit) for bit in (0,1)]
 for cc in ('gcc','clang'):
  shared=c.root/(cc+'.so');c.run([cc,'-std=c17','-O2','-ffreestanding','-fno-builtin','-shared','-fPIC','-I'+str(public),*sources,'-o',shared]);lib=ctypes.CDLL(str(shared));f=lib.fr_divmod128;f.argtypes=[Wide,Wide,ctypes.POINTER(Wide),ctypes.POINTER(Wide)];f.restype=ctypes.c_int;step=lib.fr_divstep128;step.argtypes=[Wide,Wide,ctypes.c_uint,ctypes.POINTER(Wide),ctypes.POINTER(ctypes.c_uint)];step.restype=ctypes.c_int
  for n,d in cases:
   q=wide(mask);r=wide(mask);rc=f(wide(n),wide(d),ctypes.byref(q),ctypes.byref(r));expected=divmod(n,d)
   c.require(rc==0 and (integer(q),integer(r))==expected,'unbounded integer quotient/remainder oracle',dict(n=hex(n),d=hex(d)))
   c.require(integer(q)*d+integer(r)==n and integer(r)<d,'mathematical reconstruction without target overflow')
  high_count=0
  for r,d,bit in steps:
   out=wide(mask);q=ctypes.c_uint(9);rc=step(wide(r),wide(d),bit,ctypes.byref(out),ctypes.byref(q));expected=divmod(2*r+bit,d)
   c.require(rc==0 and (q.value,integer(out))==expected,'streaming129-bit oracle',dict(r=hex(r),d=hex(d),bit=bit));high_count+=r>=2**127
  c.require(high_count>0,'streaming carry path is exercised',high_count)
  # Derive an exact bound independently: before each ordinary iteration, its
  # processed prefix has <=127 bits; r<=prefix, so high carry is unreachable.
  c.require(max((mask>>(128-k)) for k in range(128))<2**127,'ordinary zero-initialized division carry is unreachable')
 c.write('arithmetic-oracle.json',json.dumps(dict(seed=110417,divisions=len(cases),steps=len(steps),integer_model='Python unbounded divmod; no target algorithm used',ordinary_carry='Proved unreachable with zero initial remainder and exactly128 numerator bits; streaming transfer exercises real129th-bit case'),indent=2))
 for name in ('bytes','divide'):
  obj=c.root/(name+'-arm.o');c.run(['arm-none-eabi-gcc','-std=c17','-O2','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-I'+str(public),'-c',private/(name+'.c'),'-o',obj]);undefined=c.run(['arm-none-eabi-nm','-u',obj]);c.require(not undefined.strip(),'Arm core object has no implicit runtime helper',dict(name=name,undefined=undefined));c.run(['arm-none-eabi-objdump','-dr',obj])
 # Scoped observations; no closure/interrupt/board/whole-runtime promotion.
 for oid,claim,variant in [('06.a','Copy/move/set/unsigned compare execute against snapshot/sign oracle',False),('06.a','Same-arena offset admission replaces unrelated-pointer relational ordering',True),('06.b','All1785 valid16-byte arena triples preserve snapshot and guards',False),('06.b','Separately allocated objects exercise declared flat-address adapter; portable contract remains offsets',True),('10.a','Independent byte snapshot and all65536 one-byte order pairs discriminate flawed forward/signed implementations',False),('10.a','153 split-copy cases and compare antisymmetry assessed with nonoverlap preconditions',True)]:c.observe('T11.'+oid,claim,['freestanding-core'],variant)
