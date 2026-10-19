"""Independent struct-based oracle and hostile original-container campaign."""
import json,struct,subprocess,random,hashlib

def expected(data):
    # Oracle is used only for independently constructed valid fixtures and tool-
    # produced originals. Negative validity is specified case-by-case below.
    klass,order=data[4:6];e='<' if order==1 else '>';bits=32 if klass==1 else 64
    h=struct.unpack_from(e+('HHIIIIIHHHHHH' if bits==32 else 'HHIQQQIHHHHHH'),data,16)
    kind,machine,version,entry,phoff,shoff,flags,hs,ps,pn,ss,sn,strings=h
    sections=[list(struct.unpack_from(e+('IIIIIIIIII' if bits==32 else 'IIQQQQIIQQ'),data,shoff+i*ss)) for i in range(sn)]
    programs=[]
    for i in range(pn):
        p=list(struct.unpack_from(e+('IIIIIIII' if bits==32 else 'IIQQQQQQ'),data,phoff+i*ps))
        if bits==32:p=[p[0],p[6],*p[1:6],p[7]]
        programs.append(p)
    symbols=[]
    for table,s in enumerate(sections):
        if s[1] not in (2,11):continue
        text=sections[s[6]]
        for i in range(s[5]//s[9]):
            raw=struct.unpack_from(e+('IIIBBH' if bits==32 else 'IBBHQQ'),data,s[4]+i*s[9])
            if bits==32:name,value,size,info,other,section=raw
            else:name,info,other,section,value,size=raw
            start=text[4]+name;end=data.index(0,start,text[4]+text[5]) if name else start
            symbols.append([table,i,name,info,other,section,value,size,data[start:end].hex()])
    return dict(status=0,header=[bits,order,kind,machine,entry,sn,pn,strings],sections=sections,programs=programs,symbols=symbols)

def original(bits=64,order=1):
    e='<' if order==1 else '>';hs,ps,ss=(64,56,64) if bits==64 else (52,32,40)
    # Fixed offsets leave inactive bytes; none are assumed to be native structs.
    data=bytearray(1024);data[:7]=b'\x7fELF'+bytes([2 if bits==64 else 1,order,1]);phoff=hs;shoff=256
    struct.pack_into(e+('HHIQQQIHHHHHH' if bits==64 else 'HHIIIIIHHHHHH'),data,16,2,62 if bits==64 else 3,1,0x400000,phoff,shoff,0,hs,ps,1,ss,4,1)
    program=[1,5,0x300,0x400000,0,0x30,0x50,0x100]
    raw=program if bits==64 else [program[0],*program[2:7],program[1],program[7]]
    struct.pack_into(e+('IIQQQQQQ' if bits==64 else 'IIIIIIII'),data,phoff,*raw)
    names=b'\0names\0symbols\0text\0item\0';data[640:640+len(names)]=names
    sections=[[0]*10,[1,3,0,0,640,len(names),0,0,1,0],[7,2,0,0,704,48 if bits==64 else 32,1,1,8 if bits==64 else 4,24 if bits==64 else 16],[15,1,6,0x400000,768,48,0,0,16,0]]
    for i,s in enumerate(sections):struct.pack_into(e+('IIQQQQIIQQ' if bits==64 else 'IIIIIIIIII'),data,shoff+i*ss,*s)
    sym=(20,0x12,0,3,0x400000,16) if bits==64 else (20,0x400000,16,0x12,0,3)
    struct.pack_into(e+('IBBHQQ' if bits==64 else 'IIIBBH'),data,704+(24 if bits==64 else 16),*sym)
    data[768:816]=bytes(range(48));return bytes(data)

def run(c,root):
    c.group='elf-decoder';public=root/'labs/machine_image/elf_depth';private=root/'assessor/legacy_systems/elf_depth'
    c.run(['gcc','--version']);c.run(['clang','--version']);c.run(['readelf','--version'])
    specimens=c.root/'specimens';specimens.mkdir();source=c.write('object.c','int data=7;int zero[8];static int local(int x){return x+data;} int api(int x){return local(x);}\n')
    main=c.write('main.c','int api(int);int main(void){return api(35)!=42;}\n')
    artifacts=[]
    for name,args in [('rel.o',['-c',source]),('executable',['-no-pie',source,main]),('shared.so',['-shared','-fPIC',source]),('static-pie',['-static-pie',source,main])]:
        file=specimens/name;c.run(['gcc','-g',*args,'-o',file]);c.run(['readelf','-h','-SW','-lW','-sW',file]);artifacts.append(file)
    for name,cc,opts in [('ilp32.o','gcc',['-m32']),('arm-be.o','arm-none-eabi-gcc',['-mcpu=cortex-m4','-mthumb','-mbig-endian']),('ppc64-be.o','clang',['--target=powerpc64-unknown-linux-gnu'])]:
        file=specimens/name;c.run([cc,*opts,'-c',source,'-o',file]);c.run(['readelf','-h','-SW','-sW',file]);artifacts.append(file)
    c.require({expected(p.read_bytes())['header'][2] for p in artifacts}=={1,2,3},'actual REL EXEC DYN classification')
    records=[]
    for bits in (32,64):
        for order in (1,2):records.append((f'original-{bits}-{order}',original(bits,order),0))
    records += [(p.name,p.read_bytes(),0) for p in artifacts]
    stripped=bytearray(artifacts[1].read_bytes());struct.pack_into('<Q',stripped,40,0);struct.pack_into('<HHH',stripped,58,0,0,0)
    records.append(('sectionless',bytes(stripped),0));target=specimens/'sectionless';target.write_bytes(stripped);target.chmod(0o755);c.run([target]);c.run(['readelf','-lW',target])
    base=original();records += [('truncated-'+str(n),base[:n],None) for n in range(64)]
    declared=bytearray(base);declared.extend(bytes(0x1030-len(declared)))
    struct.pack_into('<Q',declared,64+8,0x1000);struct.pack_into('<Q',declared,64+48,0x1000)
    struct.pack_into('<Q',declared,256+3*64+24,0x1000);declared[0x1000:0x1030]=bytes(range(48))
    records.append(('declared-load-ranges',bytes(declared),0))
    def changed(label,offset,fmt,value,status):
        blob=bytearray(base);struct.pack_into(fmt,blob,offset,value);records.append((label,bytes(blob),status))
    changed('unsupported-class',4,'B',9,4);changed('unsupported-order',5,'B',3,4)
    changed('huge-section-offset',40,'<Q',2**64-1,3);changed('huge-program-offset',32,'<Q',2**64-1,3)
    changed('extended-sections',60,'<H',0,4);changed('extended-programs',56,'<H',65535,4)
    changed('extended-string-index',62,'<H',65535,4);changed('wrong-header-entry',58,'<H',1,3)
    changed('filesz-exceeds-memsz',64+32,'<Q',0x60,3);changed('address-wrap',64+16,'<Q',2**64-16,3)
    changed('bad-symbol-string',704+24,'<I',999,3);changed('missing-string-nul',640+24,'B',88,2)
    changed('bad-symbol-entsize',256+2*64+56,'<Q',0,2);changed('bad-alignment',256+3*64+48,'<Q',3,2)
    wrap32=bytearray(original(32));struct.pack_into('<I',wrap32,52+8,0xfffffff0);records.append(('ELF32-address-wrap',bytes(wrap32),3))
    # Byte mutations are memory-safety/determinism probes, not all declared malformed.
    rng=random.Random(90817)
    for i in range(512):
        blob=bytearray(base);at=rng.randrange(len(blob));blob[at]^=1<<rng.randrange(8);records.append(('mutated-'+str(i),bytes(blob),'compare'))
    path=c.root/'inputs.bin';path.write_bytes(b''.join(struct.pack('<I',len(blob))+blob for _,blob,_ in records))
    c.write('input-cases.json',json.dumps([{'id':name,'length':len(blob),'sha256':hashlib.sha256(blob).hexdigest(),'expected':status} for name,blob,status in records],indent=2))
    baseline=None
    for cc in ('gcc','clang'):
        for profile,opts in [('debug',['-O0']),('release',['-O2']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
            exe=c.root/(cc+'-'+profile);c.run([cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*opts,'-I'+str(public),public/'inspect.c',private/'elf_depth.c','-o',exe])
            # Explicit file redirection using subprocess APIs, not a shell.
            with path.open('rb') as stream:
                result=subprocess.run([str(exe)],stdin=stream,capture_output=True,text=True,timeout=30)
            c.commands.append(dict(group=c.group,argv=[str(exe)],cwd=str(c.root),stdin_file=str(path),stdin_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),exit=result.returncode,expected=0,diagnostic=None,stdout=result.stdout,stderr=result.stderr,passed=result.returncode==0))
            c.require(result.returncode==0,'decoder execution completes',result.stderr)
            rows=[json.loads(line) for line in result.stdout.splitlines()];c.require(len(rows)==len(records),'one result per framed input',len(rows))
            for (name,blob,status),row in zip(records,rows):
                if status==0:c.require(row==expected(blob),'independent valid decoder '+name,row)
                elif status is None:c.require(row['status']!=0,'truncation rejected '+name,row)
                elif status!='compare':c.require(row['status']==status,'intended rejection '+name,row)
            if baseline is None:baseline=rows
            else:c.require(rows==baseline,'all mutation outcomes stable across profiles',len(rows))
    # A semantic bad-range control compiles and accepts the specified unsafe segment.
    for name,implementation,opts in [('mutant',private/'elf_depth.c',['-DED_MUTANT_SEGMENT']),('learner',public/'elf_depth.c',[])]:
        exe=c.root/name;c.run(['gcc','-O2','-I'+str(public),*opts,public/'inspect.c',implementation,'-o',exe])
        chosen=next(blob for label,blob,_ in records if label=='filesz-exceeds-memsz') if name=='mutant' else base
        encoded=struct.pack('<I',len(chosen))+chosen;result=subprocess.run([str(exe)],input=encoded,capture_output=True,timeout=10)
        c.commands.append(dict(group=c.group,argv=[str(exe)],cwd=str(c.root),stdin_sha256=hashlib.sha256(encoded).hexdigest(),exit=result.returncode,expected=0,diagnostic=None,stdout=result.stdout.decode(),stderr=result.stderr.decode(),passed=result.returncode==0))
        observed=json.loads(result.stdout);c.require(result.returncode==0 and observed['status']==(0 if name=='mutant' else 4),'semantic range mutant or learner rejection '+name,observed)
    obj=c.root/'decoder-arm.o';c.run(['arm-none-eabi-gcc','-std=c17','-mcpu=cortex-m4','-mthumb','-O2','-ffreestanding','-fno-builtin','-I'+str(public),'-c',private/'elf_depth.c','-o',obj])
    unresolved=c.run(['arm-none-eabi-nm','-u',obj]);c.write('decoder-closure.txt',unresolved)
    c.require(not unresolved.strip(),'Arm decoder has no implicit runtime helper',unresolved)
    for cc in ('gcc','clang'):
        c.run(['python3',public/'run_core.py','--implementation',private/'elf_depth.c','--cc',cc],contains='"passed": true')
    c.run(['python3',public/'run_core.py','--implementation',public/'elf_depth.c'],expected=1,contains='"passed": false')
    api=c.write('accessor-contract.c',r'''#include "elf_depth.h"
#include <string.h>
#include <stdio.h>
int main(void){unsigned char file[64]={127,'E','L','F',2,1,1};file[20]=1;file[52]=64;
 struct ed_view view;memset(&view,0xa5,sizeof view);unsigned char original[sizeof view];memcpy(original,&view,sizeof view);
 if(ed_open(NULL,1,&view)!=ED_ARGUMENT||memcmp(original,&view,sizeof view)||ed_open(file,64,NULL)!=ED_ARGUMENT)return 1;
 if(ed_open(file,64,&view)!=ED_OK)return 2;
 struct ed_section section={0};section.size=123;struct ed_program program={0};program.memsz=456;struct ed_symbol symbol={0};symbol.value=789;
 const char *name="sentinel";size_t n=987;
 if(ed_section(&view,0,&section)!=ED_RANGE||section.size!=123||ed_program(&view,0,&program)!=ED_RANGE||program.memsz!=456)return 3;
 if(ed_symbol(&view,0,0,&symbol)!=ED_RANGE||symbol.value!=789||ed_name(&view,0,0,&name,&n)!=ED_RANGE||n!=987||strcmp(name,"sentinel"))return 4;
 if(ed_section(NULL,0,&section)!=ED_ARGUMENT||ed_program(&view,0,NULL)!=ED_ARGUMENT||ed_symbol(NULL,0,0,&symbol)!=ED_ARGUMENT||ed_name(&view,0,0,NULL,&n)!=ED_ARGUMENT)return 5;
 puts("ACCESSOR-CONTRACT-PASS");return 0;}
''')
    for cc in ('gcc','clang'):
        exe=c.root/(cc+'-accessors');c.run([cc,'-std=c17','-O1','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-pie','-no-pie','-I'+str(public),api,private/'elf_depth.c','-o',exe]);c.run([exe],contains='ACCESSOR-CONTRACT-PASS')
    c.observe('T09.01.a','Checked borrowed byte parser reports actual headers/sections/programs/symbols without native-struct casts',['elf-decoder','input-cases.json'])
    c.observe('T09.01.a','Same parser handles actual ELF32/64 little/big-endian original vectors and rejects extended counts explicitly',['elf-decoder'],True)
    c.observe('T09.01.b','Synthetic initialized0x30/zero0x20 ranges and filesz>memsz rejection',['elf-decoder'])
    c.observe('T09.01.b','Original executable still runs with section table removed; program headers remain inspectable',['specimens/sectionless'],True)
    c.observe('T09.01.c','Actual object/executable/shared classification uses header metadata, not suffix',['elf-decoder'])
    c.observe('T09.01.c','Actual static PIE ET_DYN artifact inspected separately from shared-object role',['specimens/static-pie'],True)
