"""Independent finite page-set oracle; does not execute an actual MMU."""
import argparse,ctypes as C,json,random,struct
class Boot(C.Structure):_fields_=[('module_start',C.c_uint32),('module_end',C.c_uint32),('map',C.c_void_p),('map_bytes',C.c_uint32),('entry_size',C.c_uint32)]
class Frames(C.Structure):_fields_=[('state',C.c_ubyte*32768)]
class Span(C.Structure):_fields_=[('start',C.c_uint64),('end',C.c_uint64)]
def main(path):
 lib=C.CDLL(path);lib.kf_frames.argtypes=[C.POINTER(Frames),C.POINTER(Boot),C.POINTER(Span),C.c_size_t];lib.kf_frames.restype=C.c_int;rng=random.Random(4802);count=0
 for _ in range(128):
  records=[(0x300000,0x100000,1)]
  for i in range(5):
   base=0x300000+rng.randrange(0x100000);length=rng.randrange(0x10000);records.append((base,length,1 if i%2 else 2))
  reservations=[]
  for i in range(3):
   start=0x300000+rng.randrange(0x100000);reservations.append((start,start+rng.randrange(0x10000)))
  payload=b''.join(struct.pack('<QQII',a,n,t,0) for a,n,t in records);buffer=C.create_string_buffer(payload);boot=Boot(0,0,C.addressof(buffer),len(payload),24);spans=(Span*len(reservations))(*(Span(*s) for s in reservations));out=Frames();assert lib.kf_frames(C.byref(out),C.byref(boot),spans,len(spans))==1
  usable=set();excluded=set()
  for a,n,t in records:
   if t==1:usable.update(range((a+4095)//4096,(a+n)//4096))
   elif n:excluded.update(range(a//4096,(a+n+4095)//4096))
  for a,z in reservations:
   if a<z:excluded.update(range(a//4096,(z+4095)//4096))
  expected={i for i in usable-excluded if 0<i<32768};actual={i for i,x in enumerate(out.state) if x==1};assert actual==expected;assert all(x in (0,1) for x in out.state);count+=1
 print(json.dumps({'passed':True,'seed':4802,'cases':count,'scope':'Independent set construction by same author,not independent assessor or physical memory map.'}))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('library');main(p.parse_args().library)
