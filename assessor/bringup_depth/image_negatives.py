"""Alter actual ELF metadata/bytes; validate intended rejection, not compilation failure."""
import importlib.util,pathlib,struct,sys,json
root=pathlib.Path(__file__).resolve().parents[2];spec=importlib.util.spec_from_file_location('imagecheck',root/'labs/bringup_depth/check_image.py');check=importlib.util.module_from_spec(spec);spec.loader.exec_module(check)
path=pathlib.Path(sys.argv[1]);out=pathlib.Path(sys.argv[2]);out.mkdir(parents=True,exist_ok=True);original=path.read_bytes();h=struct.unpack_from('<16sHHIIIIIHHHHHH',original);shoff=h[6];shnum=h[12];shnames=h[13];secs=[struct.unpack_from('<10I',original,shoff+40*i) for i in range(shnum)];ns=secs[shnames];strings=original[ns[4]:ns[4]+ns[5]]
byname={strings[s[0]:strings.index(0,s[0])].decode():(i,s) for i,s in enumerate(secs) if s[0]};vi,vs=byname['.vectors'];ti,ts=byname['.tables'];syi,sy=byname['.symtab'];ss=secs[sy[6]];names=original[ss[4]:ss[4]+ss[5]];symbols={}
for i in range(0,sy[5],sy[9]):
 no,value,size,info,other,idx=struct.unpack_from('<IIIBBH',original,sy[4]+i)
 if no:symbols[names[no:names.index(0,no)].decode()]=sy[4]+i+4
cases=[]
for label,reason in [('encoding','unsupported ELF encoding'),('truncated','truncated ELF header'),('vector-size','vector placement'),('thumb','stack/reset'),('alignment','stack/reset'),('stack-overlap','stack overlap'),('bad-copy-table','copy/zero table'),('retained-load','no-load ownership')]:
 b=bytearray(original)
 if label=='encoding':b[5]=2
 elif label=='truncated':b=b[:20]
 elif label=='vector-size':struct.pack_into('<I',b,shoff+40*vi+20,0)
 elif label=='thumb':b[vs[4]+4]&=0xfe
 elif label=='alignment':b[vs[4]]=4
 elif label=='stack-overlap':
  i,s=byname['.retained'];struct.pack_into('<I',b,shoff+40*i+12,0x2000f800)
 elif label=='bad-copy-table':b[ts[4]]^=4
 else:
  i,s=byname['.retained'];struct.pack_into('<I',b,shoff+40*i+4,1)
 dest=out/(label+'.elf');dest.write_bytes(b)
 try:check.inspect(dest,0,0x20010000)
 except (ValueError,KeyError,struct.error,IndexError) as e:
  assert reason in str(e),(label,str(e));cases.append(dict(case=label,rejected=str(e)))
 else:raise AssertionError('accepted '+label)
print(json.dumps(dict(passed=True,cases=cases)))
