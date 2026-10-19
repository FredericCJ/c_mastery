"""Independent regular-language record oracle and arbitrary-precision counts."""
import ctypes,itertools,json,re,sys
class Record(ctypes.Structure):_fields_=[('count',ctypes.c_int32),('length',ctypes.c_size_t),('name',ctypes.c_ubyte*33)]
lib=ctypes.CDLL(sys.argv[1]);lib.ls_record_parse.argtypes=[ctypes.c_char_p,ctypes.c_size_t,ctypes.c_int,ctypes.POINTER(Record)];lib.ls_record_parse.restype=ctypes.c_int
counts={'cases':0,'accepted':0,'rejected':0};alphabet=b'A;\\!'
names=[bytes(v) for n in range(6) for v in itertools.product(alphabet,repeat=n)]+[b'A'*32,b'A'*33,b'\\;'*32,b'\\;'*33,b'A\x00B']
for escaped in (0,1):
 pattern=rb'count=([0-9]+);name=('+ (rb'(?:[A-Za-z0-9_]|\\[;\\])+' if escaped else rb'[A-Za-z0-9_]+')+rb')'
 for count in (b'0',b'12',b'2147483647',b'2147483648',b'-1',b'',b'01'):
  for name in names:
   token=b'count='+count+b';name='+name;match=re.fullmatch(pattern,token);decoded=re.sub(rb'\\([;\\])',rb'\1',name) if escaped else name
   expected=bool(match and int(count)<=2147483647 and len(decoded)<=32)
   out=Record();out.count=77;out.length=4;out.name[:4]=b'KEEP';before=bytes(out);status=lib.ls_record_parse(token,len(token),escaped,ctypes.byref(out))
   assert (status==0)==expected,(token,escaped,status,expected)
   if expected:assert out.count==int(count) and out.length==len(decoded) and bytes(out.name)[:out.length+1]==decoded+b'\0'
   else:assert bytes(out)==before,(token,status)
   counts['cases']+=1;counts['accepted' if expected else 'rejected']+=1
print(json.dumps(dict(passed=True,counts=counts,bounds='All names length0..5 over A,semicolon,backslash,bang; five long/NUL boundaries; seven count forms; both policies')))
