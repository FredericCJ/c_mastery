"""Independent ELF32 little-endian image contract; no target execution."""
import argparse,json,pathlib,struct
def inspect(path,code,bank):
 b=pathlib.Path(path).read_bytes()
 if b[:7]!=b'\x7fELF\x01\x01\x01':raise ValueError('unsupported ELF encoding')
 if len(b)<52:raise ValueError('truncated ELF header')
 h=struct.unpack_from('<16sHHIIIIIHHHHHH',b);_,kind,machine,version,entry,phoff,shoff,flags,ehsize,phsize,phnum,shsize,shnum,shnames=h
 if machine!=40 or kind!=2 or version!=1 or shsize!=40 or phsize!=32:raise ValueError('unsupported target/encoding')
 if shoff+shnum*40>len(b) or phoff+phnum*32>len(b) or shnames>=shnum:raise ValueError('truncated tables')
 sections=[struct.unpack_from('<10I',b,shoff+40*i) for i in range(shnum)];ns=sections[shnames];names=b[ns[4]:ns[4]+ns[5]]
 def name(off):return names[off:names.index(0,off)].decode('ascii')
 sec={name(s[0]):s for s in sections if s[0]}
 def data(s):
  if s[1]==8 or s[4]+s[5]>len(b):raise ValueError('missing file bytes')
  return b[s[4]:s[4]+s[5]]
 vec=sec.get('.vectors')
 if not vec or vec[3]!=code or code%512 or vec[5]!=428:raise ValueError('vector placement/extent')
 sp,pc=struct.unpack_from('<II',data(vec))
 if sp!=0x20010000 or sp%8 or not(pc&1) or not(code<=pc-1<code+128*1024):raise ValueError('stack/reset encoding')
 symbols={};st=sec.get('.symtab')
 if not st:raise ValueError('symbol table required for teaching audit')
 strings=sections[st[6]];sb=data(strings)
 for i in range(0,st[5],st[9]):
  no,value,size,info,other,idx=struct.unpack_from('<IIIBBH',b,st[4]+i)
  if no:symbols[sb[no:sb.index(0,no)].decode()]=value
 lo,hi=symbols['_stack_low'],symbols['_stack_top']
 if lo%32 or hi%8 or hi-lo!=2048 or lo<0x20000020:raise ValueError('stack interval/alignment')
 for n in ['.data','.bss','.retained','.stage','.dma']:
  s=sec[n]
  if s[3]<0x20000000 or s[3]+s[5]>lo-32:raise ValueError('stack overlap '+n)
 for n in ['.bank_data','.bank_bss']:
  s=sec[n]
  if not(bank<=s[3] and s[3]+s[5]<=bank+16384):raise ValueError('second bank '+n)
 if any(sec[n][1]!=8 for n in ['.bss','.bank_bss','.retained','.stage','.dma']):raise ValueError('no-load ownership')
 def lma(s):
  for i in range(phnum):
   typ,off,va,pa,fs,ms,fl,al=struct.unpack_from('<8I',b,phoff+32*i)
   if typ==1 and off<=s[4] and s[4]+s[5]<=off+fs:return pa+s[4]-off
  raise ValueError('no load segment')
 copies=[]
 for n in ['.data','.bank_data']:
  s=sec[n];load=lma(s)
  if not(code<=load and load+s[5]<=code+128*1024):raise ValueError('copy LMA outside code')
  copies.extend([load,s[3],s[3]+s[5]])
 copies.extend([lma(sec['.data']),sec['.data'][3],sec['.data'][3]])
 zeros=[]
 for n in ['.bss','.bank_bss']:s=sec[n];zeros.extend([s[3],s[3]+s[5]])
 table=data(sec['.tables']);words=list(struct.unpack('<'+'I'*(len(table)//4),table))
 if words!=copies+zeros:raise ValueError('copy/zero table disagrees with independent ELF load mapping')
 return dict(passed=True,entry=entry,stack=[lo,hi],vectors=[sp,pc],copy_words=copies,zero_words=zeros,sections={n:dict(address=s[3],size=s[5],type=s[1]) for n,s in sec.items() if s[2]&2})
def main():
 p=argparse.ArgumentParser();p.add_argument('image');p.add_argument('--code',type=lambda s:int(s,0),required=True);p.add_argument('--bank',type=lambda s:int(s,0),required=True);a=p.parse_args()
 try:r=inspect(a.image,a.code,a.bank)
 except (ValueError,KeyError,struct.error,IndexError) as e:print('BU_IMAGE_REJECT:'+str(e));return 1
 print(json.dumps(r));return 0
if __name__=='__main__':raise SystemExit(main())
