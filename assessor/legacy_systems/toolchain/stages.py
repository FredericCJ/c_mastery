"""T07 stages, diagnostics, configuration and runtime-closure investigations."""
from pathlib import Path
import json, os, re, shlex
from common import sha

SUM = r'''
#include <stdint.h>
#include <stdio.h>
#ifndef MODE
#define MODE 0
#endif
struct block { unsigned char bytes[1024]; };
struct block copy(struct block x) { return x; }
uint32_t sum(const uint32_t *p, unsigned n) {
    uint32_t v=0; for(unsigned i=0;i<n;i++) v+=p[i]; return v;
}
int main(void) {
    uint32_t a[16]; for(unsigned i=0;i<16;i++) a[i]=UINT32_MAX-i;
    for(unsigned n=0;n<=16;n++) printf("%u %u\n",n,sum(a,n));
    if(sum(a,1)!=UINT32_MAX) return 3; /* not assert: active with NDEBUG */
    return MODE;
}
'''

def execute(c):
    c.group='stages'
    d=c.root/'stages';d.mkdir()
    c.write('stages/cfg.h','#define BIAS 7\n')
    c.write('stages/step.c','#include "cfg.h"\nint step(int x){return 3*x+BIAS;}\n')
    c.write('stages/main.c','int step(int);int main(void){return step(4)!=19;}\n')
    for cc in ('gcc','clang'):
        c.run([cc,'--version'])
        c.run([cc,'-std=c17','-E','step.c','-o',cc+'.i'],d)
        c.run([cc,'-std=c17','-S',cc+'.i','-o',cc+'.s'],d)
        c.run([cc,'-c',cc+'.s','-o',cc+'.o'],d)
        c.run([cc,'-std=c17','-c','main.c','-o','main.o'],d)
        c.run([cc,cc+'.o','main.o','-o',cc+'.exe'],d)
        c.run([d/(cc+'.exe')])
        c.run([cc,'main.o','-o','missing'],d,expected='nonzero',contains='step')
    c.observe('T07.01.a','Separate preprocessor/compiler/assembler/linker products and missing-symbol link rejection',['stages/*.i','stages/*.s','stages/*.o'])
    # Freeze the preprocessed input, then change only the consumed header.
    c.write('stages/cfg.h','#define BIAS 9\n')
    c.run(['gcc','gcc.i','main.o','-o','frozen'],d);c.run([d/'frozen'])
    c.run(['gcc','-E','step.c','-o','fresh.i'],d)
    c.require('3*x+7' in (d/'gcc.i').read_text() and '3*x+9' in (d/'fresh.i').read_text(),'first difference is preprocessed macro')
    c.run(['gcc','fresh.i','main.o','-o','fresh'],d);c.run([d/'fresh'],expected=1)
    c.observe('T07.01.b','Frozen .i retains old generated macro despite fresh source/header inputs',['stages/gcc.i','stages/fresh.i'])
    c.write('stages/select.txt','7\n')
    c.write('stages/select.py','from pathlib import Path\nPath("selected.h").write_text("#define BIAS "+Path("select.txt").read_text())\n')
    c.write('stages/cfg.h','#include "selected.h"\n');c.run(['python3','select.py'],d)
    c.run(['gcc','-E','step.c','-o','selected.i'],d)
    c.write('stages/select.txt','9\n');c.run(['python3','select.py'],d)
    os.utime(d/'step.c',None);os.utime(d/'main.c',None)
    c.run(['gcc','selected.i','main.o','-o','selected-frozen'],d);c.run([d/'selected-frozen'])
    c.run(['gcc','step.c','main.o','-o','selected-fresh'],d);c.run([d/'selected-fresh'],expected=1)
    c.observe('T07.01.b','Generated selected header changed7→9 and both C timestamps refreshed; retained .i still computes19 while fresh source computes21',['stages/select.py','stages/selected.i','actual frozen/fresh exits'],True)
    c.write('stages/cfg.h','#define BIAS 7\n')
    # -### prints the actual driver recipe; replay each emitted executable argv.
    for mode,flags in [('pie',['-fPIE','-pie']),('static',['-static'])]:
        out=c.run(['gcc','-###',*flags,'gcc.o','main.o','-o',mode],d)
        commands=[]
        for line in out.splitlines():
            if line.startswith(' ') and not line.startswith(' ('):
                words=shlex.split(line)
                if words and Path(words[0]).is_file():commands.append(words)
        c.require(bool(commands),'driver emits replayable tool argv',mode)
        for args in commands:c.run(args,d)
        c.run([d/mode]);c.run(['readelf','-SW',mode],d)
        if mode=='pie':c.require('INTERP' in c.run(['readelf','-l',mode],d),'PIE loader present')
        else:c.require('INTERP' not in c.run(['readelf','-l',mode],d),'static loader absent')
    c.observe('T07.01.c','Actual GCC link subprocess reconstructed for PIE and static, with selected startup/library inputs',['driver argv and readelf commands'])
    c.observe('T07.01.c','Static changes runtime closure; PIE changes loader and image assumptions',['driver argv and readelf commands'],True)
    c.write('stages/entry.S','.global _start\n.text\n_start:\n andq $-16,%rsp\n movl $4,%edi\n call step\n cmpl $19,%eax\n setne %dil\n movzbl %dil,%edi\n movl $60,%eax\n syscall\n.section .note.GNU-stack,"",@progbits\n')
    c.run(['gcc','-nostdlib','-no-pie','entry.S','gcc.o','-o','entry'],d);c.run([d/'entry'])
    c.require(not c.run(['nm','-u','entry'],d).strip(),'assembly entry runtime closure')
    c.observe('T07.01.a','Assembly _start establishes call alignment, passes argument and exits through Linux syscall; no CRT supplied',['stages/entry.S'],True)
    startup={}
    for item in ('crt1.o','Scrt1.o','crti.o','crtbeginS.o','crtendS.o','crtn.o','libgcc.a','libc.so'):
        path=Path(c.run(['gcc','-print-file-name='+item],d).strip())
        c.require(path.is_file(),'selected driver input exists',item);startup[str(path)]=sha(path)
    c.run(['gcc','-print-search-dirs'],d);c.run(['gcc','-v','gcc.o','main.o','-o','verbose'],d)
    c.run(['gcc','gcc.o','main.o','-L'+str(d/'absent'),'-lcmastery_missing_2026','-o','missing-lib'],d,expected='nonzero',contains='cmastery_missing_2026')
    c.write('stages/startup-inputs.json',json.dumps(startup,indent=2))
    c.observe('T07.02.b','Resolved startup/library hashes, search paths and specific missing-local-library rejection',['stages/startup-inputs.json'])

    c.group='diagnostics';d=c.root/'diagnostics';d.mkdir()
    fixtures={'valid':'unsigned add(unsigned a,unsigned b){return a+b;}\n',
      'warning':'int f(int unused){return 0;}\n',
      'constraint':'void f(void){const int x=1;x=2;}\n',
      'link':'int absent(void);int main(void){return absent();}\n',
      'shim':'#pragma GCC diagnostic push\n#pragma GCC diagnostic ignored "-Wunused-parameter"\nint shim(int shim_arg){return 0;}\n#pragma GCC diagnostic pop\nint outer(int outer_arg){return 0;}\n'}
    for name,source in fixtures.items():c.write('diagnostics/'+name+'.c',source)
    for cc in ('gcc','clang'):
        base=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic']
        c.run([*base,'-Werror','-c','valid.c','-o','valid.o'],d)
        c.run([*base,'-c','warning.c','-o','warning.o'],d,contains='unused parameter')
        c.run([*base,'-c','constraint.c','-o','constraint.o'],d,expected='nonzero',contains='const' if cc=='clang' else 'read-only')
        c.run([*base,'-include','missing_header_for_control.h','-c','constraint.c'],d,expected='nonzero',contains='missing_header_for_control.h')
        c.run([str(d/'nonexistent-compiler'),'-c','constraint.c'],d,expected='launch-unavailable')
        c.run([*base,'-c','link.c','-o','link.o'],d)
        c.run([cc,'link.o','-o','link'],d,expected='nonzero',contains='absent')
        msg=c.run([*base,'-c','shim.c','-o','shim.o'],d,contains='outer_arg')
        c.require('unused parameter' in msg and 'shim_arg' not in msg,'push/pop warning stays local',cc)
        c.run([*base,'-Wno-unused-parameter','-c','warning.c','-o','local.o'],d)
        c.run([*base,'-c','warning.c','-o','unrelated.o'],d,contains='unused parameter')
    for leaf in ('a','b','c'):
        c.observe('T07.03.'+leaf,{'a':'Tool-specific warning and C17 constraint are separate; signed-overflow reasoning is assessor-only',
            'b':'Valid control, language diagnostic, missing header, missing compiler and link-only failure classified distinctly',
            'c':'Region and TU-local suppressions preserve unrelated diagnostics'}[leaf],['diagnostics commands and complete diagnostics'])
        c.observe('T07.03.'+leaf,{'a':'GCC and Clang diagnostic classes checked without depending on complete prose',
            'b':'Compile-only link-negative is accepted as a translation but rejected at final link',
            'c':'Target-local suppression cannot suppress a separately compiled TU'}[leaf],['diagnostics commands'],True)

    c.group='profiles';d=c.root/'profiles';d.mkdir();c.write('profiles/sum.c',SUM)
    oracle=''.join(f'{n} {sum((2**32-1-i) for i in range(n)) % 2**32}\n' for n in range(17))
    profiles=[('debug',['-O0','-g']),('release',['-O2','-DNDEBUG']),('size',['-Os','-DNDEBUG']),
        ('sanitize',['-O1','-g','-fsanitize=address,undefined','-fno-pie','-no-pie']),
        ('lto',['-O2','-flto']),('coverage',['-O0','--coverage'])]
    for cc in ('gcc','clang'):
        for mode,flags in profiles:
            directory=d/(cc+'-'+mode);directory.mkdir()
            linker=['-fuse-ld='+c.clang_linker] if cc=='clang' and mode=='lto' else []
            c.run([cc,'-std=c17',*flags,'-c',d/'sum.c','-o','sum.o'],directory)
            c.run([cc,*flags,*linker,'sum.o','-o','sum'],directory)
            c.require(c.run([directory/'sum'])==oracle,'independent arbitrary-precision modulo oracle',cc+'-'+mode)
            # Clang's LTO file is bitcode; llvm-nm is the matching object reader.
            reader=c.llvm_nm if cc=='clang' and mode=='lto' else 'nm'
            c.run([reader,'-u','sum.o'],directory);c.run(['nm','-u','sum'],directory)
            c.run(['readelf','-d','sum'],directory);c.run(['size','sum'],directory)
            c.run(['objdump','-dr','sum'],directory)
            # An always-active check must still fail with NDEBUG.
            c.run([cc,'-std=c17',*flags,*linker,'-DMODE=7',d/'sum.c','-o','negative'],directory)
            c.run([directory/'negative'],expected=7)
    c.observe('T07.04.a','GCC/Clang O0/O2/Os sums n=0..16 agree with independent integer oracle; disassembly and size retained',['profiles commands'])
    c.observe('T07.04.b','Six isolated effective-flag configurations, active NDEBUG control, host-only coverage/sanitizer dependencies',['profiles commands'])
    c.observe('T07.04.b','Coverage links its own host runtime; target closure is not inferred',['coverage profile nm/readelf'],True)
    c.observe('T07.04.c','Actual sum+1024-byte copy object/final closure for O0/O2/Os/LTO/sanitizer/coverage; bitcode reader explicit',['profiles object/final nm/readelf'])
    # Stack protector can introduce helpers even when the source includes no library.
    c.write('profiles/guard.c','extern void consume(char*);void guarded(void){char b[32];consume(b);}\n')
    c.run(['gcc','-std=c17','-O2','-fstack-protector-all','-c','guard.c','-o','guard.o'],d)
    symbols=c.run(['nm','-u','guard.o'],d);c.require('__stack_chk_fail' in symbols,'stack protection creates support dependency')
    c.run(['gcc','-nostdlib','-Wl,-e,guarded','guard.o','-o','unsupported'],d,expected='nonzero',contains='__stack_chk_fail')
    c.observe('T07.04.c','Custom-entry stack protection requires support symbol and initialized guard contract; bare link refuses missing support',['profiles/guard.c'],True)
    c.write('profiles/restrict.c',r'''#include <stdint.h>
static void increment(uint32_t *restrict out,const uint32_t *restrict in,unsigned n){for(unsigned i=0;i<n;i++)out[i]=in[i]+1u;}
int main(void){uint32_t in[4]={0,1,UINT32_MAX,7},out[4]={0};increment(out,in,4);return out[0]!=1||out[1]!=2||out[2]!=0||out[3]!=8;}
''')
    for cc in ('gcc','clang'):
        for mode in ('-O0','-O2'):
            out=d/(cc+mode+'-restrict');c.run([cc,'-std=c17',mode,d/'restrict.c','-o',out]);c.run([out])
    c.observe('T07.04.a','Disjoint-buffer restrict transfer executes; overlapping modifying access is analyzed separately with no portable golden output',['profiles/restrict.c','assessor/legacy_systems/toolchain/ANALYSIS.md#restrict-transfer'],True)
