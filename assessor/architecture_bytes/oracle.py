"""Independent mathematical, encoded-byte and actual linked-placement checks."""
import json,pathlib,re,statistics,sys
def check(path,case):
 text=path.read_text();rows=[line.split()[2:] for line in text.splitlines() if line.startswith('DATA ')];rows=[[int(v) for v in row] for row in rows]
 if case=='registers':assert rows==[[0x11223344556677aa,0x112233445566bbcc,0x89abcdef]]
 elif case=='bytes':
  expected=[]
  for scale in [1,2,4,8]:
   for index in range(9):
    for disp in range(-9,10):
     for width in range(1,9):
      pos=index*scale+disp;ok=0<=pos and pos+width<=32;expected.append([scale,index,disp%(2**64),width,0 if ok else 4,4096+pos if ok else 99,4096+pos+width if ok else 88])
  assert rows==expected
 elif case=='storage':
  values=[-2**31,-1,0,1,2**31-1];expected=[[a,b,big,*values[a].to_bytes(4,'big' if big else 'little',signed=True),*values[b].to_bytes(4,'big' if big else 'little',signed=True)] for a in range(5) for b in range(5) for big in range(2)];assert rows==expected
 elif case=='decode':assert rows==[[h,(h>>8)&7,h&255] for h in range(0x2800,0x3000)]+[[6144,2048,57344]]
 elif case=='branches':
  expected=[[pc,imm,pc+2+int.from_bytes(bytes([imm]),'little',signed=True)] for pc in [0x1006,0x2006,0x3006] for imm in range(256)]
  expected += [[pc,cond,imm,pc+4+2*int.from_bytes(bytes([imm]),'little',signed=True)] for pc in [0x2000,0x3000] for cond in range(14) for imm in range(256)];assert rows==expected
 elif case=='timing':
  samples=[list(map(int,line.split()[1:])) for line in text.splitlines() if line.startswith('TIME ')];assert len(samples)==124;assert {(s,p,k) for s,p,k,*_ in samples}=={(s,p,k) for s in range(2) for p in range(31) for k in range(2)}
  for size,pair,kind,n,ns,c0,c1,value in samples:assert n==[1000,100000][size] and ns>=0 and c0>=0 and c1>=0 and value==36
  for size in range(2):
   for pair in range(31):assert [x[2] for x in samples if x[:2]==[size,pair]]==[pair&1,1-(pair&1)]
  return dict(status='pass',packets=124,median_ns=[[s,k,statistics.median(x[4] for x in samples if x[0]==s and x[2]==k)] for s in range(2) for k in range(2)],migrations=sum(x[5]!=x[6] for x in samples),limits='Unpinned CPU, scheduling/cache/assembly-size confounds retained; no universal dependency latency conclusion.')
 else:raise AssertionError(case)
 return dict(status='pass',packets=len(rows))
def main():
 p=pathlib.Path(sys.argv[1]);case=sys.argv[2]
 if case=='encoding':
  att=(p/'att.bin').read_bytes();intel=(p/'intel.bin').read_bytes();wrong=(p/'wrong.bin').read_bytes();thumb=(p/'thumb.bin').read_bytes()
  assert att==intel==bytes.fromhex('48 3d ff ff ff ff 77 fe 48 3d 78 56 34 12 3c 7f 48 83 f8 ff');assert wrong==bytes.fromhex('3d 78 56 34 12');assert thumb==bytes.fromhex('05 2a fe d1 aa 20')
  result=dict(status='pass',syntax_equivalent=True,wrong_operand_width_distinct=True,original_directive_bytes=att[:8].hex(),thumb_bytes=thumb.hex(),mnemonic_minus_one='48 83 f8 ff: actual GAS shorter form deliberately outside selected decoder')
 elif case=='placement':
  records={line.split()[-1]:(int(line.split()[0],16),int(line.split()[1],16)) for line in p.read_text().splitlines() if len(line.split())==4};assert records['ab_cpu_only']==(0x10000000,16) and records['ab_dma_candidate']==(0x20000000,16);assert all(a%4==0 and n==16 for a,n in records.values());result=dict(status='pass',objects=records,physical=False,claim='Actual typed linker placement; RM0090 CPU-only CCM excludes DMA, SRAM candidate needs remaining peripheral ownership/configuration obligations.')
 else:result=check(p,case)
 print(json.dumps(result))
if __name__=='__main__':main()
