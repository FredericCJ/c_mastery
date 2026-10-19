"""Original symbol ownership, archive extraction and relocation experiments."""
import json,struct

def run(c,root):
    c.group='symbols-archives-relocations';directory=c.root/'link';directory.mkdir()
    def obj(name,body,extra=()):
        source=c.write('link/'+name+'.c',body);output=directory/(name+'.o')
        c.run(['gcc','-std=c17','-Wall','-Wextra','-Werror','-O0','-fno-pie','-ffunction-sections','-fdata-sections',*extra,'-c',source,'-o',output]);return output
    def link(name,inputs,extra=(),expected=0,marker=None):
        exe=directory/name;c.run(['gcc','-no-pie',*map(str,inputs),*extra,'-Wl,-Map='+str(directory/(name+'.map')),'-o',exe],expected=expected,contains=marker);return exe
    def archive(name,objects):
        target=directory/name;c.run(['ar','rcs',target,*objects]);c.run(['ar','t',target]);c.run(['nm','-s',target]);return target
    for cc in ('ld','ar','nm'):c.run([cc,'--version'])
    a=obj('a','int b(void);int a(void){return b();}\n');b=obj('b','int b(void){return 42;}\n');extra=obj('unused','int unused(void){return 99;}\n')
    start=obj('main','int a(void);int main(void){return a()!=42;}\n')
    liba=archive('libA.a',[a,extra]);libb=archive('libB.a',[b])
    good=link('ordered',[start,liba,libb]);c.run([good]);symbols=c.run(['nm',good])
    c.run([link('two-objects-one-archive',[start,a,libb])])
    c.require(' unused' not in symbols and ' a' in symbols and ' b' in symbols,'demand extraction excludes unrelated member',symbols)
    link('wrong-order',[start,libb,liba],expected='nonzero',marker='undefined reference to `b')
    # An already satisfied weak definition can prevent strong archive extraction.
    weak=obj('weak','__attribute__((weak)) int choice(void){return 11;}\n')
    strong=obj('strong','int choice(void){return 22;}\n')
    pick=obj('pick','int choice(void);int main(void){return choice();}\n')
    for name,inputs,answer in [('weak-strong',[pick,weak,strong],22),('strong-weak',[pick,strong,weak],22),('unextracted-strong',[pick,weak,archive('libChoice.a',[strong])],11)]:
        exe=link(name,inputs);c.run([exe],expected=answer);c.run(['nm',exe]);c.run(['readelf','-sW',exe])
    # Tentative definitions are a tool-extension experiment, not valid portable
    # permission for multiple external definitions across a C program.
    tentative_a=obj('tentative_a','int shared;int read_a(void){return shared;}\n')
    tentative_b=obj('tentative_b','int shared;int read_b(void){return shared;}\n')
    cm=obj('common_main','int read_a(void);int read_b(void);int main(void){return read_a()+read_b();}\n')
    link('no-common',[cm,tentative_a,tentative_b],expected='nonzero',marker='multiple definition of `shared')
    ca=obj('common_a','int shared;int read_a(void){return shared;}\n',['-fcommon'])
    cb=obj('common_b','int shared;int read_b(void){return shared;}\n',['-fcommon'])
    c.run(['nm',ca],contains=' C shared');c.run([link('common-merge',[cm,ca,cb])])
    ia=obj('initialized_a','int shared=1;int read_a(void){return shared;}\n',['-fcommon'])
    ib=obj('initialized_b','int shared=2;int read_b(void){return shared;}\n',['-fcommon'])
    link('initialized-conflict',[cm,ia,ib],expected='nonzero',marker='multiple definition of `shared')
    la=obj('local_a','#ifndef LOCAL\n#define LOCAL\n#endif\nLOCAL int helper(void){return 1;}int first(void){return helper();}\n')
    lb=obj('local_b','#ifndef LOCAL\n#define LOCAL\n#endif\nLOCAL int helper(void){return 2;}int second(void){return helper();}\n')
    lm=obj('local_main','int first(void);int second(void);int main(void){return first()+second()!=3;}\n')
    link('duplicate',[lm,la,lb],expected='nonzero',marker='multiple definition of `helper')
    for name in ('local_a','local_b'):
        c.run(['gcc','-O0','-DLOCAL=static','-c',directory/(name+'.c'),'-o',directory/(name+'.o')])
    c.run([link('local-repair',[lm,la,lb])]);c.run(['nm',la],contains=' t helper')
    public=obj('public','int helper(void){return 3;}\n')
    first=obj('first','int helper(void);int first(void){return helper();}\n')
    second=obj('second','int helper(void);int second(void){return helper();}\n')
    pmain=obj('public_main','int first(void);int second(void);int main(void){return first()+second()!=6;}\n')
    c.run([link('public-authority',[pmain,first,second,public])])
    exports=c.write('link/exports.map','{global: first;second;local:*;};\n')
    shared=directory/'libscope.so';c.run(['gcc','-shared','-fPIC',directory/'first.c',directory/'second.c',directory/'public.c','-Wl,--version-script='+str(exports),'-o',shared])
    dyn=c.run(['nm','-D','--defined-only',shared]);c.require(' helper' not in dyn and ' first' in dyn and ' second' in dyn,'dynamic export policy distinct from C internal linkage',dyn)
    # Three-member archive removal is explicit: ar r does not remove omitted names.
    third=obj('third','int third(void){return 7;}\n');three=archive('libThree.a',[b,extra,third]);c.run(['ar','rcs',three,b,third]);names=c.run(['ar','t',three]);c.require('unused.o' in names,'archive update retains omitted stale member',names)
    bm=obj('bmain','int b(void);int main(void){return b()!=42;}\n');be=link('three-one-needed',[bm,three]);c.run([be]);bs=c.run(['nm',be]);c.require(' b' in bs and ' unused' not in bs and ' third' not in bs,'three-member archive extracts only demanded member')
    c.run(['ar','d',three,'unused.o']);c.require('unused.o' not in c.run(['ar','t',three]),'explicit archive member retirement')
    # A backward dependency requires another archive pass or a bounded group.
    tail=obj('tail','int tail(void){return 42;}\n');back=obj('back','int tail(void);int b(void){return tail();}\n')
    cyclic=archive('libCycle.a',[a,tail]);backlib=archive('libBack.a',[back])
    link('backward-missing',[start,cyclic,backlib],expected='nonzero',marker='undefined reference to `tail')
    c.run([link('repeated',[start,cyclic,backlib,cyclic])]);c.run([link('grouped',[start],['-Wl,--start-group',str(cyclic),str(backlib),'-Wl,--end-group'])])
    # Constructor registration has no demand edge until the member is extracted.
    reg=obj('registration','extern int registered;void register_anchor(void){}\nstatic void __attribute__((constructor)) install(void){registered=7;}\n')
    dead=obj('registration_dead','char dead_payload[4096];int dead_code(void){return 77;}\n')
    rmain=obj('registration_main','int registered;int main(void){return registered;}\n');reglib=archive('libRegistration.a',[reg,dead])
    for name,options,answer in [('normal',[],0),('minimal-group',['-Wl,--start-group',str(reglib),'-Wl,--end-group'],0),('targeted',['-Wl,-u,register_anchor'],7),('whole',['-Wl,--whole-archive',str(reglib),'-Wl,--no-whole-archive'],7)]:
        inputs=[rmain,reglib] if name in ('normal','targeted') else [rmain]
        exe=link('registration-'+name,inputs,options);c.run([exe],expected=answer);c.run(['size','-A',exe]);c.run(['nm',exe])
    clash=obj('clash','int registered=99;\n');conflict=archive('libConflict.a',[reg,clash])
    c.run([link('targeted-conflict-safe',[rmain,conflict],['-Wl,-u,register_anchor'])],expected=7)
    link('whole-conflict',[rmain],['-Wl,--whole-archive',str(conflict),'-Wl,--no-whole-archive'],expected='nonzero',marker='multiple definition of `registered')
    # Actual RELA fields with controlled symbol and place; never execute this layout.
    asm=c.write('link/reloc.S','.section .values,"a"\n.global abs32,pc32\nabs32: .long target\npc32: .long target - . - 4\n.section .note.GNU-stack,"",@progbits\n')
    robj=directory/'reloc.o';c.run(['gcc','-c',asm,'-o',robj]);c.run(['readelf','-rW',robj])
    for target in (0x1000,0x2000):
        script=c.write('link/map-'+str(target)+'.ld',f'SECTIONS {{ .values 0x100c : {{ *(.values) }} target = {target}; }}\n')
        elf=directory/('reloc-'+str(target)+'.elf');blob=directory/('reloc-'+str(target)+'.bin')
        c.run(['ld','-T',script,robj,'-o',elf]);c.run(['objcopy','-O','binary','--only-section=.values',elf,blob])
        c.require(blob.read_bytes()==struct.pack('<Ii',target,target-4-0x1010),'independent absolute and PC-relative fields',blob.read_bytes().hex())
    script=c.write('link/overflow.ld','SECTIONS { .values 0x100c : { *(.values) } target = 0x100000000; }\n')
    c.run(['ld','-T',script,robj,'-o',directory/'overflow.elf'],expected='nonzero',contains='relocation truncated to fit')
    rel32=directory/'rel32.o';c.run(['gcc','-m32','-c',asm,'-o',rel32]);c.run(['readelf','-rW',rel32],contains='R_386_PC32')
    relbytes=directory/'rel32.bin';c.run(['objcopy','-O','binary','--only-section=.values',rel32,relbytes]);c.require(relbytes.read_bytes()[4:]==struct.pack('<i',-4),'REL addend is encoded in original field',relbytes.read_bytes().hex())
    limits=[]
    for signed,width in [(False,32),(True,32),(True,8),(False,8)]:
        lo=-(1<<(width-1)) if signed else 0;hi=(1<<(width-1))-1 if signed else (1<<width)-1
        for value in (lo-1,lo,hi,hi+1,-20,236):limits.append(dict(signed=signed,width=width,value=value,accepted=lo<=value<=hi,encoding=(value%(1<<width)).to_bytes(width//8,'little').hex() if lo<=value<=hi else None))
    c.require(next(x for x in limits if x['signed'] and x['width']==8 and x['value']==-20)['accepted'] and not next(x for x in limits if x['signed'] and x['width']==8 and x['value']==236)['accepted'],'same truncated byte different admissibility')
    c.write('relocation-boundaries.json',json.dumps(limits,indent=2))
    trace={'ordered':[{'input':'main.o','unresolved':['a']},{'input':'A(a.o)','unresolved':['b']},{'input':'B(b.o)','unresolved':[]}],
      'reversed':[{'input':'main.o','unresolved':['a']},{'input':'B.a','extracted':[],'unresolved':['a']},{'input':'A(a.o)','unresolved':['b']}],
      'weak_then_archive':'weak definition resolves choice before archive; strong member not extracted',
      'backward':'A extracts a; B extracts b and adds tail; second A scan/group extracts tail',
      'registration':'group repeats only unresolved searches; it does not force a constructor-only member'}
    c.write('link-traces.json',json.dumps(trace,indent=2))
    for suffix,ordinary,variant in [
      ('02.a','Actual local/global/weak/undefined/common symbols; no-common rejection and labeled common extension','Initialized external definitions conflict despite -fcommon'),
      ('02.b','Unresolved-set trace plus actual link maps identify a/b and strong-vs-weak origin','Strong definition remains unextracted when prior weak definition satisfied demand'),
      ('02.c','Duplicate helper diagnostic repaired with static ownership; exported set inspected separately','Single authoritative public helper executes through two clients'),
      ('03.a','Three relocatable archive members and symbol index inspected','ar update retains omitted stale member; explicit delete removes it'),
      ('03.b','main,A,B executes42; reversed archive order fails specifically onb','Backward reference succeeds with repeated A and minimal group'),
      ('03.c','Normal/minimal group/targeted/whole extraction distinguish registration and unrelated bytes','Whole-archive introduces unused conflicting definition; targeted extraction remains valid'),
      ('04.a','Actual S=4096,A=-4,P=4112 field encodes-20 alongside absolute4096','Actual i386 REL object retains in-field addend-4'),
      ('04.b','Actual absolute32 and PC32 bytes match independent encodings for two target regions','Moving only target changes displacement; no common-load-base assumption applies'),
      ('04.c','Synthetic signed8 bounds reject236 while accepting-20; actual out-of-range layout linker rejects','Signed/unsigned8/32 interval table derives bounds before encoding')]:
        c.observe('T09.'+suffix,ordinary,['symbols-archives-relocations','link-traces.json']);c.observe('T09.'+suffix,variant,['symbols-archives-relocations','relocation-boundaries.json'],True)
