"""Synthetic, never executed memory maps; real retention and image transformations."""
import hashlib,json,re,struct,zlib


def run(c,root):
    c.group='layout-retention-images';d=c.root/'layout';d.mkdir()
    def source(name,text):return c.write('layout/'+name,text)
    def asm(name,text):
        p=source(name+'.S',text+'\n.section .note.GNU-stack,"",@progbits\n');o=d/(name+'.o');c.run(['gcc','-g','-c',p,'-o',o]);return o
    def link(name,obj,script,extra=(),expected=0,marker=None):
        out=d/(name+'.elf');c.run(['ld','-T',script,*obj,*extra,'-Map='+str(d/(name+'.map')),'-o',out],expected=expected,contains=marker);return out
    def symbols(elf):
        out=c.run(['nm','-n',elf]);return {parts[2]:int(parts[0],16) for line in out.splitlines() if len(parts:=line.split())==3 and re.fullmatch('[0-9a-fA-F]+',parts[0])}
    def binary(elf,name,flags=()):
        out=d/name;c.run(['objcopy','-O','binary',*flags,elf,out]);return out.read_bytes()
    payload='.section .text.entry,"ax"\n.global entry\nentry: .byte 0x90,0x90,0x90,0xc3\n.section .rodata,"a"\n.byte 9,8,7,6\n.section .data,"aw"\n.byte '+','.join(str(x) for x in range(16))+'\n.section .bss,"aw",@nobits\n.space 24\n'
    obj=asm('image',payload)
    script=source('image.ld',r'''ENTRY(entry)
MEMORY { FLASH (rx) : ORIGIN = 0x10000, LENGTH = 0x100
         RAM (rw) : ORIGIN = 0x20000, LENGTH = 0x80 }
SECTIONS {
 .text 0x10000 : { *(.text.entry) *(.text*) *(.rodata*) } > FLASH
 .copy_table 0x10040 : { LONG(LOADADDR(.data)); LONG(ADDR(.data)); LONG(SIZEOF(.data)); LONG(ADDR(.bss)); LONG(SIZEOF(.bss)); } > FLASH
 .data 0x20000 : AT(0x10080) { data_start = .; *(.data*) data_end = .; } > RAM
 data_load = LOADADDR(.data);
 .bss (NOLOAD) : { bss_start = .; *(.bss*) bss_end = .; } > RAM
 ASSERT(LOADADDR(.data)+SIZEOF(.data)<=ORIGIN(FLASH)+LENGTH(FLASH), "FLASH_BUDGET")
 ASSERT(bss_end<=ORIGIN(RAM)+LENGTH(RAM), "RAM_BUDGET")
 ASSERT((data_start & 15)==0, "DATA_ALIGNMENT")
 /DISCARD/ : { *(.note.GNU-stack) }
}
''')
    image=link('image',[obj],script);s=symbols(image);c.run(['readelf','-lW','-SW',image]);c.run(['objdump','-h',image])
    def allocated_names(elf):
        data=elf.read_bytes();h=struct.unpack_from('<HHIQQQIHHHHHH',data,16)
        sh=[struct.unpack_from('<IIQQQQIIQQ',data,h[5]+i*h[10]) for i in range(h[11])];strings=sh[h[12]];names=[]
        for section in sh:
            if section[2]&2:
                at=strings[4]+section[0];names.append(data[at:data.index(0,at,strings[4]+strings[5])].decode('ascii'))
        return set(names)
    approved={'.text','.copy_table','.data','.bss'}
    c.require(allocated_names(image)<=approved,'independent allocated-section allowlist accepts declared image')
    orphan=asm('orphan','.section .surprise,"a"\n.byte 123\n');orphan_image=link('orphan',[obj,orphan],script)
    c.require(allocated_names(orphan_image)-approved=={'.surprise'},'post-link policy rejects unapproved allocatable orphan before use')
    c.require({k:s[k] for k in ('data_start','data_end','data_load','bss_start','bss_end')}=={'data_start':0x20000,'data_end':0x20010,'data_load':0x10080,'bss_start':0x20010,'bss_end':0x20028},'independently derived run/load intervals',s)
    raw=binary(image,'image.bin',['--gap-fill=0']);c.require(len(raw)==0x90 and raw[0x80:]==bytes(range(16)),'load extent includes padding and16 initializers, excludes24 BSS bytes')
    table=struct.unpack_from('<IIIII',raw,0x40);c.require(table==(0x10080,0x20000,16,0x20010,24),'actual startup table carries source/destination/count and zero interval',table)
    # Independent interval model: validate every row before committing any write.
    def initialize(records):
        ram=bytearray([0xA5]*0x80);before=bytes(ram)
        for kind,src,dst,n in records:
            if n<0 or dst<0x20000 or dst>0x20080 or n>0x20080-dst:return False,before
            if kind=='copy' and (src<0x10000 or src>0x10090 or n>0x10090-src):return False,before
            if kind not in ('copy','zero'):return False,before
        for kind,src,dst,n in records:ram[dst-0x20000:dst-0x20000+n]=raw[src-0x10000:src-0x10000+n] if kind=='copy' else bytes(n)
        return True,bytes(ram)
    valid=[('copy',table[0],table[1],table[2]),('zero',0,table[3],table[4])]
    ok,ram=initialize(valid);c.require(ok and ram[:16]==bytes(range(16)) and ram[16:40]==bytes(24) and ram[40:]==bytes([0xA5])*88,'validated table initialization preserves unrelated bytes')
    rejected=[]
    for bad in [('copy',0xffff,0x20000,1),('copy',0x10090,0x20000,1),('zero',0,0x20080,1),('zero',0,0x1ffff,1),('zero',0,0x20000,2**64-1),('wrong',0,0x20000,0)]:
        ok,ram=initialize(valid+[bad]);c.require(not ok and ram==bytes([0xA5])*128,'all-table validation prevents partial writes '+str(bad));rejected.append(bad)
    c.write('layout/startup-model.json',json.dumps(dict(table=table,source_interval=[0x10080,0x10090],destination_interval=[0x20000,0x20010],zero_interval=[0x20010,0x20028],rejected=rejected),indent=2))
    # Address-valued linker symbols: compile and link convention, no fabricated C objects.
    declaration=source('symbols.c','#include <stdint.h>\nextern unsigned char data_start[],data_end[],data_load[];\nint checked_count(uintptr_t *n){uintptr_t a=(uintptr_t)data_start,b=(uintptr_t)data_end;if(b<a)return 0;*n=b-a;return 1;}uintptr_t load_address(void){return (uintptr_t)data_load;}\n')
    decl=d/'symbols.o';c.run(['gcc','-std=c17','-Os','-ffreestanding','-fno-pie','-fno-asynchronous-unwind-tables','-c',declaration,'-o',decl]);c.run(['objdump','-dr',decl]);link('symbol-convention',[obj,decl],script)
    huge=asm('huge',payload.replace('.space 24','.space 128'));link('huge',[huge],script,expected='nonzero',marker='RAM_BUDGET')
    padded=source('padded.ld',script.read_text().replace('AT(0x10080)','AT(0x100a0)'))
    padimage=link('padded',[obj],padded);padraw=binary(padimage,'padded.bin');c.require(len(padraw)==0xb0 and struct.unpack_from('<I',padraw,0x40)[0]==0x100a0,'padding changes copy source and raw extent independently of RAM')
    # An input section is indivisible across two banks, even with enough total space.
    bankobj=asm('bank','.section .bank,"aw",@nobits\n.space 96\n')
    banks=source('banks.ld','MEMORY { R1 (rw) : ORIGIN = 0x20000, LENGTH = 64\n R2 (rw) : ORIGIN = 0x30000, LENGTH = 64 }\nSECTIONS { .bank : { *(.bank) } > R1\n /DISCARD/ : { *(.text*) *(.data*) *(.bss*) *(.note.GNU-stack) } }\n')
    link('bank-fails',[bankobj],banks,expected='nonzero',marker="will not fit in region `R1'")
    split=asm('split','.section .bank.first,"aw",@nobits\n.space 48\n.section .bank.second,"aw",@nobits\n.space 48\n')
    split_script=source('split.ld',banks.read_text().replace('.bank : { *(.bank) } > R1','.first : { *(.bank.first) } > R1\n.second : { *(.bank.second) } > R2'))
    split_image=link('bank-split',[split],split_script);c.run(['readelf','-SW',split_image])
    # Real section-GC and archive extraction are distinct from compiler emission.
    keep=source('registry.ld','SECTIONS { .registry : { registry_start = .; KEEP(*(.registry)) registry_end = .; } } INSERT AFTER .rodata;\n')
    ordinary=source('registry-no-keep.ld',keep.read_text().replace('KEEP(*(.registry))','*(.registry)'))
    entry=source('register.c',r'''typedef int (*fn)(void);
int callback_a(void){return 11;} int callback_b(void){return 22;} int unrelated(void){return 99;}
void registration_anchor(void){}
#ifndef OMIT
__attribute__((used,section(".registry"))) static fn table[]={callback_a,callback_b};
#else
__attribute__((section(".registry"))) static fn table[]={callback_a,callback_b};
#endif
''')
    walker=source('enumerate.c',r'''#include <stdio.h>
typedef int (*fn)(void);extern fn registry_start[],registry_end[];
int main(void){int sum=0;
#ifndef SKIP
for(fn *p=registry_start;p!=registry_end;++p)sum+=(*p)();
#endif
printf("sum=%d count=%ld\n",sum,(long)(registry_end-registry_start));return 0;}
''')
    registry_results=[]
    for lto in (False,True):
        flags=['-flto'] if lto else [];tag='lto' if lto else 'plain';o=d/('register-'+tag+'.o')
        c.run(['gcc','-O2','-ffunction-sections','-fdata-sections',*flags,'-c',entry,'-o',o]);archive=d/('registry-'+tag+'.a');c.run(['ar','rcs',archive,o]);c.run(['nm',o])
        for case,inputs,extra,scriptpath,answer,count in [
          ('unextracted',[archive],[],keep,0,0),('extracted',[archive],['-Wl,-u,registration_anchor'],keep,33,2),
          ('direct',[o],[],keep,33,2),('collected',[o],[],ordinary,0,0),('not-enumerated',[o],['-DSKIP'],keep,0,2)]:
            exe=d/(tag+'-'+case);c.run(['gcc','-O2',*flags,walker,*inputs,*extra,'-Wl,--gc-sections','-Wl,-T,'+str(scriptpath),'-Wl,-Map='+str(exe)+'.map','-o',exe]);out=c.run([exe]);c.require(out==f'sum={answer} count={count}\n','registry stage '+tag+'/'+case,out)
            nm=c.run(['nm',exe]);c.require('unrelated' not in nm,'unrelated function collected '+tag+'/'+case)
            if count:c.require('callback_a' in nm and 'callback_b' in nm,'retained pointer edges preserve both callbacks')
            registry_results.append(dict(lto=lto,case=case,sum=answer,count=count))
        missing=d/('omitted-'+tag+'.o');c.run(['gcc','-O2',*flags,'-DOMIT','-c',entry,'-o',missing]);exe=d/('omitted-'+tag);c.run(['gcc','-O2',*flags,walker,missing,'-Wl,--gc-sections','-Wl,-T,'+str(keep),'-o',exe]);c.run([exe],contains='count=0')
    # Numeric offsets lack relocation edges: keep the table while callbacks vanish.
    numeric=asm('numeric','.section .registry,"a"\n.quad 16,32\n.section .text.numeric_callback,"ax"\n.global numeric_callback\nnumeric_callback: ret\n')
    exe=d/'numeric';c.run(['gcc',walker,numeric,'-DSKIP','-Wl,--gc-sections','-Wl,-T,'+str(keep),'-o',exe]);c.run([exe],contains='count=2');c.require('numeric_callback' not in c.run(['nm',exe]),'numeric values do not create callback reachability edges')
    c.write('layout/registry-stages.json',json.dumps(registry_results,indent=2))
    # BSS budget arithmetic includes alignment and a reserved stack.
    budget=source('budget.ld','MEMORY { RAM (rw) : ORIGIN = 0x20000, LENGTH = 256 }\nSECTIONS { .bss 0x20000 (NOLOAD) : { *(.first) . = ALIGN(32); *(.second) budget_end = .; } > RAM\n stack_bottom = 0x20040; stack_top = 0x20100;\n ASSERT(budget_end <= 0x20040,"STATIC_BUDGET64")\n ASSERT(budget_end <= stack_bottom,"STACK_COLLISION")\n /DISCARD/ : { *(.text*) *(.data*) *(.bss*) *(.note.GNU-stack) } }\n')
    for name,n,m,end,accept in [('b32',32,0,32,True),('b128',128,0,128,False),('aligned',17,16,48,True),('padding-overflow',33,1,65,False)]:
        o=asm(name,f'.section .first,"aw",@nobits\n.space {n}\n.section .second,"aw",@nobits\n'+(f'.space {m}\n' if m else ''))
        e=link(name,[o],budget,expected=0 if accept else 'nonzero',marker=None if accept else 'STATIC_BUDGET64')
        if accept:c.require(symbols(e)['budget_end']==0x20000+end,'independent aligned budget end '+name)
    # Constant merging changes retained bytes without changing program results.
    constsrc=source('constants.c','#include <stdio.h>\nstatic const char a[]="same",b[]="same";static int initialized=7,zero[8];int main(void){printf("%s:%s:%d:%d\\n",a,b,initialized,zero[0]);return 0;}\n')
    const_metrics=[]
    for name,flags in [('keep',['-O0']),('gc',['-O2','-ffunction-sections','-fdata-sections','-Wl,--gc-sections'])]:
        exe=d/('constants-'+name);c.run(['gcc','-g',*flags,constsrc,'-o',exe]);c.run([exe],contains='same:same:7:0');out=c.run(['size','-A',exe]);c.run(['readelf','-SW',exe]);const_metrics.append(dict(configuration=name,sections=out,file_bytes=exe.stat().st_size))
    c.write('layout/resource-metrics.json',json.dumps(dict(synthetic_elf_bytes=image.stat().st_size,raw_load_extent=len(raw),initialized_ram_bytes=16,zero_fill_ram_bytes=24,constants=const_metrics),indent=2))
    # Sparse originals; independent Intel HEX decoder checks length/address/checksum.
    sparse=asm('sparse','.section .low,"a"\n.byte 1,2\n.section .high,"a"\n.byte 254,255\n')
    sparse_script=source('sparse.ld','SECTIONS { .low 0x1000 : { *(.low) } .high 0x1010 : { *(.high) } /DISCARD/ : { *(.note.GNU-stack) } }\n');sparse_elf=link('sparse',[sparse],sparse_script)
    for fill in (0,0xA5):
        blob=binary(sparse_elf,'sparse-'+str(fill)+'.bin',['--gap-fill='+str(fill)]);c.require(blob==b'\x01\x02'+bytes([fill])*14+b'\xfe\xff','independent raw base/fill/span '+str(fill))
    def decode_hex(text):
        decoded={};eof=False
        for line in text.splitlines():
            if eof or not line.startswith(':'):raise ValueError('record framing/EOF')
            b=bytes.fromhex(line[1:])
            if len(b)<5 or len(b)!=b[0]+5 or sum(b)%256:raise ValueError('length/checksum')
            address=int.from_bytes(b[1:3],'big');kind=b[3]
            if kind==0:
                if address+b[0]>65536:raise ValueError('unsupported record address wrap')
                for j,value in enumerate(b[4:-1]):
                    if address+j in decoded:raise ValueError('overlap')
                    decoded[address+j]=value
            elif kind==1:
                if b[0] or address:raise ValueError('EOF shape')
                eof=True
            elif kind==3:
                if b[0]!=4 or address:raise ValueError('start-record shape')
                # The GNU start record is retained but never used as a data address.
            else:raise ValueError('unsupported address/start record type')
        if not eof:raise ValueError('missing EOF')
        return decoded
    hexpath=d/'sparse.hex';c.run(['objcopy','-O','ihex',sparse_elf,hexpath]);hextext=hexpath.read_text();decoded=decode_hex(hextext)
    c.require(decoded=={0x1000:1,0x1001:2,0x1010:254,0x1011:255},'address-bearing sparse data retained independently of14 absent bytes')
    filled=d/'filled.hex';c.run(['objcopy','-O','ihex','--gap-fill=165',sparse_elf,filled]);filled_map=decode_hex(filled.read_text())
    c.require(filled_map==dict(zip(range(0x1000,0x1012),b'\x01\x02'+bytes([165])*14+b'\xfe\xff')),'nonzero HEX fill changes actual records/checksums and addressed byte count')
    lines=hextext.splitlines();bad_checksum=lines[0][:-2]+('00' if lines[0][-2:]!='00' else '01')
    for label,text in [('checksum','\n'.join([bad_checksum,*lines[1:]])),('EOF','\n'.join(lines[:-1])),('overlap','\n'.join([lines[0],*lines]))]:
        try:decode_hex(text)
        except ValueError:pass
        else:raise RuntimeError('bad HEX accepted '+label)
    c.require(True,'HEX checksum missing-EOF and overlap controls rejected')
    # Strip/debuglink identity, address lookup and deliberately mismatched build.
    debugsrc=source('debug.c','int marker(int x){return x+7;}\nint main(void){return marker(35)!=42;}\n')
    dbg=d/'debug';c.run(['gcc','-g','-O0','-fno-pie','-no-pie',debugsrc,'-Wl,--build-id=sha1','-o',dbg]);before=hashlib.sha256(dbg.read_bytes()).hexdigest()
    companion=d/'debug.info';stripped=d/'stripped';c.run(['objcopy','--only-keep-debug',dbg,companion]);c.run(['objcopy','--strip-all',dbg,stripped]);c.run(['objcopy','--add-gnu-debuglink='+str(companion),stripped]);c.run([stripped])
    addr=symbols(dbg)['marker'];c.run(['addr2line','-e',companion,hex(addr)],contains='debug.c:1')
    debuglink=d/'debuglink.bin';c.run(['objcopy','--dump-section','.gnu_debuglink='+str(debuglink),stripped]);contents=debuglink.read_bytes();c.require(struct.unpack_from('<I',contents,len(contents)-4)[0]==zlib.crc32(companion.read_bytes()),'independent debuglink CRC')
    build_id=lambda p:re.search(r'Build ID: ([0-9a-f]+)',c.run(['readelf','-n',p]))[1]
    identity=build_id(dbg);c.require(identity==build_id(companion)==build_id(stripped),'same build identity across strip/debug split')
    changed=d/'different-base';c.run(['gcc','-g','-O0','-fno-pie','-no-pie',debugsrc,'-Wl,-Ttext-segment=0x500000','-Wl,--build-id=sha1','-o',changed]);different=d/'different.info';c.run(['objcopy','--only-keep-debug',changed,different]);c.require(build_id(different)!=identity and symbols(changed)['marker']!=addr,'different base rejects old address/build correlation')
    correct=companion.read_bytes();companion.write_bytes(different.read_bytes())
    try:
        out=c.run(['gdb','-nx','-batch','-iex','set debuginfod enabled off','-iex','set debug-file-directory /nonexistent','-ex','file '+str(stripped),'-ex','info line marker'],env={'DEBUGINFOD_URLS':''},expected=1,contains='CRC mismatch')
        c.require('No symbol table is loaded' in out,'GDB rejects wrong named companion for intended CRC reason',out)
    finally:companion.write_bytes(correct)
    c.require(hashlib.sha256(dbg.read_bytes()).hexdigest()==before,'original debug ELF remains immutable')
    # Checksum has two explicitly different equations; preserve pre/post identities.
    initial=bytearray(range(16));position=7;initial[position]=0;covered=sum(initial)%256
    patched=bytearray(initial);patched[position]=covered
    zerosum=bytearray(initial);zerosum[position]=(-sum(initial))%256
    c.require(sum(patched[i] for i in range(16) if i!=position)%256==patched[position] and sum(zerosum)%256==0,'excluded-field and included-zero-sum checksum equations')
    original=d/'checksum-original.bin';original.write_bytes(initial)
    def patch_once(input_path,identity,output_path,include):
        data=input_path.read_bytes()
        if len(data)!=16 or hashlib.sha256(data).hexdigest()!=identity:raise ValueError('PREPATCH_IDENTITY')
        result=bytearray(data);result[position]=0
        total=0
        for i in range(len(result)):
            if i!=position:total=(total+result[i])&255
        result[position]=(-total)&255 if include else total
        with output_path.open('xb') as stream:stream.write(result)
    prehash=hashlib.sha256(initial).hexdigest();output=d/'checksum-patched.bin';patch_once(original,prehash,output,False)
    c.require(output.read_bytes()==patched,'actual patched file matches independent excluded-field model')
    for inp,out in [(original,output),(output,d/'repatch.bin')]:
        try:patch_once(inp,prehash,out,False)
        except (FileExistsError,ValueError):pass
        else:raise RuntimeError('duplicate/wrong-identity patch accepted')
    zout=d/'checksum-zero-sum.bin';patch_once(original,prehash,zout,True);c.require(zout.read_bytes()==zerosum and original.read_bytes()==initial,'zero-sum output and immutable prepatch artifact')
    manifest=dict(prepatch=initial.hex(),pre_sha256=prehash,excluded_field={'index':position,'post':patched.hex(),'sha256':hashlib.sha256(patched).hexdigest()},zero_sum={'post':zerosum.hex(),'sha256':hashlib.sha256(zerosum).hexdigest()},debug_elf_sha256=before)
    c.write('layout/checksum-manifests.json',json.dumps(manifest,indent=2))
    for suffix,ordinary,variant in [
      ('08.a','Declared FLASH/RAM layout accepted and over-capacity image rejected by RAM_BUDGET','96-byte indivisible section fails64-byte bank; explicitly split48-byte sections use two banks'),
      ('08.b','Actual16-byte data LMA/VMA and24-byte BSS intervals independently checked against binary','32-byte LMA padding changes copy table and raw extent while RAM intervals remain fixed'),
      ('08.c','Address-valued symbol declarations compile/link with checked uintptr_t subtraction under GNU convention','Actual emitted startup table validated transactionally against six invalid range/kind cases'),
      ('09.a','KEEP alone does not extract archive; explicit anchor and direct object preserve table under GC','Direct objects expose sections to KEEP; actual plain/LTO variants distinguish extraction'),
      ('09.b','Two retained pointer callbacks execute11+22; unrelated function eliminated','Retained numeric table creates no callback relocation edge and numeric callback is collected'),
      ('09.c','Independent absent emission/extraction/retention/enumeration cases identify first missing stage','Actual GCC LTO repeats registry experiment; slim object evidence differs from final symbols'),
      ('10.a','32-byte BSS accepts and128 rejects64-byte budget; aligned17+16 ends at48','Reserved stack boundary and padding-overflow33+1 reject static collision'),
      ('10.b','Actual text/constants/data/BSS placement and40-byte runtime residency separated from16-byte initializers','Equivalent constant program under optimization/GC records actual retained section deltas'),
      ('10.c','ELF byte size,144-byte raw extent and40-byte RAM requirements reported as distinct metrics','Sparse Intel HEX preserves four addressed bytes without representing14 zero-gap bytes'),
      ('12.a','Original0x1000/0x1010 sparse bytes produce independently checked18-byte zero-filled raw image and HEX','Nonzero0xA5 gap fill preserves extent and changes materialized gap bytes'),
      ('12.b','Matching debug companion build ID/CRC resolves marker address; executable runs stripped','Different link base produces different ID/address; GDB rejects wrong companion CRC'),
      ('12.c','16-byte original patches only excluded checksum byte and preserves pre/post hashes','Included-field zero-sum uses additive inverse; original debug ELF hash remains unchanged')]:
        c.observe('T09.'+suffix,ordinary,['layout-retention-images']);c.observe('T09.'+suffix,variant,['layout-retention-images'],True,limitations=['Synthetic addresses are never executed and do not establish a board memory map.'])
