"""Relocatable usage requirements and actual host/foreign tool separation."""
import json, os, shutil, struct, time
from pathlib import Path
from common import sha
from graphs import GENERATOR,STEP,MAIN

def execute(c):
    c.group='packages';d=c.root/'packages';d.mkdir()
    support='''cmake_minimum_required(VERSION 3.20)
project(Support LANGUAGES C)
add_library(support support.c)
target_include_directories(support PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
install(TARGETS support EXPORT SupportTargets ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT SupportTargets NAMESPACE Support:: DESTINATION lib/cmake/Support)
install(FILES SupportConfig.cmake DESTINATION lib/cmake/Support)
'''
    tiny='''cmake_minimum_required(VERSION 3.20)
project(Tiny LANGUAGES C)
find_package(Support CONFIG REQUIRED)
add_library(tiny STATIC tiny.c)
target_include_directories(tiny PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include> PRIVATE private)
target_compile_definitions(tiny PUBLIC TINY_WIDTH=2)
target_link_libraries(tiny PRIVATE Support::support)
if(PUBLIC_TYPE)
 target_compile_definitions(tiny PUBLIC TINY_PUBLIC_TYPE=1)
 target_link_libraries(tiny PUBLIC Support::support)
endif()
install(TARGETS tiny EXPORT TinyTargets ARCHIVE DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT TinyTargets NAMESPACE Tiny:: DESTINATION lib/cmake/Tiny)
install(FILES TinyConfig.cmake DESTINATION lib/cmake/Tiny)
'''
    files={'support/CMakeLists.txt':support,'support/support.c':'#include "support.h"\nint support(int x){return x+5;}\n',
      'support/include/support.h':'struct support_value {int delta;};\nint support(int);\n',
      'support/SupportConfig.cmake':'include("${CMAKE_CURRENT_LIST_DIR}/SupportTargets.cmake")\n',
      'tiny/CMakeLists.txt':tiny,'tiny/include/tiny.h':'#ifndef TINY_WIDTH\n#error missing layout requirement\n#endif\n#ifdef TINY_PUBLIC_TYPE\n#include <support.h>\n#endif\nstruct tiny {int data[TINY_WIDTH];};\nint tiny_value(struct tiny);\n',
      'tiny/tiny.c':'#include "tiny.h"\n#include "support.h"\n#include "helper.h"\nint tiny_value(struct tiny x){return support(x.data[0])+PRIVATE_VALUE;}\n',
      'tiny/private/helper.h':'#define PRIVATE_VALUE 3\n#define LOCAL_VALUE 99\n',
      'tiny/TinyConfig.cmake':'include(CMakeFindDependencyMacro)\nfind_dependency(Support CONFIG)\ninclude("${CMAKE_CURRENT_LIST_DIR}/TinyTargets.cmake")\n',
      'consumer/CMakeLists.txt':'''cmake_minimum_required(VERSION 3.20)
project(Consumer LANGUAGES C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Tiny CONFIG REQUIRED)
foreach(n IN ITEMS one two)
 add_executable(${n} ${n}/main.c)
 target_include_directories(${n} PRIVATE ${n})
 target_link_libraries(${n} PRIVATE Tiny::tiny)
endforeach()
'''}
    for n,value in [('one',17),('two',23)]:
        files['consumer/'+n+'/helper.h']=f'#define LOCAL_VALUE {value}\n'
        files['consumer/'+n+'/main.c']=f'''#include <tiny.h>
#include <helper.h>
int main(void){{struct tiny t={{{{4,0}}}};
#ifdef TINY_PUBLIC_TYPE
struct support_value s={{1}};if(s.delta!=1)return 2;
#endif
return sizeof(t)!=2*sizeof(int)||tiny_value(t)!=12||LOCAL_VALUE!={value};}}
'''
    for name,text in files.items():c.write('packages/'+name,text)
    config=['-DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF','-DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF']
    for mode,shared,public in [('static',False,False),('shared',True,False),('public-type',False,True)]:
        prefix=d/(mode+'-A');relocated=d/(mode+'-B');separate=d/(mode+'-support-B')
        c.run(['cmake','-S',d/'support','-B',d/(mode+'-support'),'-G','Ninja','-DCMAKE_INSTALL_PREFIX='+str(prefix),'-DBUILD_SHARED_LIBS='+('ON' if shared else 'OFF'),*config])
        c.run(['cmake','--build',d/(mode+'-support')]);c.run(['cmake','--install',d/(mode+'-support')])
        c.run(['cmake','-S',d/'tiny','-B',d/(mode+'-tiny'),'-G','Ninja','-DCMAKE_INSTALL_PREFIX='+str(prefix),'-DCMAKE_PREFIX_PATH='+str(prefix),'-DPUBLIC_TYPE='+('ON' if public else 'OFF'),*config])
        c.run(['cmake','--build',d/(mode+'-tiny')]);c.run(['cmake','--install',d/(mode+'-tiny')])
        shutil.copytree(prefix,relocated)
        # Separate dependency prefix in the transfer: only Support files copied;
        # the consumer must discover it through find_dependency, never producer paths.
        if mode=='public-type':
            separate.mkdir();(separate/'lib/cmake').mkdir(parents=True);(separate/'include').mkdir()
            shutil.copytree(relocated/'lib/cmake/Support',separate/'lib/cmake/Support')
            shutil.copy2(relocated/'lib/libsupport.a',separate/'lib/libsupport.a')
            shutil.copy2(relocated/'include/support.h',separate/'include/support.h')
            for p in (relocated/'lib/cmake/Support').iterdir():p.unlink()
            (relocated/'lib/cmake/Support').rmdir();(relocated/'lib/libsupport.a').unlink();(relocated/'include/support.h').unlink()
        exported=list(relocated.rglob('*.cmake'))
        c.require(all(str(prefix) not in p.read_text() and str(d/'tiny') not in p.read_text() for p in exported),'installed package has no private absolute producer paths',mode)
        search=str(relocated)+(';' +str(separate) if public else '')
        build=d/(mode+'-consumer')
        c.run(['cmake','-S',d/'consumer','-B',build,'-G','Ninja','-DCMAKE_PREFIX_PATH='+search,*config])
        c.run(['cmake','--build',build,'--verbose'])
        for n in ('one','two'):c.run([build/n]);c.run(['readelf','-d',build/n])
        db=json.loads((build/'compile_commands.json').read_text())
        c.require(all('-DTINY_WIDTH=2' in x['command'] and '/tiny/private' not in x['command'] for x in db),'public layout propagated; private include excluded',mode)
        deps=c.run(['readelf','-d',build/'one'])
        c.require(('libsupport.so' in deps)==shared,'final static versus shared runtime dependency',mode)
        c.run(['nm','-u',d/(mode+'-tiny')/'libtiny.a'])
    # Deliberately expose the wrong private include ahead of a consumer's include.
    c.run(['gcc','-DTINY_WIDTH=2','-I'+str(d/'tiny/private'),'-I'+str(d/'consumer/one'),'-I'+str(d/'static-B/include'),d/'consumer/one/main.c',d/'static-B/lib/libtiny.a',d/'static-B/lib/libsupport.a','-o',d/'leaked'])
    c.run([d/'leaked'],expected=1)
    for leaf,claim in [('a','Tiny static unresolved Support call closes at final consumer link; public layout is independently propagated'),
        ('b','Two consumers retain different private helper values; deliberate private-path leak gives intended wrong result'),
        ('c','Installed exported targets consumed from relocated prefix without producer-tree search paths')]:
        c.observe('T07.06.'+leaf,claim,['packages source, exported configs, compile databases and commands'])
    c.observe('T07.06.a','Support shared variant changes actual final DT_NEEDED closure',['shared consumer readelf'],True)
    c.observe('T07.06.b','Making support_value public changes the transitive include/link contract',['public-type consumer builds'],True)
    c.observe('T07.06.c','Support separately relocated and resolved by explicit find_dependency',['public-type-B and public-type-support-B'],True)

    c.group='generation-phase';d=c.root/'phase';d.mkdir()
    for name,text in [('generator.py',GENERATOR),('step.c',STEP),('main.c',MAIN),('schema.txt','VALUE=7\n')]:c.write('phase/'+name,text)
    common='''cmake_minimum_required(VERSION 3.20)
project(Generation C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_executable(app step.c main.c)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
'''
    configure='''set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS schema.txt generator.py)
execute_process(COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/generator.py ${CMAKE_CURRENT_SOURCE_DIR}/schema.txt ${CMAKE_CURRENT_BINARY_DIR} RESULT_VARIABLE generated)
if(NOT generated EQUAL 0)
 message(FATAL_ERROR "GEN_REJECT:configure")
endif()
'''
    buildtime='''add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cfg.h ${CMAKE_CURRENT_BINARY_DIR}/mirror.h
 COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/generator.py ${CMAKE_CURRENT_SOURCE_DIR}/schema.txt ${CMAKE_CURRENT_BINARY_DIR}
 DEPENDS schema.txt generator.py VERBATIM)
target_sources(app PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/cfg.h ${CMAKE_CURRENT_BINARY_DIR}/mirror.h)
'''
    for mode,generation in [('configure',configure),('build',buildtime)]:
        c.write('phase/CMakeLists.txt',common+generation);folder=d/mode
        c.run(['cmake','-S',d,'-B',folder,'-G','Ninja','--trace-expand'])
        if mode=='build':c.require(not (folder/'cfg.h').exists(),'build-owned input absent after configuration')
        c.run(['cmake','--build',folder]);c.require(c.run([folder/'app'])=='19\n','generated stage initial result',mode)
        saved=sha(folder/'cfg.h');c.write('phase/schema.txt','FAIL\n')
        c.run(['cmake','--build',folder],expected='nonzero',contains='GEN_REJECT:input')
        c.require(sha(folder/'cfg.h')==saved,'failed phase cannot publish new output',mode)
        c.write('phase/schema.txt','VALUE=7\n');c.run(['cmake','--build',folder])
        c.run(['cmake','-S',d,'-B',folder]);c.require(sha(folder/'cfg.h')==saved,'reconfigure retains coherent value',mode)
    c.observe('T07.07.a','Actual configure and build owners, failure propagation and preserved generated outputs',['phase traces and source'])
    c.observe('T07.07.a','Build-time custom command explicitly owns generated headers and input edges',['phase build branch'],True)

    c.group='foreign';d=c.root/'foreign';d.mkdir()
    host='''#include <stdio.h>
int main(int argc,char**argv){unsigned value;char tail;FILE *in,*out;if(argc!=3)return 2;
in=fopen(argv[1],"r");if(!in)return 3;if(fscanf(in,"VALUE=%u %c",&value,&tail)!=1||value>100){fclose(in);return 4;}fclose(in);
out=fopen(argv[2],"w");if(!out)return 5;int bad=fprintf(out,"#define VALUE %u\\n",value)<0;if(fclose(out))bad=1;return bad;}
'''
    c.write('foreign/hostgen.c',host);c.write('foreign/value.txt','VALUE=7\n')
    c.run(['gcc','-std=c17','-Wall','-Wextra','-Werror',d/'hostgen.c','-o',d/'hostgen'])
    c.run([d/'hostgen',d/'value.txt',d/'cfg.h'])
    c.write('foreign/probe.c','#include "cfg.h"\nunsigned value(void){return VALUE;}\n')
    c.run(['gcc','-std=c17','-c',d/'probe.c','-o',d/'native.o'])
    c.run(['arm-none-eabi-gcc','-std=c17','-mcpu=cortex-m4','-mthumb','-ffreestanding','-c',d/'probe.c','-o',d/'target.o'])
    def elf(path):
        b=path.read_bytes();return {'class':b[4],'data':b[5],'machine':struct.unpack_from('<H',b,18)[0],'sha256':sha(path)}
    manifest={'host_generator':{'path':str(d/'hostgen'),'target':'x86-64 Linux','runnable':True},'native':elf(d/'native.o'),'foreign':elf(d/'target.o'),
        'compiler':c.run(['arm-none-eabi-gcc','--version']).splitlines()[0],'triple':c.run(['arm-none-eabi-gcc','-dumpmachine']).strip(),
        'sysroot':c.run(['arm-none-eabi-gcc','-print-sysroot']).strip(),'execution':'compile/link inspection only'}
    c.require(manifest['native']['class']==2 and manifest['native']['machine']==62 and manifest['foreign']['class']==1 and manifest['foreign']['machine']==40,'native/foreign ELF identity independently decoded',manifest)
    wrong=dict(manifest['foreign'],machine=62);c.require(wrong!=elf(d/'target.o'),'mislabeled target rejected before execution')
    c.write('foreign/toolchain.cmake',f'''set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_FLAGS_INIT "-mcpu=cortex-m4 -mthumb -ffreestanding")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH "{d/'target-root'}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
''')
    c.write('foreign/CMakeLists.txt',f'''cmake_minimum_required(VERSION 3.20)
project(Foreign C)
include(CheckCSourceCompiles)
check_c_source_compiles("_Static_assert(sizeof(void*)==4,\\\"pointer width\\\"); int f(void){{return 0;}}" PTR32)
if(NOT PTR32)
 message(FATAL_ERROR "target pointer width probe failed")
endif()
add_custom_command(OUTPUT ${{CMAKE_CURRENT_BINARY_DIR}}/cfg.h COMMAND "{d/'hostgen'}" "{d/'value.txt'}" ${{CMAKE_CURRENT_BINARY_DIR}}/cfg.h DEPENDS "{d/'hostgen'}" value.txt VERBATIM)
add_library(target STATIC probe.c ${{CMAKE_CURRENT_BINARY_DIR}}/cfg.h)
target_include_directories(target PRIVATE ${{CMAKE_CURRENT_BINARY_DIR}})
''')
    c.run(['cmake','-S',d,'-B',d/'cmake-target','-G','Ninja','-DCMAKE_TOOLCHAIN_FILE='+str(d/'toolchain.cmake')])
    c.run(['cmake','--build',d/'cmake-target','--verbose'])
    c.run(['arm-none-eabi-readelf','-h',d/'cmake-target/libtarget.a'])
    c.run([str(d/'missing-hostgen'),d/'value.txt',d/'cfg.h'],expected='launch-unavailable')
    c.observe('T07.02.a','Native/Arm artifact manifest verifies class, endianness and machine before any execution',['foreign/manifest.json'])
    c.observe('T07.02.c','Separate compiled host C generator creates input to target-only Arm compilation',['foreign/hostgen.c','foreign CMake commands'])
    c.observe('T07.02.c','Host C generator remains explicitly separate from non-native application compiler',['foreign/hostgen.c'],True)
    c.observe('T07.07.c','STATIC_LIBRARY target try-compile establishes pointer width; no accidental target execution',['foreign/toolchain.cmake','CMake configure trace'])
    # Compatible ISA, different call conventions. Only safe link/disassembly tests.
    c.write('foreign/float.c','float add(float a,float b){return a+b;}\n')
    c.write('foreign/caller.c','float add(float,float);float use(void){return add(1.0f,2.0f);}\n')
    c.write('foreign/variadic.c','#include <stdarg.h>\nfloat vadd(int count,...){va_list a;va_start(a,count);float x=(float)va_arg(a,double);va_end(a);return count?x:0.0f;}\n')
    attrs={}
    for abi in ('softfp','hard'):
        flags=['-mcpu=cortex-m4','-mthumb','-mfpu=fpv4-sp-d16','-mfloat-abi='+abi]
        for name in ('float','caller','variadic'):c.run(['arm-none-eabi-gcc','-std=c17','-O2',*flags,'-c',d/(name+'.c'),'-o',d/(name+'-'+abi+'.o')])
        attrs[abi]=c.run(['arm-none-eabi-readelf','-A',d/('float-'+abi+'.o')]);c.run(['arm-none-eabi-objdump','-dr',d/('variadic-'+abi+'.o')])
        c.run(['arm-none-eabi-gcc',*flags,'-nostdlib','-r',d/('float-'+abi+'.o'),d/('caller-'+abi+'.o'),'-o',d/('compatible-'+abi+'.o')])
    c.require('VFP registers' in attrs['hard'] and 'VFP registers' not in attrs['softfp'],'attribute distinguishes same-ISA calling convention')
    c.run(['arm-none-eabi-ld','-r',d/'float-hard.o',d/'caller-softfp.o','-o',d/'incompatible.o'],expected='nonzero',contains='VFP register arguments')
    c.observe('T07.02.a','Same Cortex-M4 instruction support, actual softfp/hard attributes and incompatible-link rejection',['foreign ABI commands'],True)
    c.observe('T07.09.c','Arm compatible and incompatible fixed-prototype ABI objects inspected and safely linked',['foreign ABI commands'])
    c.observe('T07.09.c','Variadic double argument is consumed using base PCS even for hard-float build; conversion helper/stack instructions retained',['foreign/variadic.c and disassembly'],True,
        ['Compile observation plus AAPCS32 rule; no Arm execution inferred.'])
    manifest['abi_attributes']=attrs;c.write('foreign/manifest.json',json.dumps(manifest,indent=2))
