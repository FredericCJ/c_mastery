"""Positive/four-negative origin audits and conditional callback boundaries."""
import hashlib,importlib.util,json,re
def run(c,root,args):
 c.group='freestanding-closure';pub=root/'labs/machine_image/freestanding_depth';private=root/'assessor/legacy_systems/freestanding_depth'
 spec=importlib.util.spec_from_file_location('image_audit',private/'image_audit.py');audit=importlib.util.module_from_spec(spec);spec.loader.exec_module(audit)
 start=c.write('closure/start.S','.text\n.global _start\n_start:\n andq $-16,%rsp\n call run\n movl %eax,%edi\n movl $60,%eax\n syscall\n.section .note.GNU-stack,"",@progbits\n')
 positive=c.write('closure/positive.c','#include "free.h"\nint run(void){unsigned char a[16]={0},b[16]={7};fr_move(a,b,16);return a[0]==7?42:1;}\n')
 forbidden=c.write('closure/forbidden.c','__attribute__((noinline)) int forbidden_helper(int x){volatile int y=x;return y+1;}\n')
 call=c.write('closure/call.c','extern int forbidden_helper(int);int run(void){return forbidden_helper(41);}\n')
 missing=c.write('closure/missing.c','extern int absent_helper(void);int run(void){return absent_helper();}\n')
 recursive=c.write('closure/recursive.S','.text\n.global run\nrun:\n movl $42,%eax\n ret\n.global memcpy\nmemcpy:\n call memcpy\n ret\n.section .note.GNU-stack,"",@progbits\n')
 hosted=c.write('closure/hosted.c','#include <stdio.h>\nint main(void){puts("HOSTED");return 0;}\n')
 prior=c.root/'prior-audit';c.run(['gcc','-std=c17','-O2',root/'assessor/machine_image/audit.c','-o',prior])
 results=[]
 def bundle(image,mapfile,allowed,provider=(),map_format='bfd',inputs=None):
  inputs=inputs or {str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in allowed if hasattr(p,'read_bytes') and p.is_file()}
  policy=dict(allowed_loads=list(map(str,allowed)),forbidden_symbols=['forbidden_helper'],provider_symbols=list(provider),map_format=map_format,approved_build_inputs=inputs,scope='Selected GNU bfd/lld x86-64 final image, direct call graph and explicit supplied origin policy; no inlining/indirect semantic proof');text=json.dumps(policy,sort_keys=True);report={}
  for name,argv in [('symbols',['nm','-P','--defined-only',image]),('undefined',['nm','-u',image]),('dynamic',['readelf','-d',image]),('segments',['readelf','-l',image]),('sections',['readelf','-SW',image]),('disassembly',['objdump','-drw',image])]:
   value=c.run(argv);report[name]=dict(text=value,exit=0,sha256=audit.digest(value))
  value=mapfile.read_text();report['map']=dict(text=value,exit=0,sha256=audit.digest(value));return dict(schema=1,binary_sha256=hashlib.sha256(image.read_bytes()).hexdigest(),policy=policy,policy_text=text,policy_sha256=audit.digest(text),reports=report,build_inputs=inputs)
 for cc in ('gcc','clang'):
  for opt in ('O0','O2','Os'):
   for kind in ('positive','resolved','recursive'):
    stem=cc+'-'+opt+'-'+kind;folder=c.root/'closure'/stem;folder.mkdir();sources=[start,positive,private/'bytes.c',private/'memory_provider.c'] if kind=='positive' else [start,call,forbidden] if kind=='resolved' else [start,recursive];objects=[]
    for source in sources:
     obj=folder/(source.stem+'.o');c.run([cc,'-std=c17','-'+opt,'-ffreestanding','-fno-builtin','-fno-pie','-I'+str(pub),'-c',source,'-o',obj]);c.run(['nm','-u',obj]);objects.append(obj)
    image=folder/'image';mapfile=folder/'image.map';c.run([cc,'-nostdlib','-static','-no-pie',*objects,'-Wl,-Map='+str(mapfile),'-o',image]);data=bundle(image,mapfile,objects,['memcpy','fr_move','fr_copy','fr_set']);errors=audit.validate(data,data['binary_sha256'],data['policy_sha256']);expected=[] if kind=='positive' else ['resolved forbidden symbol'] if kind=='resolved' else ['provider call cycle']
    c.require(all(e in errors for e in expected) and bool(errors)==bool(expected),'dependency control rejected for intended property' if expected else 'positive origin audit accepted',dict(profile=stem,errors=errors))
    if kind!='recursive':c.run([image],expected=42)
    # Original accepted auditor cannot see a statically resolved forbidden
    # provider: execute unchanged baseline to demonstrate the exact gap.
    if kind=='resolved':
     paths=[]
     for field in ('undefined','dynamic','segments'):paths.append(c.write('closure/'+stem+'/'+field+'.txt',data['reports'][field]['text']))
     c.run([prior,*paths]);c.require(not data['reports']['undefined']['text'].strip(),'resolved forbidden negative has no undefined symbols',stem)
    results.append(dict(profile=stem,rejections=errors,binary_sha256=data['binary_sha256']))
    if kind=='positive' and opt=='O2':
     from copy import deepcopy
     for name,mutate,expected_error in [
      ('missing',lambda d:d['reports'].pop('sections'),'missing report'),
      ('truncated',lambda d:d['reports']['segments'].update(text='cut'),'invalid report segments'),
      ('report edit',lambda d:d['reports']['map'].update(text=d['reports']['map']['text']+'extra\n'),'report identity map'),
      ('tool fail',lambda d:d['reports']['undefined'].update(exit=1),'invalid report undefined'),
      ('overlong',lambda d:d['reports']['dynamic'].update(text='x'*8193+'\n'),'invalid report dynamic')]:
      bad=deepcopy(data);mutate(bad);c.require(expected_error in audit.validate(bad,data['binary_sha256'],data['policy_sha256']),'bounded evidence rejection '+name,cc)
     c.require('binary identity' in audit.validate(data,'0'*64,data['policy_sha256']),'changed binary invalidates receipt',cc);c.require('policy identity' in audit.validate(data,data['binary_sha256'],'0'*64),'changed allowlist invalidates receipt',cc)
     stripped=folder/'stripped';c.run(['strip','--strip-all','-o',stripped,image]);bad=bundle(stripped,mapfile,objects);c.require('missing symbol table' in audit.validate(bad,bad['binary_sha256'],bad['policy_sha256']),'stripped image fails closed',cc)
     c.write('closure/'+stem+'/bundle.json',json.dumps(data,indent=2))
  # An unresolved relocatable object is a separate negative category: not an
  # executable and never allowed through a final-image acceptance path.
  obj=c.root/(cc+'-missing.o');c.run([cc,'-ffreestanding','-c',missing,'-o',obj]);out=c.run(['nm','-u',obj]);c.require('absent_helper' in out,'unresolved helper specimen is genuine',cc);c.run([cc,'-nostdlib','-static','-no-pie',start,obj,'-o',c.root/(cc+'-missing')],expected='nonzero',contains='absent_helper')
  image=c.root/(cc+'-dynamic');mapfile=c.root/(cc+'-dynamic.map');c.run([cc,'-O2',hosted,'-Wl,-Map='+str(mapfile),'-o',image]);data=bundle(image,mapfile,[]);errors=audit.validate(data,data['binary_sha256'],data['policy_sha256']);c.require('external runtime' in errors,'actual dynamic dependency rejected',cc)
 # LTO collapses object boundaries. Bind original input identities as well as
 # retained generated objects; the map alone cannot recover inlined origins.
 inline=c.write('closure/inline.c','int inline_provider(int x){volatile int y=x;return y+1;}\n')
 inline_call=c.write('closure/inline-call.c','extern int inline_provider(int);int run(void){return inline_provider(41);}\n')
 guard=c.write('closure/guard.c','__attribute__((used)) unsigned long __stack_chk_guard=0x7391a64b;__attribute__((used,noreturn,no_stack_protector)) void __stack_chk_fail(void){__asm__ volatile("movl $99,%%edi;movl $60,%%eax;syscall":::"rax","rdi","rcx","r11","memory");__builtin_unreachable();}\n')
 for cc in ('gcc','clang'):
  for mode in ('lto','stack','stack-lto'):
   for kind in ('positive','inline-forbidden'):
    folder=c.root/'closure'/(cc+'-'+mode+'-'+kind);folder.mkdir();image=folder/'image';mapfile=folder/'image.map';sources=[start,positive,private/'bytes.c',private/'memory_provider.c'] if kind=='positive' else [start,inline_call,inline]
    if 'stack' in mode:sources.append(guard)
    objects=[];opt=['-O2']+(['-flto'] if 'lto' in mode else []);link=['-fuse-ld=/home/frede/cmastery-b06-tools/usr/bin/ld.lld-21'] if cc=='clang' and 'lto' in mode else [];instrument=['-fstack-protector-all','-mstack-protector-guard=global'] if 'stack' in mode else []
    for source in sources:
     obj=folder/(source.stem+'.o');c.run([cc,'-std=c17',*opt,*instrument,'-ffreestanding','-fno-builtin','-fno-pie','-I'+str(pub),'-c',source,'-o',obj]);objects.append(obj)
    c.run([cc,*opt,*link,*(['-Wl,--save-temps'] if link else []),'-save-temps','-nostdlib','-static','-no-pie',*objects,'-Wl,-Map='+str(mapfile),'-o',image]);c.run([image],expected=42)
    mapping=mapfile.read_text();map_format='lld' if link else 'bfd';allowed=list(objects)
    generated=list(folder.glob('image*.o'))
    if 'lto' in mode:
     c.require(bool(generated),'LTO final machine objects retained',dict(cc=cc,mode=mode))
     for obj in generated:c.run(['nm','-u',obj]);c.run(['objdump','-dr',obj])
     allowed+=generated
    if link:allowed+=['<internal>']
    inputs={str(s):hashlib.sha256(s.read_bytes()).hexdigest() for s in sources};data=bundle(image,mapfile,allowed,['memcpy','fr_copy','fr_move','fr_set'],map_format,inputs)
    if kind=='inline-forbidden':
     data['policy']['approved_build_inputs']={k:v for k,v in inputs.items() if k!=str(inline)};data['policy_text']=json.dumps(data['policy'],sort_keys=True);data['policy_sha256']=audit.digest(data['policy_text'])
    errors=audit.validate(data,data['binary_sha256'],data['policy_sha256']);c.require(('unapproved build input' in errors) if kind!='positive' else not errors,'LTO/instrumentation origin policy discriminates forbidden input',dict(cc=cc,mode=mode,kind=kind,errors=errors))
    if 'stack' in mode:c.require('__stack_chk_fail' in data['reports']['disassembly']['text'],'final stack instrumentation is retained',dict(cc=cc,mode=mode))
    c.write('closure/'+folder.name+'/bundle.json',json.dumps(data,indent=2));results.append(dict(profile=folder.name,rejections=errors,binary_sha256=data['binary_sha256']))
 c.write('closure/control-matrix.json',json.dumps(results,indent=2))
 # Callback contract boundary: original component imports a callback whose
 # actual provider is separately identified. Its allocation claim changes.
 callback=c.write('closure/callback.c','typedef void *(*acquire_fn)(void*,unsigned);int component(acquire_fn fn,void *ctx){void*p=fn(ctx,8);return p?0:1;}\n')
 harness=c.write('closure/callback-host.c','#include <stdlib.h>\n#include <stdio.h>\nextern int component(void*(*)(void*,unsigned),void*);static void*get(void*ctx,unsigned n){void**p=ctx;*p=malloc(n);return *p;}int main(void){void*p=0;int rc=component(get,&p);free(p);puts("CALLBACK:allocation-provider");return rc;}\n')
 obj=c.root/'callback.o';c.run(['gcc','-std=c17','-O2','-ffreestanding','-c',callback,'-o',obj]);out=c.run(['nm','-u',obj]);c.require(not out.strip(),'callback allocation is absent from component undefined symbols');c.run(['objdump','-dr',obj]);exe=c.root/'callback';c.run(['gcc',harness,obj,'-o',exe]);c.run([exe],contains='CALLBACK:allocation-provider');c.run(['nm','-u',exe]);c.write('closure/callback-contract.json',json.dumps(dict(producer='Original host allocation callback',consumer='component',ownership='Callback grants8-byte allocation to host caller; host frees exactly once after component returns',conditional_claim='Component has no direct allocator import but whole invocation acquires heap storage',limits='No proof of unknown future callback behavior'),indent=2))
 for oid,ordinary,changed in [('01.a','Five environment/profile claims are assessed independently against actual parser/byte/raw/runtime/callback specimens','Allocating callback changes allocation-free claim without introducing a direct malloc symbol in component'),('01.b','Hosted oracle and restricted archive/image have separately recorded dependencies','Same original byte primitive executes inside strict no-library native and M4 closures'),('01.c','Headers, object helpers, final origins, syscalls, storage and callback ownership are distinct manifest fields','Actual producer callback contract is bound and executed; unknown callbacks remain conditional'),('06.c','GCC/Clang O0/O2/Os final direct-call graph rejects original recursive provider before execution','Changed compiler and optimization require separate object/final inspections; historical receipt grants no new profile'),('09.a','Actual dynamic, unresolved and statically resolved negatives exercise different audit layers','Stripped symbols reject rather than masquerade as empty dependency evidence'),('09.b','Object imports and final map/call origins discriminate a resolved forbidden provider','LTO/instrumentation need their separately retained final-image/origin evidence; indirect/inlined semantics remain bounded'),('09.c','Each declared audited source/configuration tuple carries its own reports and negative controls','Changing binary or allowlist invalidates prior receipt; malformed/truncated/tool-failed reports reject')]:
  c.observe('T11.'+oid,ordinary,['freestanding-closure']);c.observe('T11.'+oid,changed,['freestanding-closure'],True)
