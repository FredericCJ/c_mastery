"""Independent Python codec comparisons, never linked into learner/target code."""
import codecs,ctypes as C,json,random,sys
class State(C.Structure):_fields_=[('bytes',C.c_ubyte*4),('used',C.c_uint)]
class Result(C.Structure):_fields_=[('consumed',C.c_size_t),('produced',C.c_size_t)]
lib=C.CDLL(sys.argv[1]);lib.lu_decode.argtypes=[C.POINTER(State),C.POINTER(C.c_ubyte),C.c_size_t,C.POINTER(C.c_uint32),C.c_size_t,C.c_int,C.POINTER(Result)];lib.lu_encode.argtypes=[C.c_uint32,C.POINTER(C.c_ubyte),C.c_size_t,C.POINTER(C.c_size_t)];lib.lu_utf16.argtypes=[C.c_uint32,C.POINTER(C.c_uint16),C.c_size_t,C.POINTER(C.c_size_t)]
counts={'scalar_encodings':0,'utf16_encodings':0,'stream_calls':0,'invalid_rejections':0,'capacity_rejections':0,'incomplete_prefixes':0,'codec_deferred_impossible_prefix':0};valid_prefixes=set()
for u in range(0x110000):
 if 0xd800<=u<=0xdfff:continue
 out=(C.c_ubyte*5)(*[99]*5);n=C.c_size_t(99);expected=chr(u).encode('utf-8');assert lib.lu_encode(u,out,4,C.byref(n))==0 and bytes(out[:n.value])==expected and out[n.value]==99;counts['scalar_encodings']+=1
 for cut in range(1,len(expected)):valid_prefixes.add(expected[:cut])
 units=(C.c_uint16*3)(99,99,99);expected16=chr(u).encode('utf-16-le');assert lib.lu_utf16(u,units,2,C.byref(n))==0 and b''.join(int(x).to_bytes(2,'little') for x in units[:n.value])==expected16 and units[n.value]==99;counts['utf16_encodings']+=1
def trace(data,policy,cuts,cap):
 state=State();oracle=codecs.getincrementaldecoder('utf-8')('strict' if policy==0 else 'replace');offset=0
 for end in cuts:
  part=data[offset:end];offset=end;before=bytes(state);old=oracle.getstate();result=Result(99,99);out=(C.c_uint32*40)(*[0xdeadbeef]*40);inp=(C.c_ubyte*len(part)).from_buffer_copy(part)
  try:
   answer=oracle.decode(part,final=False)
   pending=oracle.getstate()[0]
   if pending and pending not in valid_prefixes:
    counts['codec_deferred_impossible_prefix']+=1
    # Python may defer ED A0 until another call. The contract instead requires
    # an actually extendible prefix. The prefix set comes from its complete
    # scalar encoder, not the C decoder's branch/range algorithm.
    answer+=oracle.decode(b'',final=True)
   expected=3 if len(answer)>cap else (1 if oracle.getstate()[0] else 0)
  except UnicodeDecodeError:expected=2;answer=''
  actual=lib.lu_decode(C.byref(state),inp,len(part),out,cap,policy,C.byref(result));assert actual==expected,(data.hex(),part.hex(),policy,cap,actual,expected)
  if expected in (2,3):
   oracle.setstate(old);assert bytes(state)==before and result.consumed==99 and result.produced==99 and list(out)==[0xdeadbeef]*40;counts['invalid_rejections' if expected==2 else 'capacity_rejections']+=1
  else:
   assert result.consumed==len(part) and result.produced==len(answer) and list(out[:len(answer)])==list(map(ord,answer)) and list(out[len(answer):])==[0xdeadbeef]*(40-len(answer))
   assert bytes(state.bytes[:state.used])==oracle.getstate()[0]
   counts['incomplete_prefixes']+=expected==1
  counts['stream_calls']+=1
for a in range(256):
 for b in range(256):
  data=bytes((a,b))
  for policy in (0,1):trace(data,policy,[2],4)
rng=random.Random(120617)
for i in range(4096):
 data=bytes(rng.randrange(256) for _ in range(rng.randrange(1,33)))
 for policy in (0,1):trace(data,policy,sorted(set([len(data),rng.randrange(len(data)+1),rng.randrange(len(data)+1)])),i%5)
print(json.dumps({'passed':True,'seed':120617,'counts':counts,'limits':'Finite byte/chunk/capacity traces, all Unicode scalar encodings. Python host codecs are an independent implementation, not proof of all streaming histories.'},sort_keys=True))
