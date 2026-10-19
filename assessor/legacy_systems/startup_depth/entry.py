"""Real Linux entries, original stack snapshots, and controlled bridge defects."""
import hashlib,json,re,struct

def metadata(path):
 b=path.read_bytes();h=struct.unpack_from('<HHIQQQIHHHHHH',b,16)
 assert b[:6]==b'\x7fELF\x02\x01'
 ph=[struct.unpack_from('<IIQQQQQQ',b,h[4]+i*h[8]) for i in range(h[9])]
 return dict(type=h[0],entry=h[3],interpreter=next((b[p[2]:p[2]+p[5]].rstrip(b'\0').decode() for p in ph if p[0]==3),None),loads=[dict(flags=p[1],offset=p[2],address=p[3],filesz=p[5],memsz=p[6]) for p in ph if p[0]==1])

def run(c,root,args):
 c.group='startup-entry';d=c.root/'entry';d.mkdir();public=root/'labs/machine_image/startup_depth';private=root/'assessor/legacy_systems/startup_depth'
 def src(name,text):return c.write('entry/'+name,text)
 hosted=src('hosted.c','#include <stdio.h>\n_Thread_local int tls=7;int data=19;int *pointer=&data;int bss;static int hook;__attribute__((constructor)) static void initialize(void){hook=23;} int main(void){if(tls!=7||*pointer!=19||bss||hook!=23)return 1;puts("HOSTED-42");return 0;}\n')
 images={};metas={};snapshots={}
 for mode,flags in [('dynamic',['-fno-pie','-no-pie']),('pie',['-fPIE','-pie']),('static',['-static','-fno-pie','-no-pie']),('static-pie',['-fPIE','-static-pie'])]:
  exe=d/mode;c.run(['gcc','-std=c17','-g','-O0',hosted,*flags,'-o',exe]);c.run([exe],contains='HOSTED-42');images[mode]=exe;metas[mode]=metadata(exe)
  c.run(['readelf','-h','-lW','-dW','-rW',exe]);c.run(['nm',exe],contains='_start')
  out=d/(mode+'-entry.json');script=src(mode+'.gdb',f'''set pagination off
set confirm off
set disable-randomization off
starti
python
import gdb,json
inf=gdb.selected_inferior()
sp=int(gdb.parse_and_eval('$rsp'));pc=int(gdb.parse_and_eval('$pc'))
def word(i):return int.from_bytes(inf.read_memory(sp+8*i,8).tobytes(),'little')
argc=word(0);assert argc<256
p=argc+2;assert word(argc+1)==0
while word(p):p+=1;assert p<512
p+=1;aux=[]
while word(p):aux.append([word(p),word(p+1)]);p+=2;assert p<1024
p+=2
entry=dict(aux)[9]
record=dict(initial_pc=pc,initial_sp=sp,argc=argc,auxv=aux,application_entry=entry,table_hex=inf.read_memory(sp,p*8).tobytes().hex(),entry_code=inf.read_memory(pc,16).tobytes().hex())
gdb.execute('info proc mappings')
if pc!=entry:
 gdb.Breakpoint('*'+hex(entry));gdb.execute('continue')
record['application_pc']=int(gdb.parse_and_eval('$pc'))
record['application_sp']=int(gdb.parse_and_eval('$rsp'))
open({str(out)!r},'w').write(json.dumps(record,indent=2))
end
break main
continue
printf "REACHED-MAIN\\n"
continue
''')
  c.run(['gdb','-q','-nx','-batch','-x',script,exe],contains='REACHED-MAIN',timeout=60)
  snap=json.loads(out.read_text());snapshots[mode]=snap;c.require(snap['application_pc']==snap['application_entry'],'AT_ENTRY breakpoint belongs to application',mode)
  c.require((snap['initial_pc']!=snap['application_entry'])==bool(metas[mode]['interpreter']),'first instruction owner follows actual PT_INTERP',mode)
  if mode in ('dynamic','static'):c.require(snap['application_entry']==metas[mode]['entry'],'fixed ET_EXEC entry matches loaded address',mode)
  if mode=='static-pie':
   script=src('self-relocation.gdb','set pagination off\nset disable-randomization off\nstarti\nbreak _dl_relocate_static_pie\ncontinue\nbt\nprintf "STATIC-SELF-RELOCATION\\n"\nbreak main\ncontinue\n')
   c.run(['gdb','-q','-nx','-batch','-x',script,exe],contains='STATIC-SELF-RELOCATION')
 # Minimal images include no system startup objects. Check their actual origins.
 base=['-std=c17','-g','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-pie','-no-pie','-nostdlib','-Wl,--build-id=sha1','-I'+str(public)]
 sources=[private/n for n in ('start.S','raw_x86_64.S','image.c','stack.c','transfer.c','fd.c')]
 raw={};manifest=[]
 for cc in ('gcc','clang'):
  for opt in ('-O0','-O2'):
   for variant,defines,expected in [('base',[],42),('hook',['-DSD_INIT_HOOK'],42),('early',['-DSD_EARLY_FAILURE'],73),('saved',['-DSD_BAD_SAVED_SP'],71),('align',['-DSD_BAD_ALIGNMENT'],72),('parser',['-DSD_PARSE'],42),('decimal',['-DSD_DECIMAL'],42),('nodiv',['-DSD_DECIMAL','-DSD_NODIV'],42)]:
    name=f'{cc}-{opt[1:]}-{variant}';exe=d/name;mapfile=d/(name+'.map')
    actual=sources+([private/'decimal.c'] if variant in ('parser','decimal','nodiv') else [])
    c.run([cc,*base,opt,'-save-temps=obj','-ffunction-sections','-fdata-sections','-fstack-usage',*defines,*actual,'-Wl,--gc-sections','-Wl,-Map='+str(mapfile),'-o',exe])
    if cc=='clang':
     # Clang21 uses source basenames under the output directory for saved
     # temporaries; freeze them before the next configuration can overwrite.
     for source in actual:
      for temp in d.glob(source.stem+'.*'):
       if temp.suffix in ('.o','.s','.bc','.i','.su'):
        destination=d/(name+'-'+temp.name);assert not destination.exists();temp.rename(destination)
    objects=sorted(d.glob(name+'-*.o'))
    c.require(len(objects)==len(actual),'actual retained target objects for every source',dict(image=name,objects=[p.name for p in objects]))
    defined=set();imports=set();object_audit=[]
    for obj in objects:
     table=c.run(['nm','-g',obj]);code=c.run(['objdump','-dr',obj]);object_audit.append(dict(path=obj.name,sha256=hashlib.sha256(obj.read_bytes()).hexdigest(),symbols=table))
     for line in table.splitlines():
      fields=line.split()
      if len(fields)>=2:
       if fields[-2]=='U':imports.add(fields[-1])
       elif fields[-2].upper() in ('T','D','B','R','W','V'):defined.add(fields[-1])
    c.require(not imports-defined,'all object imports resolve to original selected sources',dict(image=name,imports=sorted(imports),definitions=sorted(defined)))
    out=c.run([exe,'arg1','arg2'],expected=expected)
    want=(''.join(chr(65+i%26) for i in range(4096))+('4294967295' if variant in ('decimal','nodiv') else '')) if expected==42 else ''
    c.require(out==want,'raw process exact bytes and deterministic exit',name)
    undefined=c.run(['nm','-u',exe]);c.require(not undefined.strip(),'raw final undefined-symbol set empty',name)
    meta=metadata(exe);c.require(meta['interpreter'] is None,'raw image no interpreter',name)
    tags=c.run(['readelf','-dW','-rW',exe]);c.require('NEEDED' not in tags and 'There are no relocations' in tags,'raw final no dynamic dependencies or remaining relocations',name)
    c.run(['objdump','-d',exe],contains='syscall');size=c.run(['size','-A',exe]);symbols=c.run(['nm','-S','--size-sort',exe])
    c.require(all(term not in symbols for term in ('__libc','__aeabi','__udiv','__stack_chk','malloc','memcpy','memset')),'raw symbol allowlist rejects known helper origins',name)
    manifest.append(dict(name=name,sha256=hashlib.sha256(exe.read_bytes()).hexdigest(),mode=variant,metadata=meta,size=size,symbols=symbols,objects=object_audit,map_sha256=hashlib.sha256(mapfile.read_bytes()).hexdigest()))
    raw[name]=exe
 c.run(['python3',public/'run_target.py','--image',raw['gcc-O2-base']],contains='"passed": true')
 c.run(['python3',public/'run_target.py','--image',raw['clang-O2-decimal'],'--decimal'],contains='"passed": true')
 c.run(['python3',public/'run_target.py','--image',raw['gcc-O2-early']],expected=1,contains='"passed": false')
 # Capture before C and after call; addresses vary with ASLR, equations do not.
 exe=raw['gcc-O2-base'];capture=d/'bridge.json'
 script=src('bridge.gdb',f'''set pagination off
set disable-randomization off
starti
python
import gdb,json
first=dict(pc=int(gdb.parse_and_eval('$pc')),sp=int(gdb.parse_and_eval('$rsp')))
first['code']=gdb.selected_inferior().read_memory(first['pc'],16).tobytes().hex()
end
break sd_before_call
continue
python
first['before_call_sp']=int(gdb.parse_and_eval('$rsp'))
first['saved']=int(gdb.parse_and_eval('sd_original_sp'))
end
break sd_probe
continue
python
first['helper_sp']=int(gdb.parse_and_eval('$rsp'))
first['return_address']=int.from_bytes(gdb.selected_inferior().read_memory(first['helper_sp'],8).tobytes(),'little')
open({str(capture)!r},'w').write(json.dumps(first,indent=2))
end
continue
''')
 c.run(['gdb','-q','-nx','-batch','-x',script,exe]);snap=json.loads(capture.read_text())
 c.require(snap['sp']==snap['saved'] and snap['before_call_sp']%16==0 and snap['helper_sp']+8==snap['before_call_sp'],'original stack preserved across aligned call and real pushed return address',snap)
 # Matching separate debug image recovers the same instruction/entry identity.
 stripped=d/'stripped';debug=d/'stripped.debug';c.run(['objcopy','--only-keep-debug',exe,debug]);c.run(['objcopy','--strip-all',exe,stripped]);c.run(['objcopy','--add-gnu-debuglink='+str(debug),stripped])
 c.run(['gdb','-q','-nx','-batch','-ex','set disable-randomization off','-ex','starti','-ex','info address _start','-ex','x/8i $pc','-ex','info files',stripped],contains='_start')
 c.require(metadata(stripped)['entry']==metadata(exe)['entry'],'stripping preserves actual entry')
 # Each original PIE rejects an unrelocated slot before dereference. Assigning
 # this one slot is a bounded repair, not a general ELF relocation engine.
 for cc in ('gcc','clang'):
  for name,defines,status in [('unrelocated',[],86),('relocated',['-DSD_REPAIR_RELOCATION'],42),('two-defects',['-DSD_BAD_ALIGNMENT'],72),('alignment-fixed',[],86)]:
   pie=d/(cc+'-'+name);flags=[x for x in base if x not in ('-fno-pie','-no-pie')]
   c.run([cc,*flags,'-O0','-fPIE','-static-pie',*defines,*sources,'-o',pie]);out=c.run([pie],expected=status)
   c.run(['readelf','-rW',pie],contains='R_X86_64_RELATIVE');c.run(['objdump','-d',pie])
   c.require(metadata(pie)['interpreter'] is None,'original unrelocated image genuinely has no interpreter')
   if status==42:c.require(len(out)==4096,'explicit pointer repair retains full output')
 # Explicit ROM-to-RAM copy specimen. Linux ELF zero-fill does not implement
 # this board-style copy contract; this host model makes no board claim.
 rom=src('copy-initialized.c','#include <stdint.h>\nstatic const unsigned char rom[4]={19,0,0,0};static unsigned char ram[4];int main(void){\n#ifndef OMIT_COPY\n for(unsigned i=0;i<4;++i)ram[i]=rom[i];\n#else\n (void)rom;\n#endif\n uint32_t value=0;for(unsigned i=0;i<4;++i)value|=(uint32_t)ram[i]<<(8*i);return value==19?0:74;}\n')
 for cc in ('gcc','clang'):
  for name,flags,status in [('copy',[],0),('omitted',['-DOMIT_COPY'],74)]:
   program=d/(cc+'-'+name);c.run([cc,'-std=c17','-O2',*flags,rom,'-o',program]);c.run([program],expected=status)
 # Over-aligned aggregate is passed in stack storage aligned32 before call.
 aligned=src('over.c','struct __attribute__((aligned(32))) block{unsigned long w[4];};__attribute__((noinline)) int over(struct block b){return ((unsigned long)&b&31)?91:(b.w[0]==11&&b.w[3]==31?42:92);}\n')
 assembly=src('over.S','.text\n.globl _start\n_start:\n mov %rsp,%r12\n and $-32,%rsp\n sub $32,%rsp\n movq $11,(%rsp)\n movq $17,8(%rsp)\n movq $23,16(%rsp)\n movq $31,24(%rsp)\n call over\n cmpq $1,(%r12)\n jne bad\n mov %eax,%edi\n jmp finish\nbad: mov $93,%edi\nfinish: mov $60,%eax\n syscall\n ud2\n.section .note.GNU-stack,"",@progbits\n')
 for cc in ('gcc','clang'):
  over=d/(cc+'-over');c.run([cc,*base,'-O2',aligned,assembly,'-o',over]);c.run([over],expected=42);c.run(['objdump','-d',over])
 c.write('entry/image-manifests.json',json.dumps(manifest,indent=2));c.write('entry/hosted-metadata.json',json.dumps(metas,indent=2));c.write('entry/hosted-snapshots.json',json.dumps(snapshots,indent=2))
 for obj,claim,var in [
 ('01.a','GDB first instruction, AT_ENTRY and application entry compared for dynamic/PIE/static originals',False),('01.a','Actual static-PIE entry reaches self-relocation before main',True),
 ('01.c','Raw assembly preserves original stack, establishes call, and exits42 without returning through argc',False),('01.c','Helper early failure73 still terminates explicitly',True),
 ('02.c','Actual initial/call/helper stack pointers satisfy preserved-pointer and return-address equations',False),('02.c','32-byte aligned aggregate handoff and original argc retained across stronger alignment',True),
 ('04.b','Only bounded decode/output/explicit exit admitted by original raw target',False),('04.b','Restricted original initialization hook writes private state without runtime dependencies',True),
 ('05.c','Raw finish translates helper result and retains explicit ud2 terminal fallback',False),('05.c','Failure before argument decoding takes deterministic exit73',True),
 ('08.a','Dynamic/static/static-PIE metadata and actual execution retain distinct runtime ownership',False),('08.a','TLS original executes in all modes using their real C runtime initialization',True),
 ('08.b','4096 exact patterned output bytes and exit42 verified across GCC/Clang O0/O2',False),('08.b','Bounded maximum decimal formatting adds exact10 bytes with inspected final closure',True),
 ('09.b','O0/O2 parser/formatter increments retain code size, symbols, link maps and stack-usage artifacts',False),('09.b','Division and bounded subtraction variants satisfy same maximum-output contract with separate image manifests',True),
 ('10.a','GDB stops before C and records actual entry bytes, original stack and call boundary',False),('10.a','Stripped image keeps entry identity and matched companion recovers entry analysis',True),
 ('10.b','Separate controlled alignment, omitted ROM copy and unrelocated static-PIE specimens reject distinct invariants',False),('10.b','Two-defect PIE rejects alignment first; its repair exposes relocation failure before dereference',True),
 ('10.c','Wrong saved pointer minimized to original assembly bridge and checked helper boundary',False),('10.c','GCC and Clang O0/O2 reject same saved-pointer invariant with exit71',True)]:c.observe('T10.'+obj,claim,['startup-entry'],var)
