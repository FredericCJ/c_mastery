"""Independent assembler bytes and real linker veneer; no target execution claim."""
from pathlib import Path
import subprocess,json,hashlib,datetime,ctypes as C,struct
ROOT=Path(__file__).resolve().parents[1]
def section(path,name):
 b=path.read_bytes();assert b[4:6]==b'\x02\x01';off=struct.unpack_from('<Q',b,40)[0];size,num,strings=struct.unpack_from('<HHH',b,58);rows=[struct.unpack_from('<IIQQQQIIQQ',b,off+i*size) for i in range(num)];s=rows[strings];names=b[s[4]:s[4]+s[5]]
 for r in rows:
  n=names[r[0]:].split(b'\0',1)[0].decode()
  if n==name:return b[r[4]:r[4]+r[5]]
 raise ValueError(name)
def main():
 lab=ROOT/'labs/architecture_ports/targets';ref=ROOT/'assessor/architecture_ports/targets/encoding.c';w=ROOT/'build/b06-encoding';w.mkdir(parents=True,exist_ok=True);rows=[]
 def run(label,cmd):
  p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True);row=dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=p.returncode==0);rows.append(row);return row['passed']
 so=w/'encoding.so';run('encoder build',['gcc','-std=c17','-shared','-fPIC','-I',lab,ref,'-o',so]);lib=C.CDLL(str(so));lib.pe_branch.argtypes=[C.c_int32,C.c_uint,C.c_uint,C.POINTER(C.c_uint32)];lib.pe_jump.argtypes=[C.c_int32,C.c_uint,C.POINTER(C.c_uint32)]
 for kind,values in [('branch',[-4096,-4094,-2050,-2048,-34,-2,0,2,30,32,2046,2048,4094]),('jump',[-1048576,-1048574,-4098,-4096,-2050,-2,0,2,2046,2048,4094,4096,1048574])]:
  asm=w/(kind+'.S');asm.write_text('.option norvc\n.option norelax\n.text\n'+''.join((f'beq x3,x4,{v}\n' if kind=='branch' else f'jal x1,{v}\n') for v in values));obj=w/(kind+'.o');run(kind+' independent assembler',['clang','--target=riscv64-none-elf','-march=rv64i','-c',asm,'-o',obj]);words=struct.unpack('<'+'I'*len(values),section(obj,'.text'));got=[]
  for v in values:
   out=C.c_uint32(91);ok=lib.pe_branch(v,3,4,C.byref(out)) if kind=='branch' else lib.pe_jump(v,1,C.byref(out));got.append(out.value if ok else None)
  rows.append(dict(label=kind+' exact instruction words',passed=got==list(words),displacements=values,assembler=list(words),implementation=got))
 out=C.c_uint32(91);ok=True
 for d in [-4098,4096,1,-1]:ok &= lib.pe_branch(d,3,4,C.byref(out))==0 and out.value==91
 for d in [-1048578,1048576,1,-1]:ok &= lib.pe_jump(d,1,C.byref(out))==0 and out.value==91
 rows.append(dict(label='range parity unchanged rejection',passed=bool(ok)))
 lld=Path.home()/'cmastery-b06-tools/usr/bin/ld.lld-21';elf=w/'veneer.elf';run('out-of-range AArch64 call link',['clang','--target=aarch64-none-elf','-nostdlib','-fuse-ld='+str(lld),'-Wl,-T,'+str(lab/'veneer.ld'),lab/'veneer.S','-o',elf]);run('veneer symbol identity',['readelf','-sW',elf]);entry=section(elf,'.entry');words=list(struct.unpack('<'+'I'*(len(entry)//4),entry));# LLD emitted a literal load and BR x16; retain actual words and target literal.
 rows.append(dict(label='veneer uses IP0 not argument/callee saved registers',passed=len(words)==6 and words[2]==0x58000050 and words[3]==0xd61f0200 and words[4]+(words[5]<<32)==0x50000000,entry_words=words,scope='LDR x16 from target literal plus BR x16 witnessed; flags/IP1 remain permitted clobbers even if unused by this linker'))
 files=[Path(__file__),ref,*[lab/n for n in ['encoding.h','encoding.c','veneer.S','veneer.ld']]];report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),source_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['Independent Clang assembler compared26 scattered-immediate boundary words; not exhaustive every encoding.','Veneer actual link/disassembly bytes only; not execution or every linker convention.']);(ROOT/'research/v5/expansion/b06/encoding_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'passed':report['passed'],'checks':len(rows)}));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
