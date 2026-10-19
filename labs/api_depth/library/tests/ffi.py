"""Independent ctypes client: explicit ABI, no private layout assumptions."""
import ctypes as c,gc,json,sys
lib=c.CDLL(sys.argv[1]);bytep=c.POINTER(c.c_ubyte);done_type=c.CFUNCTYPE(None,c.c_void_p,c.c_int)
signatures={'lc_release':([],c.c_uint),'lc_info':([c.c_uint,bytep,c.c_size_t],c.c_int),'lc_create':([c.c_size_t],c.c_void_p),'lc_append':([c.c_void_p,bytep,c.c_size_t],c.c_int),'lc_view':([c.c_void_p,c.POINTER(c.c_size_t)],bytep),'lc_copy':([c.c_void_p,bytep,c.c_size_t,c.POINTER(c.c_size_t)],c.c_int),'lc_release_buffer':([c.c_void_p],None),'lc_start':([done_type,c.c_void_p],c.c_void_p),'lc_cancel':([c.c_void_p],c.c_int),'lc_poll':([c.c_void_p],c.c_int),'lc_ack':([c.c_void_p],c.c_int),'lc_release_operation':([c.c_void_p],c.c_int)}
for name,(args,result) in signatures.items():getattr(lib,name).argtypes=args;getattr(lib,name).restype=result
release=lib.lc_release();descriptor=(c.c_ubyte*24)(*([99]*24));assert lib.lc_info(1,descriptor,16)==0
assert list(descriptor[:8])==[76,67,1,release,c.sizeof(c.c_void_p),c.sizeof(c.c_size_t),c.alignment(c.c_size_t),0]
for abi,length in [(0,16),(2,16),(1,0),(1,15)]:
 before=bytes(descriptor);assert lib.lc_info(abi,descriptor,length)==3 and bytes(descriptor)==before
assert lib.lc_info(1,descriptor,24)==(3 if release==1 else 0)
assert bytes(descriptor[16:])==bytes([99]*8)
for cap in range(9):
 p=lib.lc_create(cap);assert p
 try:
  n=c.c_size_t(999);view=lib.lc_view(p,c.byref(n));assert n.value==0
  data=(c.c_ubyte*3)(65,0,66);status=lib.lc_append(p,data,3);assert status==(0 if cap>=3 else 2)
  view=lib.lc_view(p,c.byref(n));assert n.value==(3 if cap>=3 else 0);assert c.string_at(view,n.value)==(b'A\0B' if cap>=3 else b'')
  # Preserve opaque address for paired release; c_char_p would hide it in a copy.
  output=(c.c_ubyte*4)(7,7,7,7);written=c.c_size_t(88)
  if n.value:assert lib.lc_copy(p,output,2,c.byref(written))==2 and written.value==88 and bytes(output)==b'\7'*4
  assert lib.lc_copy(p,output,4,c.byref(written))==0 and written.value==n.value
 finally:lib.lc_release_buffer(p)
events=[]
oracle_cases=0
for cap in range(9):
 for prior in range(cap+1):
  for count in range(10):
   p=lib.lc_create(cap);assert p
   prefix=bytes(range(prior));incoming=bytes(range(10,10+count));a=(c.c_ubyte*prior)(*prefix);b=(c.c_ubyte*count)(*incoming)
   assert lib.lc_append(p,a,prior)==0
   expected=(2,prefix) if prior+count>cap else (0,prefix+incoming)
   assert lib.lc_append(p,b,count)==expected[0]
   n=c.c_size_t();v=lib.lc_view(p,c.byref(n));assert c.string_at(v,n.value)==expected[1];lib.lc_release_buffer(p);oracle_cases+=1
assert oracle_cases==450
@done_type
def callback(context,status):events.append((context,status))
retained=[callback];operation=lib.lc_start(retained[0],123);assert operation
gc.collect();assert lib.lc_cancel(operation)==5 and not events
assert lib.lc_release_operation(operation)==7;assert lib.lc_poll(operation)==5
assert lib.lc_ack(operation)==0 and events==[(123,6)];assert lib.lc_release_operation(operation)==0;retained.clear();gc.collect()
# Analyze the unsuitable default-int interpretation without making a misdeclared
# call, dereferencing a truncated value, or leaking a second allocation.
p=lib.lc_create(0);assert p;truncated=c.c_int(p).value;lib.lc_release_buffer(p)
print(json.dumps(dict(release=release,pointer_bytes=c.sizeof(c.c_void_p),size_t_bytes=c.sizeof(c.c_size_t),default_int_reconstruction_equal=(truncated==p),descriptor_tail_preserved=True,callback_retained_until_ack=True,independent_append_oracle_cases=oracle_cases)))
