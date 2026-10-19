"""Source-bound author compiler extension campaign; never learner acceptance."""
from pathlib import Path
import ctypes as C, datetime, hashlib,json,random,subprocess,tempfile,argparse
ROOT=Path(__file__).resolve().parents[1]
class Ins(C.Structure):_fields_=[('op',C.c_uint),('arg',C.c_uint)]
class Func(C.Structure):_fields_=[('begin',C.c_uint),('end',C.c_uint),('locals',C.c_uint),('types',C.c_uint*16),('name',C.c_char*16)]
class Program(C.Structure):_fields_=[('count',C.c_uint),('functions',C.c_uint),('code',Ins*512),('fn',Func*4)]
class Diagnostic(C.Structure):_fields_=[('begin',C.c_size_t),('end',C.c_size_t),('reason',C.c_uint)]
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--report',default='research/v5/expansion/b06/extension_validation.json');a=ap.parse_args();records=[];lab=ROOT/'labs/subset_compiler/extension';ref=ROOT/'assessor/subset_compiler/extension/extension.c';serial=ref.with_name('serialization.c');thumb1=ref.with_name('thumb1.c');files=list(lab.glob('*'))+list(ref.parent.glob('*'))+[Path(__file__)];pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files if p.is_file()}
 def run(cmd,label,expected=0,marker=None,timeout=60):
  try:
   r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=timeout);ok=r.returncode==expected and (marker is None or marker in r.stdout+r.stderr);v={'label':label,'command':list(map(str,cmd)),'returncode':r.returncode,'stdout':r.stdout,'stderr':r.stderr,'expected':expected,'marker':marker,'passed':ok}
  except (OSError,subprocess.TimeoutExpired) as e:v={'label':label,'command':list(map(str,cmd)),'passed':False,'error':str(e)}
  records.append(v);return v
 def check(label,ok,details):records.append(dict(label=label,passed=bool(ok),details=details))
 with tempfile.TemporaryDirectory(prefix='cm-b06-extension-') as td:
  w=Path(td)
  for compiler,flags in [('gcc',['-O0','-g']),('gcc',['-O3']),('clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
   exe=w/'test';base=[compiler,'-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',*flags,'-I',lab]
   if run(base+[ref,serial,thumb1,lab/'tests.c','-o',exe],compiler+' reference build')['passed']:run([exe],compiler+' semantic cases',marker='B06_EXT_PASS')
   for m in range(1,4):
    if run(base+['-DCE_MUTATION='+str(m),ref,serial,thumb1,lab/'tests.c','-o',exe],compiler+' mutant '+str(m)+' build')['passed']:run([exe],compiler+' mutant '+str(m)+' rejected',1,'B06_EXT_CHECK')
   if run(base+[lab/'extension.c',lab/'serialization.c',lab/'tests.c','-o',exe],compiler+' learner build')['passed']:run([exe],compiler+' learner semantic rejection',1,'B06_EXT_CHECK')
  shared=w/'compiler.so';ok=run(['gcc','-std=c17','-O2','-shared','-fPIC','-I',lab,ref,serial,thumb1,'-o',shared],'compiler shared build')['passed']
  if ok:
   cc=C.CDLL(str(shared));cc.ce_compile.argtypes=[C.c_char_p,C.c_size_t,C.POINTER(Program),C.POINTER(Diagnostic)];cc.ce_run.argtypes=[C.POINTER(Program),C.c_uint32,C.POINTER(C.c_uint),C.POINTER(C.c_uint32)];cc.ce_emit.argtypes=[C.POINTER(Program),C.c_uint,C.c_char_p,C.c_size_t]
   fixture=json.loads((lab/'cases.json').read_text());rng=random.Random(fixture['seed'])
   for row in fixture['cases']:
    source=row['source'].encode();p=Program();d=Diagnostic();accepted=cc.ce_compile(source,len(source),C.byref(p),C.byref(d));check(row['id']+' compile',accepted==1,{'diagnostic':d.reason,'instructions':p.count})
    if not accepted:continue
    xs=[0,1,2,3,65535,0xffffffff]+[rng.randrange(2**32) for _ in range(26)]
    def model(x):
     if row['model']=='signed-error':return None
     if row['model']=='side-effect-order':return (x+1)&0xffffffff
     if row['model']=='sum-seven':v=sum(2*(x+i) for i in range(7))+(-1 if x<3 else 2)
     elif row['model']=='array-eight':v=sum(x+i-1 for i in range(8))
     elif row['model']=='nested':v=x*x+1+(x+1)*(x+1)-x-3
     elif row['model']=='signed-checked':v=-21-(x if x<1000 else 0)
     else:v=34 if x==0 else 21+x
     return v&0xffffffff
    wants=list(map(model,xs));observed=[]
    for x,want in zip(xs,wants):
     fuel=C.c_uint(10000);value=C.c_uint32(99);result=cc.ce_run(C.byref(p),x,C.byref(fuel),C.byref(value));observed.append((x,want,result,value.value,fuel.value))
    check(row['id']+' independent Python/IR',all((r[2]==0 and r[3]==99) if r[1] is None else (r[2]==1 and r[1]==r[3]) for r in observed),observed)
    text=C.create_string_buffer(131072);assert cc.ce_emit(C.byref(p),0,text,len(text));asm=w/'target.S';asm.write_bytes(text.value);oracle=w/'oracle.c';oracle.write_text('#include <stdint.h>\n'+row['c']);dll=w/(row['id']+'.so')
    if run(['gcc','-shared','-fPIC','-O2',asm,lab/'hostile_x86.S',oracle,'-o',dll],row['id']+' emitted x86/GCC oracle link')['passed']:
     native=C.CDLL(str(dll));native.ce_checked.argtypes=[C.c_uint32,C.POINTER(C.c_uint),C.POINTER(C.c_uint)];native.ce_checked.restype=C.c_uint32;native.ref.argtypes=[C.c_uint32];native.ref.restype=C.c_uint32;results=[]
     for x,want in zip(xs,wants):
      fuel=C.c_uint(10000);status=C.c_uint();got=native.ce_checked(x,C.byref(fuel),C.byref(status));results.append((x,want,got,(native.ref(x) if want is not None else None),status.value,fuel.value))
     check(row['id']+' independent Python/x86/GCC',all(r[4]==1 if r[1] is None else (r[1]==r[2]==r[3] and r[4]==0) for r in results),results)
     fuel=C.c_uint(1);status=C.c_uint();native.ce_checked(2,C.byref(fuel),C.byref(status));check(row['id']+' emitted fuel rejection',status.value==1 and fuel.value==0,{'status':status.value,'fuel':fuel.value})
    for target,cpu,machine,hostile in [(1,'cortex-m4','mps2-an386','hostile_thumb.S'),(2,'cortex-m0','microbit','hostile_thumb1.S')]:
     assert cc.ce_emit(C.byref(p),target,text,len(text));asm.write_bytes(text.value)
     checks=''.join(('fuel=10000;status=0;(void)ce_checked(%uu,&fuel,&status);if(status!=1)stop(1);'%x) if want is None else ('fuel=10000;status=0;if(ce_checked(%uu,&fuel,&status)!=%uu||status)stop(1);'%(x,want)) for x,want in zip(xs,wants))
     (w/'start.c').write_text('''typedef unsigned U;extern U ce_checked(U,U*,U*);extern U stack_top;
 static void semi(U op,const void*p){register U r0 __asm__("r0")=op;register const void*r1 __asm__("r1")=p;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");}
 static void stop(U n){U a[2]={0x20026,n};semi(4,n?"B06_EXT_THUMB_FAIL\\n":"B06_EXT_THUMB_PASS\\n");semi(0x20,a);for(;;){}}
 void reset(void){U fuel,status;'''+checks+'''fuel=1;status=0;(void)ce_checked(2,&fuel,&status);if(!status||fuel)stop(2);stop(0);}static void fault(void){stop(3);}
 __attribute__((section(".vectors"),used))const void*v[]={&stack_top,reset,fault,fault,fault,fault,fault,0,0,0,0,fault,fault,0,fault,fault};''')
     (w/'link.ld').write_text('ENTRY(reset)\nMEMORY { FLASH (rx) : ORIGIN = 0, LENGTH = 1M RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 16K }\nSECTIONS { .vectors : { KEEP(*(.vectors)) } > FLASH .text : { *(.text*) *(.rodata*) } > FLASH .data : { *(.data*) } > RAM AT>FLASH .bss : { *(.bss*) *(COMMON) } > RAM stack_top = ORIGIN(RAM)+LENGTH(RAM); }')
     elf=w/'target.elf'
     if run(['arm-none-eabi-gcc','-mcpu='+cpu,'-mthumb','-std=c17','-O1','-ffreestanding','-fno-builtin','-nostdlib','-Wl,-T,'+str(w/'link.ld'),asm,lab/hostile,w/'start.c','-o',elf],row['id']+' '+cpu+' Thumb link')['passed']:
      r=run(['arm-none-eabi-nm','-u',elf],row['id']+' helper audit');check(row['id']+' no undefined helpers',r['passed'] and not r.get('stdout','').strip(),r.get('stdout'))
      run(['qemu-system-arm','-M',machine,'-cpu',cpu,'-nographic','-monitor','none','-serial','none','-nic','none','-semihosting-config','enable=on,target=native','-kernel',elf],row['id']+' '+cpu+' Thumb execution',marker='B06_EXT_THUMB_PASS')
  fuzz=w/'fuzz'
  if run(['clang','-std=c17','-O1','-g','-fsanitize=fuzzer,address,undefined','-I',lab,ref,serial,thumb1,lab/'fuzz.c','-o',fuzz],'parser fuzz build')['passed']:
   corpus=w/'corpus';corpus.mkdir()
   for i,row in enumerate(json.loads((lab/'cases.json').read_text())['cases']):(corpus/str(i)).write_text(row['source'])
   run([fuzz,'-runs=20000','-seed=176001','-max_len=2048',corpus],'bounded parser semantic fuzz',marker='DONE')
 passed=all(r['passed'] for r in records);report={'passed':passed,'recorded_at':datetime.datetime.now(datetime.timezone.utc).isoformat(),'input_sha256':pins,'checks':records,'scope':'Author bounded extension implementation, not general ISO C compiler, learner acceptance or independent review','limitations':['Typed u32/i32 notation with runtime checked signed overflow and checked unsigned-to-signed cast; this safety policy is stronger than ISO C and not a full C frontend','Four acyclic unary functions,16locals,8arraycells,32operanddepth,512IR instructions; fuel is execution policy','Pointer values retain implicit per-frame array provenance; no pointer escape, comparison or cross-object arithmetic','Thumb-1 and Thumb-2 are separately emitted, assembled and emulated; no physical board claim','Finite fixtures and bounded fuzz are falsification evidence, not correctness proof']};out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'passed':passed,'checks':len(records),'failures':[r['label'] for r in records if not r['passed']],'report':str(out)}));return 0 if passed else 1
if __name__=='__main__':raise SystemExit(main())
