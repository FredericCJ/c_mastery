"""Bounded original B06 experiments, with explicit missing-target statuses."""
from pathlib import Path
import ctypes as C,datetime,hashlib,itertools,json,random,shutil,subprocess,tempfile
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'research/v5/expansion/b06/target_validation.json'
class Ins(C.Structure):_fields_=[('op',C.c_uint),('value',C.c_uint32)]
class Journal(C.Structure):_fields_=[('seq',C.c_uint32),('value',C.c_uint32),('check',C.c_uint32),('commit',C.c_uint32)]
class Request(C.Structure):_fields_=[('submitted',C.c_uint),('completed',C.c_uint),('cancelled',C.c_uint),('released',C.c_uint)]
def main():
    records=[];gaps=[];pins={}
    def pin(path):pins[path.relative_to(ROOT).as_posix()]=hashlib.sha256(path.read_bytes()).hexdigest()
    def run(command,kind,expected=0,marker=None,seconds=60,nonzero=False):
        try:
            r=subprocess.run(list(map(str,command)),cwd=ROOT,text=True,capture_output=True,timeout=seconds)
            ok=(r.returncode!=0 if nonzero else r.returncode==expected) and (marker is None or marker in r.stdout+r.stderr)
            row=dict(kind=kind,command=list(map(str,command)),returncode=r.returncode,stdout=r.stdout,stderr=r.stderr,passed=ok,expected_nonzero=nonzero,expected_returncode=expected,required_marker=marker)
        except (OSError,subprocess.TimeoutExpired) as exc:row=dict(kind=kind,command=list(map(str,command)),passed=False,error=str(exc))
        records.append(row);return row
    def check(kind,passed,details):records.append(dict(kind=kind,passed=bool(passed),details=details))
    with tempfile.TemporaryDirectory(prefix='cmastery-b06-experiments-') as tmp:
        work=Path(tmp)
        def core(family):
            source=ROOT/'assessor'/family/'core.c';header=ROOT/'labs'/family/'include/core.h';pin(source);pin(header)
            lib=work/(family+'.so')
            if not run(['gcc','-std=c17','-O1','-fPIC','-shared','-I',header.parent,source,'-lm','-o',lib],family+' shared core')['passed']:return None
            return C.CDLL(str(lib))
        # Original local OS observations; io_uring setup outcome is recorded, not
        # converted into an operation/cancellation success claim.
        source=ROOT/'labs/linux_io_depth/targets/observe.c';pin(source)
        if run(['gcc','-std=c17','-Wall','-Wextra','-O2',source,'-o',work/'io'],'linux probe build')['passed']:
            run([work/'io'],'local epoll/eventfd/timerfd/signalfd/fsync/sendfile',0,marker='FSYNC file+directory')
        gaps.append('io_uring actual submission/cancellation and splice under adversarial schedules remain unexecuted; setup probe is narrower.')
        # Safe bounded format witness and three isolated original UB programs.
        source=ROOT/'labs/memory_defense/targets/format_target.c';pin(source)
        if run(['clang','-std=c17','-O1',source,'-o',work/'format'],'format target build')['passed']:
            run([work/'format','flawed'],'format integrity witness',marker='sentinel=3 output=abc')
            run([work/'format','repaired'],'format repair',marker='sentinel=91 output=abc%n')
        source=ROOT/'labs/memory_defense/targets/heap_target.c';pin(source)
        if run(['clang','-std=c17','-O1','-g','-fsanitize=address','-fno-omit-frame-pointer',source,'-o',work/'heap'],'original heap target build')['passed']:
            for mode,diagnostic in [('bounds','heap-buffer-overflow'),('lifetime','heap-use-after-free'),('double','double-free')]:run([work/'heap',mode],'ASan '+mode,marker=diagnostic,nonzero=True)
            run([work/'heap','repaired'],'ASan repaired owner')
        # Explicit vector comparison is independent of C17 core representation.
        header=ROOT/'labs/numeric_simd/include';reference=ROOT/'assessor/numeric_simd/core.c';vector=ROOT/'assessor/numeric_simd/targets/vector.c';test=ROOT/'labs/numeric_simd/targets/vector_test.c'
        for source in [reference,vector,test]:pin(source)
        if run(['gcc','-std=c17','-O2','-I',header,reference,vector,test,'-lm','-o',work/'vector'],'explicit vector build')['passed']:run([work/'vector'],'vector/scalar differential',marker='lengths=68')
        run(['gcc','-std=c17','-O2','-I',header,'-S',vector,'-o',work/'vector.s'],'explicit vector assembly')
        assembly=(work/'vector.s').read_text() if (work/'vector.s').exists() else ''
        check('emitted x86 packed arithmetic',any(op in assembly for op in ['paddd','vpaddd']),{'matched':[op for op in ['paddd','vpaddd','pcmpgtd'] if op in assembly],'assembly':assembly})
        if run(['gcc','-std=c17','-O2','-ffast-math','-I',header,reference,ROOT/'labs/numeric_simd/tests/tests.c','-lm','-o',work/'fast'],'changed-math build')['passed']:run([work/'fast','compensated'],'fast-math counterexample',expected=1,marker='B06_CHECK')
        gaps.append('No timing/speed advantage, fenv mode, physical SIMD target or universal constant-time claim was measured.')
        # Fuzzing semantic oracle; empty and corrupt frames are retained seeds.
        family='verification_fuzzing';target=ROOT/'labs'/family/'targets/fuzz_frame.c';pin(target)
        source=ROOT/'assessor'/family/'core.c';pin(source)
        if run(['clang','-std=c17','-O1','-g','-I',ROOT/'labs'/family/'include',source,target,'-fsanitize=fuzzer,address,undefined','-o',work/'fuzz'],'fuzz semantic target build')['passed']:
            corpus=work/'corpus';corpus.mkdir()
            for n,data in enumerate([b'',bytes([0,0]),bytes([2,3,5,6]),bytes([2,3,5,7])]):(corpus/str(n)).write_bytes(data)
            run([work/'fuzz','-runs=10000','-seed=1706','-max_len=64',corpus],'bounded semantic fuzz',marker='DONE')
        if not shutil.which('cbmc'):gaps.append('CBMC executable absent: authored bounded proof harness not executed; no proof success claimed.')
        else:run(['cbmc','--version'],'CBMC availability only');gaps.append('CBMC proof campaign remains required; version availability alone is not proof.')
        # Enumerate every event word through length six, restarting state each.
        vf=core('verification_fuzzing');states=0
        if vf:
            vf.vf_request.argtypes=[C.POINTER(Request),C.c_uint]
            good=True
            for length in range(7):
                for word in itertools.product(range(4),repeat=length):
                    state=Request(0,0,0,0);model=[0,0,0,0]
                    for event in word:
                        old=list(model);valid=False
                        if event==0 and not model[0] and not model[3]:model[0]=1;valid=True
                        elif event==1 and model[0] and not model[1] and not model[2] and not model[3]:model[2]=1;valid=True
                        elif event==2 and model[0] and not model[1] and not model[3]:model[1]=1;valid=True
                        elif event==3 and model[1] and not model[3]:model[3]=1;valid=True
                        result=vf.vf_request(C.byref(state),event)
                        observed=[state.submitted,state.completed,state.cancelled,state.released]
                        if result!=valid or observed!=model:good=False;check('schedule divergence',False,dict(word=word,event=event,before=old,expected=model,observed=observed));break
                    states+=1
                    if not good:break
                if not good:break
            check('exhaustive event words through length6',good,dict(words=states,alphabet=4,model='single-owner atomic transitions; no weak-memory or liveness claim'))
        # Byte-prefix persistence cuts, with previous valid record retained.
        lc=core('firmware_lifecycle')
        if lc:
            lc.lc_journal_select.argtypes=[C.POINTER(Journal),C.POINTER(Journal),C.POINTER(C.c_uint)]
            old=Journal(1,7,1^7^0x5a5a5a5a,0xa5);desired=Journal(2,9,2^9^0x5a5a5a5a,0xa5)
            raw=bytes(desired);results=[]
            for cut in range(17):
                new=Journal.from_buffer_copy(raw[:cut]+b'\xff'*(16-cut));selected=C.c_uint(99)
                accepted=lc.lc_journal_select(C.byref(old),C.byref(new),C.byref(selected));expected=1 if cut==16 else 0
                results.append(dict(cut=cut,accepted=accepted,selected=selected.value,expected=expected))
            check('journal byte-prefix reset cuts',all(r['accepted']==1 and r['selected']==r['expected'] for r in results),results)
        gaps.append('Synthetic byte-prefix cuts are not all flash fault patterns, trusted authentication, or physical power-loss validation.')
        # Cross-assembly and explicit undefined-reference inspection only.
        for arch,target in [('rv64','riscv64-none-elf'),('aa64','aarch64-none-elf')]:
            source=ROOT/'assessor/architecture_ports/targets'/(arch+'_context.S');pin(source);obj=work/(arch+'.o')
            if run(['clang','--target='+target,'-ffreestanding','-c',source,'-o',obj],arch+' cross assembly')['passed']:
                r=run(['nm','-u',obj],arch+' unresolved symbols');check(arch+' no undefined helpers',r['passed'] and not r.get('stdout','').strip(),r.get('stdout',''))
        gaps.extend(['RV64/AArch64 context assembly not executed; reset, hostile ABI, trap and page-table experiments remain mandatory.','Kernel teaching module not built or loaded: disposable matching guest headers and execution not available in this campaign.','No F439/H745 physical flashing, captures, update/reset or ten-interface campaign performed.'])
        compiler_experiment(core,work,run,check,pin,gaps)
    pin(Path(__file__))
    report=dict(passed=all(r['passed'] for r in records),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),checks=records,input_sha256=pins,remaining_required=gaps,reviewer='author agent; no independent human review',learner_assessment='not-assessed',physical_validation='not-run')
    OUT.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':report['passed'],'checks':len(records),'failures':[r for r in records if not r['passed']],'remaining_required':gaps,'output':str(OUT)},indent=2))
    return 0 if report['passed'] else 1
def compiler_experiment(core,work,run,check,pin,gaps):
    cc=core('subset_compiler')
    if not cc:return
    cc.cc_parse.argtypes=[C.c_char_p,C.c_size_t,C.POINTER(Ins),C.c_size_t,C.POINTER(C.c_size_t)]
    for name in ['cc_x86','cc_thumb']:getattr(cc,name).argtypes=[C.POINTER(Ins),C.c_size_t,C.c_char_p,C.c_size_t]
    cc.cc_run.argtypes=[C.POINTER(Ins),C.c_size_t,C.c_uint32,C.POINTER(C.c_uint32)]
    rng=random.Random(1760)
    def tree(depth):
        if depth==0 or rng.randrange(4)==0:return 'x' if rng.randrange(2) else rng.randrange(65536)
        return (rng.choice(['+','*']),tree(depth-1),tree(depth-1))
    def render(t,suffix=False):return '('+render(t[1],suffix)+t[0]+render(t[2],suffix)+')' if isinstance(t,tuple) else str(t)+('u' if suffix and isinstance(t,int) else '')
    def value(t,x):return ((value(t[1],x)+value(t[2],x)) if t[0]=='+' else (value(t[1],x)*value(t[2],x)))&0xffffffff if isinstance(t,tuple) else x if t=='x' else t
    trees=[('*',('+','x',65535),('*','x',17)),('+',65535,('*',65535,65535))]+[tree(3) for _ in range(6)]
    for index,t in enumerate(trees):
        source=render(t).encode();ir=(Ins*128)();used=C.c_size_t();assembly=C.create_string_buffer(4096)
        ok=cc.cc_parse(source,len(source),ir,128,C.byref(used))==1
        check('compiler parse generated tree',ok,dict(index=index,source=source.decode(),C_correspondence=render(t,True)))
        if not ok:continue
        xs=[0,1,65535,0xffffffff]+[rng.randrange(2**32) for _ in range(28)]
        expected=[value(t,x) for x in xs]
        if cc.cc_x86(ir,used.value,assembly,4096):
            asm=work/f'expr{index}.s';asm.write_bytes(assembly.value);reference=work/f'ref{index}.c'
            reference.write_text('unsigned ref(unsigned x){return '+render(t,True)+';}\n')
            lib=work/f'expr{index}.so'
            if run(['gcc','-shared','-fPIC','-O2',asm,reference,'-o',lib],'x86 emitted tree link')['passed']:
                target=C.CDLL(str(lib));target.cm_expr.argtypes=[C.c_uint32];target.cm_expr.restype=C.c_uint32;target.ref.argtypes=[C.c_uint32];target.ref.restype=C.c_uint32
                outputs=[]
                for x,want in zip(xs,expected):
                    interpreted=C.c_uint32();accepted=cc.cc_run(ir,used.value,x,C.byref(interpreted));outputs.append((x,want,interpreted.value,target.cm_expr(x),target.ref(x),accepted))
                check('Python/IR/x86/GCC differential',all(r[1]==r[2]==r[3]==r[4] and r[5]==1 for r in outputs),dict(tree=index,cases=outputs))
        else:check('x86 emission',False,dict(tree=index))
        if not (shutil.which('arm-none-eabi-gcc') and shutil.which('qemu-system-arm')):
            gaps.append('Thumb emitter execution tools unavailable');continue
        if not cc.cc_thumb(ir,used.value,assembly,4096):check('Thumb emission',False,dict(tree=index));continue
        asm=work/f'thumb{index}.S';asm.write_bytes(assembly.value)
        cases=''.join(f'if(cm_expr({x}u)!={want}u)stop(1);' for x,want in zip(xs,expected))
        startup=work/f'start{index}.c'
        startup.write_text('''typedef unsigned U;extern U cm_expr(U);extern U stack_top;
static void semi(U op,const void *p){register U r0 __asm__("r0")=op;register const void *r1 __asm__("r1")=p;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");}
static void stop(U n){U args[2]={0x20026,n};semi(4,n?"B06_THUMB_FAIL\\n":"B06_THUMB_PASS\\n");semi(0x20,args);for(;;){}}
void reset(void){'''+cases+'''stop(0);}static void fault(void){stop(2);}
__attribute__((section(".vectors"),used)) const void *vectors[]={&stack_top,reset,fault,fault,fault,fault,fault,0,0,0,0,fault,fault,0,fault,fault};
''')
        linker=work/'thumb.ld';linker.write_text('ENTRY(reset)\nMEMORY { FLASH (rx) : ORIGIN = 0, LENGTH = 1M RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K }\nSECTIONS { .vectors : { KEEP(*(.vectors)) } > FLASH .text : { *(.text*) *(.rodata*) } > FLASH .data : { *(.data*) } > RAM AT>FLASH .bss : { *(.bss*) *(COMMON) } > RAM stack_top = ORIGIN(RAM) + LENGTH(RAM); }\n')
        elf=work/f'thumb{index}.elf'
        if run(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-std=c17','-O1','-ffreestanding','-fno-builtin','-nostdlib','-Wl,-T,'+str(linker),asm,startup,'-o',elf],'Thumb emitted tree link')['passed']:
            r=run(['arm-none-eabi-nm','-u',elf],'Thumb final closure');check('Thumb final no helpers',r['passed'] and not r.get('stdout','').strip(),r.get('stdout',''))
            run(['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf],'Thumb emulator differential',marker='B06_THUMB_PASS')
    gaps.append('Compiler validates the declared expression subset only; control flow, locals, pointers, function calls and Thumb-1 remain required extensions.')
if __name__=='__main__':raise SystemExit(main())
