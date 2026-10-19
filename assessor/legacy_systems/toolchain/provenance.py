"""Target search roots, offline dependencies and falsifiable provenance records."""
from pathlib import Path
import json, shutil, struct
from common import sha

def execute(c):
    c.group='target-roots';d=c.root/'foreign'
    target=d/'target-root';host=d/'host-root'
    for root,value in [(target,7),(host,99)]:
        (root/'include').mkdir(parents=True,exist_ok=True);(root/'lib').mkdir(exist_ok=True)
        (root/'include/tiny.h').write_text(f'#define SELECTED {value}\n')
        compiler='arm-none-eabi-gcc' if value==7 else 'gcc'
        c.run([compiler,'-c',d/'probe.c','-o',root/'lib/tiny.o'])
        c.run(['ar','rcs',root/'lib/libtiny.a',root/'lib/tiny.o'])
    c.write('foreign/entry.c','extern unsigned value(void);unsigned entry(void){return value();}\n')
    c.run(['arm-none-eabi-gcc','-ffreestanding','-c',d/'entry.c','-o',d/'entry.o'])
    def link(script,out,expected=0):
        return c.run(['arm-none-eabi-gcc','-nostdlib','-Wl,-e,entry','-T',script,d/'entry.o','-L'+str(target/'lib'),'-ltiny','-o',out],expected=expected,contains='tiny' if expected else None)
    objects=sha(d/'entry.o');maps={}
    for address in (0x10000,0x20000):
        path=c.write(f'foreign/map-{address}.ld',f'SECTIONS {{ . = {address}; .text : {{ *(.text*) }} .data : {{ *(.data*) }} .bss : {{ *(.bss*) }} }}\n')
        out=d/f'linked-{address}.elf';link(path,out)
        maps[str(address)]=c.run(['arm-none-eabi-nm','-n',out])
        c.require(f'{address:08x}' in maps[str(address)],'link-map placement revalidated',address)
    c.require(sha(d/'entry.o')==objects,'memory map change reuses unchanged compile artifact')
    lib=target/'lib/libtiny.a';lib.rename(target/'lib/libtiny.saved')
    link(d/'map-65536.ld',d/'missing-library.elf','nonzero')
    (target/'lib/libtiny.saved').rename(lib)
    manifest=json.loads((d/'manifest.json').read_text());manifest.update(linker_scripts={p.name:sha(p) for p in d.glob('map-*.ld')},target_libraries={str(lib):sha(lib)},maps=maps)
    c.write('foreign/manifest.json',json.dumps(manifest,indent=2))
    c.observe('T07.09.a','Actual target link scripts/library inventory and missing target library rejection; no host substitution',['foreign/manifest.json','missing-library command'])
    c.observe('T07.09.a','Changing only memory map changes linked placement, not input object bytes',['foreign/map-*.ld and nm'],True)
    # A CMake find-only project verifies root policy including nested packages.
    for root,label in [(target,'target'),(host,'host')]:
        for package in ('Outer','Inner'):
            directory=root/'lib/cmake'/package;directory.mkdir(parents=True)
            body='find_package(Inner CONFIG REQUIRED)\n' if package=='Outer' else ''
            body+=f'set({package}_ORIGIN "{label}")\n'
            (directory/(package+'Config.cmake')).write_text(body)
    find=d/'finder';find.mkdir()
    c.write('foreign/finder/CMakeLists.txt',f'''cmake_minimum_required(VERSION 3.20)
project(RootPolicy NONE)
set(CMAKE_FIND_ROOT_PATH "{target}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
find_program(GENERATOR hostgen PATHS "{d}" NO_DEFAULT_PATH REQUIRED)
find_path(HEADER tiny.h PATHS /include REQUIRED)
find_library(LIBRARY tiny PATHS /lib REQUIRED)
find_package(Outer CONFIG REQUIRED)
if(NOT HEADER STREQUAL "{target/'include'}" OR NOT LIBRARY STREQUAL "{lib}" OR NOT GENERATOR STREQUAL "{d/'hostgen'}" OR NOT Inner_ORIGIN STREQUAL "target" OR NOT Outer_ORIGIN STREQUAL "target")
 message(FATAL_ERROR "ROOT_REJECT:contamination")
endif()
file(WRITE "${{CMAKE_BINARY_DIR}}/resolved.txt" "${{GENERATOR}}\\n${{HEADER}}\\n${{LIBRARY}}\\n${{Outer_DIR}}\\n${{Inner_DIR}}\\n")
''')
    c.run(['cmake','-S',find,'-B',d/'found','-DCMAKE_PREFIX_PATH='+str(host)+';/'])
    c.require(str(target) in (d/'found/resolved.txt').read_text(),'resolved target root recorded')
    c.write('foreign/selection.c','#include <tiny.h>\n_Static_assert(SELECTED==7,"target required");\nint selected(void){return SELECTED;}\n')
    c.run(['arm-none-eabi-gcc','-std=c17','-I'+str(target/'include'),'-c',d/'selection.c','-o',d/'selection.o'])
    c.run(['arm-none-eabi-gcc','-std=c17','-I'+str(host/'include'),'-c',d/'selection.c','-o',d/'contaminated.o'],expected='nonzero',contains='target required')
    c.observe('T07.09.b','Host generator and same-named target header/library selected under explicit root modes; contamination diagnosed before execution',['foreign/finder/CMakeLists.txt','foreign/found/resolved.txt'])
    c.observe('T07.09.b','Nested Outer→Inner package discovery remains in target root',['foreign/found/resolved.txt'],True)
    # Keep the target compiler and consumer object unchanged; change only the
    # selected library root, then compare resolved input identities and linked code.
    second=d/'target-root-second';shutil.copytree(target,second)
    c.write('foreign/other-value.c','unsigned value(void){return 9;}\n')
    c.run(['arm-none-eabi-gcc','-c',d/'other-value.c','-o',second/'lib/tiny.o'])
    (second/'lib/libtiny.a').unlink();c.run(['arm-none-eabi-ar','rcs',second/'lib/libtiny.a',second/'lib/tiny.o'])
    c.run(['arm-none-eabi-gcc','-nostdlib','-Wl,-e,entry','-T',d/'map-65536.ld',d/'entry.o','-L'+str(second/'lib'),'-ltiny','-o',d/'second-root.elf'])
    c.require(sha(d/'entry.o')==objects and sha(second/'lib/libtiny.a')!=sha(lib),'changed root invalidates selected library receipt, not unchanged consumer object')
    c.require(sha(d/'second-root.elf')!=sha(d/'linked-65536.elf'),'different target root changes linked artifact')
    c.run(['arm-none-eabi-objdump','-dr',d/'second-root.elf'])
    c.observe('T07.02.b','Only selected target library root changes while compiler/consumer object stay fixed; changed dependency and image hashes invalidate old closure conclusions',['foreign second-root commands and identity controls'],True)
    # Named emulator variant: original minimal firmware, no HAL/libc/compiler helper.
    c.group='named-emulator';d=c.root/'emulator';d.mkdir()
    c.write('emulator/start.S','.syntax unified\n.cpu cortex-m3\n.thumb\n.section .vectors,"a"\n.word 0x20010000\n.word _start\n.text\n.global _start\n.thumb_func\n_start:\n bl run\n b .\n.section .note.GNU-stack,"",%progbits\n')
    c.write('emulator/main.c',r'''static unsigned call(unsigned op,const void *arg){register unsigned r0 __asm__("r0")=op;register const void *r1 __asm__("r1")=arg;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");return r0;}
void run(void){static const char text[]="TOOLCHAIN_EMULATOR_PASS\n";unsigned exit_args[2]={0x20026,0};call(4,text);call(0x20,exit_args);for(;;){}}
''')
    c.write('emulator/image.ld','ENTRY(_start)\nSECTIONS { . = 0; .vectors : { *(.vectors) } .text : { *(.text*) *(.rodata*) } .data : { *(.data*) } .bss : { *(.bss*) } }\n')
    c.run(['arm-none-eabi-gcc','-std=c17','-mcpu=cortex-m3','-mthumb','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-nostdlib','-T',d/'image.ld',d/'start.S',d/'main.c','-o',d/'image.elf'])
    c.require(not c.run(['arm-none-eabi-nm','-u',d/'image.elf']).strip(),'emulated image has no unresolved runtime helpers')
    c.run(['arm-none-eabi-readelf','-l','-d',d/'image.elf'])
    c.run(['qemu-system-arm','--version'])
    c.run(['qemu-system-arm','-M','lm3s6965evb','-cpu','cortex-m3','-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],contains='TOOLCHAIN_EMULATOR_PASS',timeout=20)
    c.observe('T07.07.c','Explicit QEMU lm3s6965evb Cortex-M3 run, original semihost output/exit and final-image closure',['emulator commands and source'],True,
        ['Semihost calls are emulator/host services; not admitted in a board-runtime closure or evidence of physical peripherals.'])

    c.group='offline-dependencies';d=c.root/'dependencies';d.mkdir()
    files={'tiny.c':'int support(int);int tiny(int x){return support(x)+3;}\n','support.c':'int support(int x){return x+5;}\n',
        'tiny.h':'int tiny(int);\n','CMakeLists.txt':'cmake_minimum_required(VERSION 3.20)\nproject(Dep C)\nadd_library(tiny STATIC tiny.c support.c)\ntarget_include_directories(tiny PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})\n'}
    original=d/'source';original.mkdir()
    for name,text in files.items():(original/name).write_text(text)
    manifest={'version':'1.0','files':{name:sha(original/name) for name in files}}
    c.write('dependencies/manifest.json',json.dumps(manifest,indent=2))
    def verify(directory,m=manifest):
        return all((directory/name).is_file() and sha(directory/name)==value for name,value in m['files'].items())
    c.require(verify(original),'complete dependency content manifest')
    patched=d/'patched';shutil.copytree(original,patched);(patched/'tiny.c').write_text(files['tiny.c']+'/* same version; different bytes */\n')
    c.require(not verify(patched),'same-version patched source rejected')
    renamed=dict(manifest,version='9.9');c.require(renamed['files']==manifest['files'],'version rename does not change content identity')
    missing=d/'missing';shutil.copytree(original,missing);(missing/'support.c').unlink();c.require(not verify(missing),'missing transitive dependency rejected')
    # Generated inputs include both their generator/configuration and output.
    (original/'gen.py').write_text('from pathlib import Path\nPath("generated.h").write_text("#define EXTRA "+Path("config.txt").read_text())\n')
    (original/'config.txt').write_text('0\n');c.run(['python3','gen.py'],original)
    extended={'version':'1.0','files':{p.name:sha(p) for p in original.iterdir() if p.is_file()}}
    c.write('dependencies/extended-manifest.json',json.dumps(extended,indent=2));(original/'config.txt').write_text('1\n')
    c.require(not verify(original,extended),'generator configuration invalidates extended receipt');(original/'config.txt').write_text('0\n')
    consumer='#include "tiny.h"\nint main(void){return tiny(4)!=12;}\n';c.write('dependencies/main.c',consumer)
    for method in ('prefix','vendored','cache'):
        local=d/method;shutil.copytree(original,local)
        c.require(verify(local),'dependency acquisition source identity',method)
        if method=='prefix':
            c.run(['gcc','-c','tiny.c','support.c'],local);c.run(['ar','rcs','libtiny.a','tiny.o','support.o'],local)
            c.run(['gcc','-I'+str(local),d/'main.c',local/'libtiny.a','-o',d/(method+'-app')]);c.run([d/(method+'-app')])
        else:
            project=d/(method+'-consumer');project.mkdir()
            acquisition='add_subdirectory("'+str(local)+'" dependency)' if method=='vendored' else '''include(FetchContent)
set(FETCHCONTENT_FULLY_DISCONNECTED ON CACHE BOOL "" FORCE)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "" FORCE)
FetchContent_Declare(tiny SOURCE_DIR "'''+str(local)+'''")
FetchContent_MakeAvailable(tiny)'''
            (project/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(Consumer C)\n'+acquisition+'\nadd_executable(app "'+str(d/'main.c')+'")\ntarget_link_libraries(app PRIVATE tiny)\n')
            c.run(['cmake','-S',project,'-B',project/'build','-G','Ninja']);c.run(['cmake','--build',project/'build']);c.run([project/'build/app'])
    (d/'cache/support.c').unlink();c.require(not verify(d/'cache'),'offline missing cache rejects before build')
    c.run(['cmake','-S',d/'cache-consumer','-B',d/'cache-fresh','-G','Ninja'],expected='nonzero',contains='support.c')
    for leaf,claim in [('a','Corrupt/missing files rejected; renamed version label separated from content identity'),
        ('b','Same API actually built from prefix archive, vendored source and prepopulated FetchContent source'),
        ('c','Explicit local-source disconnected clean build; missing cache input fails without network fallback')]:
        c.observe('T07.10.'+leaf,claim,['dependencies manifests and commands'])
    c.observe('T07.10.a','Generator/config/output all pinned; configuration change rejected',['dependencies/extended-manifest.json'],True)
    c.observe('T07.10.b','Same-version patched vendored bytes do not pass original manifest',['dependencies/patched'],True)
    c.observe('T07.10.c','Transitive Support change/absence violates complete cache manifest',['dependencies/missing and cache controls'],True)

    c.group='matrix-verdicts';d=c.root/'verdicts';d.mkdir()
    c.write('verdicts/positive.c','unsigned add(unsigned a,unsigned b){return a+b;}int main(void){return add(2,3)!=5;}\n')
    c.write('verdicts/failure.c','#include <stdio.h>\n#include "expected.h"\nint main(void){if(EXPECTED!=5){fputs("CHECK:wrong expected value\\n",stderr);return 1;}return 0;}\n')
    c.write('verdicts/expected.h','#define EXPECTED 6\n')
    for cc in ('gcc','clang'):
        for opt in ('-O0','-O2'):
            out=d/(cc+opt);c.run([cc,'-std=c17',opt,d/'positive.c','-o',out]);c.run([out])
            c.run([cc,'--target=arm-none-eabi','-mcpu=cortex-m4','-ffreestanding',opt,'-c',d/'positive.c','-o',d/(cc+opt+'.o')] if cc=='clang' else ['arm-none-eabi-gcc','-mcpu=cortex-m4','-ffreestanding',opt,'-c',d/'positive.c','-o',d/(cc+opt+'.o')])
    c.run(['gcc','-std=c17',d/'failure.c','-o',d/'failure']);c.run([d/'failure'],expected=1,contains='CHECK:wrong expected value')
    receipt={'source':sha(d/'failure.c'),'header':sha(d/'expected.h'),'binary':sha(d/'failure'),'argv':['gcc','-std=c17','failure.c','-o','failure'],'environment':{'LC_ALL':'C'},'expected_stderr':'CHECK:wrong expected value\n','expected_exit':1}
    c.write('verdicts/failure-receipt.json',json.dumps(receipt,indent=2));fresh=d/'fresh';fresh.mkdir()
    for p in ('failure.c','expected.h'):shutil.copy2(d/p,fresh/p)
    c.run(receipt['argv'],fresh);c.run([fresh/'failure'],expected=1,contains=receipt['expected_stderr'])
    c.require(sha(fresh/'failure')==receipt['binary'],'fresh local failure reproduces same binary under declared flags')
    c.write('verdicts/expected.h','#define EXPECTED 5\n');c.require(sha(d/'expected.h')!=receipt['header'],'header-only change rejects old binary receipt')
    verdicts={'native-unit':'PASS','two-component-link':'PASS:packages consumer evidence','expected-diagnostic':'PASS:diagnostics valid-control evidence',
        'foreign-compilation':'PASS:object inspection only','foreign-native-execution':'UNAVAILABLE:non-native artifact intentionally not launched',
        'emulator':'PASS:separate named Cortex-M3 image','physical':'UNAVAILABLE:no board observation',
        'broken-compiler':'FAIL:launch-unavailable, never diagnostic pass'}
    c.write('verdicts/registration.json',json.dumps(verdicts,indent=2))
    c.observe('T07.11.a','Unit/integration/diagnostic/target registration separates execution scope and broken-compiler failure',['verdicts/registration.json','diagnostics controls'])
    c.observe('T07.11.a','Named emulator result is separate from unavailable physical cell',['emulator actual execution'],True)
    c.observe('T07.11.b','GCC/Clang O0/O2 native cells execute; Arm GCC/Clang cells compile only under explicit target contracts',['verdicts commands'])
    c.observe('T07.11.b','LTO cells actually rebuild and inspect final closure; no native result fills foreign execution',['profiles LTO commands'],True)
    c.observe('T07.11.c','Exact source/header/argv/binary/diagnostic receipt reproduces marked failure in fresh directory',['verdicts/failure-receipt.json'])
    c.observe('T07.11.c','Header-only changed bytes invalidate prior binary receipt',['verdicts header hash control'],True)

    c.group='reproducibility';d=c.root/'repro';d.mkdir()
    source='#include <stdio.h>\n#include "stamp.h"\nint main(void){printf("19|%s|%s\\n",__FILE__,STAMP);return 0;}\n'
    artifacts={}
    for name,stamp in [('root-one','1001'),('root-two','1002')]:
        folder=d/name;folder.mkdir();(folder/'main.c').write_text(source);(folder/'stamp.h').write_text('#define STAMP "'+stamp+'"\n')
        for policy,flags in [('raw',[]),('path',['-ffile-prefix-map='+str(folder)+'=/src']),('normalized',['-ffile-prefix-map='+str(folder)+'=/src'])]:
            if policy=='normalized':(folder/'stamp.h').write_text('#define STAMP "1000"\n')
            out=folder/policy;c.run(['gcc','-std=c17','-g','-O2',*flags,folder/'main.c','-Wl,--build-id=sha1','-o',out],folder)
            result=c.run([out]);c.require(result.split('|')[0]=='19','declared computation result',name+'-'+policy)
            c.run(['readelf','-n',out]);c.run(['readelf','--debug-dump=info',out])
            text=folder/(policy+'.text');debug=folder/(policy+'.debug');c.run(['objcopy','--dump-section','.text='+str(text),'--dump-section','.debug_info='+str(debug),out])
            artifacts[name+'-'+policy]={'binary':sha(out),'text':sha(text),'debug':sha(debug),'output':result}
    a,b=artifacts['root-one-raw'],artifacts['root-two-raw'];c.require(a['binary']!=b['binary'],'raw roots/time differ')
    c.require(artifacts['root-one-path']['binary']!=artifacts['root-two-path']['binary'],'path mapping alone cannot normalize generated timestamp')
    c.require(artifacts['root-one-normalized']['binary']==artifacts['root-two-normalized']['binary'],'declared identical normalized inputs reproduce whole artifact')
    c.write('repro/comparison.json',json.dumps(artifacts,indent=2))
    # Archive order changes representation and can change semantics for duplicate symbols.
    c.write('repro/a.c','int choose(void){return 1;}\n');c.write('repro/b.c','int choose(void){return 2;}\n')
    c.write('repro/use.c','int choose(void);int main(void){return choose();}\n')
    c.run(['gcc','-c','a.c','b.c'],d)
    for order,expected in [('ab',1),('ba',2)]:
        c.run(['ar','rcsD',order+'.a',*(x+'.o' for x in order)],d)
        c.run(['gcc','use.c',order+'.a','-o',order],d);c.run([d/order],expected=expected)
    c.require(sha(d/'ab.a')!=sha(d/'ba.a'),'deterministic member timestamps do not erase member order')
    # Nondeterministic table ordering is a semantic failure, not debug noise.
    c.write('repro/table.c','#include "table.h"\nint main(void){for(unsigned i=0;i<2;i++)if(keys[i]==7)return values[i];return 3;}\n')
    for name,values,expected in [('first','1,2',1),('reordered','2,1',2)]:
        c.write('repro/table.h','static const int keys[]={7,7};static const int values[]={'+values+'};\n')
        c.run(['gcc','table.c','-o',name],d);c.run([d/name],expected=expected)
    provenance={'compiler':c.run(['gcc','--version']).splitlines()[0],'linker':c.run(['ld','--version']).splitlines()[0],
        'effective_argv':['gcc','-std=c17','-g','-O2','-ffile-prefix-map=<root>=/src','<root>/main.c','-Wl,--build-id=sha1','-o','<root>/normalized'],
        'sources':{str(p.relative_to(d)):sha(p) for p in (d/'root-one/main.c',d/'root-one/stamp.h')},
        'driver_inputs':json.loads((c.root/'stages/startup-inputs.json').read_text()),'output':sha(d/'root-one/normalized')}
    c.write('repro/provenance.json',json.dumps(provenance,indent=2));(d/'root-one/stamp.h').write_text('#define STAMP "changed"\n')
    c.require(any(sha(d/p)!=h for p,h in provenance['sources'].items()),'changed input rejects provenance reuse')
    c.run(['clang','-std=c17','-g','-O2',d/'root-two/main.c','-o',d/'clang-output'])
    c.require(c.run([d/'clang-output']).split('|')[0]=='19','different compiler same declared computation')
    c.require(sha(d/'clang-output')!=provenance['output'],'cross-compiler behavior does not imply byte identity')
    c.observe('T07.12.a','Path, generated-time, debug and build-ID contributions retained; normalized input policy reproduces exactly',['repro/comparison.json'])
    c.observe('T07.12.a','Deterministic archives retain ordering; duplicate definition extraction changes actual exit1/2',['repro archive commands'],True)
    c.observe('T07.12.b','Whole-file, text, debug and declared computation are separately compared; output metadata remains observable',['repro/comparison.json'])
    c.observe('T07.12.b','Generated table order changes first-match result; normalization would hide actual semantic defect',['repro/table.c controls'],True)
    c.observe('T07.12.c','Provenance binds sources/generated header/tool identity/flags/driver inputs/output and rejects changed input',['repro/provenance.json'])
    c.observe('T07.12.c','Different GCC/Clang implementations agree on declared computation but not binary identity',['repro clang comparison'],True,
        ['This is a compiler-family change, not a second GCC release. The learner must redo the release-specific comparison if claiming that narrower variant.'])
