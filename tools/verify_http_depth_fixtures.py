"""Independent Python integer/calendar/dictionary oracle calls actual C."""
import argparse,ctypes as C,datetime as D,json,random
def main():
 p=argparse.ArgumentParser();p.add_argument('--library',required=True);a=p.parse_args();lib=C.CDLL(a.library);u=C.c_uint32;s=C.c_size_t
 lib.hd_length.argtypes=[C.c_char_p,s,u,C.POINTER(u)]
 lib.hd_date.argtypes=[C.c_uint,u,u,C.c_void_p,s,C.POINTER(s)]
 cases={'length':0,'date':0,'lifecycle':0};rng=random.Random(42002)
 values=[b'',b'+1',b'1025x',b'4294967296',b'\x001',b' 1']+[str(x).encode() for x in range(1100)]+[str(rng.randrange(1<<40)).encode() for _ in range(1000)]
 for raw in values:
  cap=rng.choice([0,1,1024]);out=u(987654);expected=400 if not raw or not all(48<=x<=57 for x in raw) or int(raw)>(1<<32)-1 else 413 if int(raw)>cap else 0
  actual=lib.hd_length(raw,len(raw),cap,C.byref(out));assert actual==expected,(raw,actual,expected)
  assert out.value==(int(raw) if not expected else 987654);cases['length']+=1
 epoch=D.date(1970,1,1);days=[0,11016,47541,2932896]+[rng.randrange(2932897) for _ in range(2000)]
 for day in days:
  sec=rng.randrange(86400);value=epoch+D.timedelta(days=day);expected=f'{["Mon","Tue","Wed","Thu","Fri","Sat","Sun"][value.weekday()]}, {value.day:02} {["Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"][value.month-1]} {value.year:04} {sec//3600:02}:{sec//60%60:02}:{sec%60:02} GMT'.encode()
  out=C.create_string_buffer(30);n=s(999);assert lib.hd_date(1,day,sec,out,30,C.byref(n))==0;assert n.value==29 and out.value==expected,(day,out.value,expected);cases['date']+=1
 class Handle(C.Structure):_fields_=[('index',u),('generation',u)]
 class Slot(C.Structure):_fields_=[('fd',C.c_int),('generation',u),('live',C.c_uint),('retired',C.c_uint),('interest',C.c_uint)]
 class Pool(C.Structure):_fields_=[('slots',Slot*4)]
 lib.hd_admit.argtypes=[C.POINTER(Pool),C.c_int,C.c_uint,C.POINTER(Handle)];lib.hd_close.argtypes=[C.POINTER(Pool),Handle,C.POINTER(C.c_int)];lib.hd_interest.argtypes=[C.POINTER(Pool),Handle,C.c_uint];lib.hd_event.argtypes=[C.POINTER(Pool),Handle,C.c_int,C.c_uint,C.POINTER(C.c_uint)]
 pool=Pool();model=[dict(fd=-1,gen=0,live=False,interest=0) for _ in range(4)];history=[]
 for step in range(3000):
  action=rng.randrange(4)
  if action==0:
   fd=rng.randrange(8)+10;h=Handle(999,999);free=next((i for i,v in enumerate(model) if not v['live']),None);expected=1 if any(v['live'] and v['fd']==fd for v in model) else 2 if free is None else 0
   assert lib.hd_admit(C.byref(pool),fd,1,C.byref(h))==expected
   if not expected:model[free].update(fd=fd,gen=model[free]['gen']+1,live=True,interest=1);assert(h.index,h.generation)==(free,model[free]['gen']);history.append((h,fd))
   else:assert h.index==999 and h.generation==999
  elif history:
   h,fd=rng.choice(history);v=model[h.index];live=v['live'] and v['gen']==h.generation
   if action==1:
    out=C.c_int(-99);assert lib.hd_close(C.byref(pool),h,C.byref(out))==(0 if live else 3)
    if live:assert out.value==fd;v.update(live=False,interest=0)
    else:assert out.value==-99
   elif action==2:
    mask=rng.randrange(4);assert lib.hd_interest(C.byref(pool),h,mask)==(0 if live else 3)
    if live:v['interest']=mask
   else:
    ready=rng.randrange(16);out=C.c_uint(99);assert lib.hd_event(C.byref(pool),h,fd,ready,C.byref(out))==(0 if live else 3);assert out.value==(ready&(v['interest']|12) if live else 99)
  cases['lifecycle']+=1
 print(json.dumps(dict(passed=True,cases=cases,total=sum(cases.values()),scope='Independent representations, same-author finite generated tests; no physical or independent assessor claim.')))
if __name__=='__main__':main()
