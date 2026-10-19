"""Five real component boundaries; declarations and linkage have separate evidence."""
from pathlib import Path
import hashlib,json,shlex

def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()

def run(c,root,args):
 c.group='freestanding-closure';pub=root/'labs/machine_image/freestanding_depth';private=root/'assessor/legacy_systems/freestanding_depth';startup=root/'assessor/legacy_systems/startup_depth/decimal.c';startpub=root/'labs/machine_image/startup_depth'
 parser=c.write('manifest/parser.c','#include "startup.h"\n#include <stdio.h>\nint main(void){uint32_t value=93;if(sd_parse_u32((const unsigned char*)"4294967295",10,&value)!=SD_OK||value!=UINT32_MAX)return 1;value=93;if(sd_parse_u32((const unsigned char*)"4294967296",10,&value)!=SD_RANGE||value!=93)return 2;puts("PARSER:valid-and-overflow-preserved");return 0;}\n')
 obj=c.root/'manifest/parser.o';exe=c.root/'manifest/parser';mapping=c.root/'manifest/parser.map'
 default=c.root/'manifest/parser-default.o';c.run(['gcc','-std=c17','-O2','-I'+str(startpub),'-c',startup,'-o',default]);c.run(['nm','-u',default]);c.run(['objdump','-dr',default])
 # This distribution enables stack protection by default in the translation
 # unit's formatters. Keep that observation, then declare the bare component
 # policy explicitly; the hosted verification process keeps its normal CRT.
 c.run(['gcc','-std=c17','-O2','-fno-stack-protector','-I'+str(startpub),'-c',startup,'-o',obj]);undefined=c.run(['nm','-u',obj]);c.require(not undefined.strip(),'actual T10 parser object has no runtime import')
 c.run(['gcc','-std=c17','-O2','-I'+str(startpub),parser,obj,'-Wl,-Map='+str(mapping),'-o',exe]);c.run([exe],contains='PARSER:valid-and-overflow-preserved')
 byte=c.root/'manifest/bytes.o';archive=c.root/'manifest/bytes.a';c.run(['gcc','-std=c17','-O2','-ffreestanding','-fno-builtin','-I'+str(pub),'-c',private/'bytes.c','-o',byte]);c.run(['ar','rcs',archive,byte]);c.run(['ar','t',archive]);listing=c.run(['nm','-u',archive]);c.require(not any(line.strip() and not line.strip().endswith(':') for line in listing.splitlines()),'actual byte archive has no unresolved implementation')
 # -M includes compiler/system headers too: their availability is not linkage.
 def headers(source,include,flags=()):
  raw=c.run(['gcc','-std=c17',*flags,'-I'+str(include),'-M',source]);paths=shlex.split(raw.replace('\\\n',' ').split(':',1)[1]);return [{'path':p,'sha256':digest(p)} for p in paths]
 header_sets={'parser':headers(startup,startpub)+headers(parser,startpub),'bytes':headers(private/'bytes.c',pub,('-ffreestanding','-fno-builtin')),'runtime':headers(c.root/'target/native.c',pub,('-ffreestanding','-fno-builtin')),'callback':headers(c.root/'closure/callback-host.c',pub)+headers(c.root/'closure/callback.c',pub)}
 header_sets['raw']=header_sets['bytes']+headers(c.root/'lowering/memory.c',pub,('-ffreestanding','-fno-builtin'))+headers(private/'memory_provider.c',pub,('-ffreestanding','-fno-builtin'))
 rows=[]
 def record(name,boundary,artifact,objects,source_headers,compile_profile,syscalls,callbacks,storage,classes,mapfile=None):
  imports=[]
  for item in objects:imports.append(dict(path=str(item),sha256=digest(item),undefined=c.run(['nm','-u',item]),disassembly=c.run(['objdump','-dr',item])))
  final=dict(path=str(artifact),sha256=digest(artifact),undefined=c.run(['nm','-u',artifact]))
  if artifact.suffix!='.a':
   final.update(symbols=c.run(['nm','-P',artifact]),dynamic=c.run(['readelf','-d',artifact]),segments=c.run(['readelf','-l',artifact]),disassembly=c.run(['objdump','-dr',artifact]))
  else:final['archive_members']=c.run(['ar','t',artifact])
  if mapfile is not None:final['map']=dict(path=str(mapfile),sha256=digest(mapfile))
  # Capture these large reports separately, keeping the manifest concise enough
  # for embedded receipt content. Each report remains source/hash bound.
  evidence=c.write('manifest/'+name+'-reports.json',json.dumps(dict(objects=imports,final=final),indent=2))
  rows.append(dict(id=name,boundary=boundary,artifact_sha256=final['sha256'],report=dict(path=evidence.relative_to(c.root).as_posix(),sha256=digest(evidence)),headers=source_headers,compile_profile=compile_profile,syscalls=syscalls,callback_contract=callbacks,storage_acquisition=storage,classifications=classes,limits='Trusted build capture and named providers only. This is not a proof about arbitrary callees, compiler correctness or all startup actions of a hosted process.'))
 record('hosted-parser','Original T10 decimal object and the separately hosted test process',exe,[obj],header_sets['parser'],'C17 hosted GCC O2 fno-stack-protector for component; default-instrumented object separately retained', 'Parser makes none; hosted puts/startup delegate OS interaction to explicitly linked libc. No exhaustive libc syscall trace claimed.', 'No parser callback', 'Parser uses caller output and stack; hosted harness startup/libc allocation is not proved absent.',dict(hosted='yes: harness',freestanding='no: selected compilation',libc_free='parser yes under declared component flags; whole process no',allocation_free='parser yes; whole process unknown',custom_startup='no: compiler-selected CRT'),mapping)
 record('byte-archive','Original restricted fr_* byte component; archive is not a whole program',archive,[byte],header_sets['bytes'],'C17 freestanding fno-builtin GCC O2','None in original component','None','Caller-owned live spans and stack only',dict(hosted='not applicable: component',freestanding='yes: selected compilation',libc_free='yes: component closure',allocation_free='yes: component',custom_startup='not applicable: no entry'))
 record('raw-image','Original aggregate copy/compare image and original byte providers',c.root/'gcc-O2-no-builtin',[c.root/'gcc-O2-no-builtin.o'],header_sets['raw'],'C17 freestanding fno-builtin GCC O2; static nostdlib','Linux x86-64 raw exit(60); kernel creates address space and entry stack','None','Kernel-provided stack; automatic arrays; no allocator',dict(hosted='no: selected custom environment',freestanding='yes: selected compilation',libc_free='yes: selected whole image',allocation_free='yes: no dynamic acquisition after entry',custom_startup='yes: original _start'),c.root/'gcc-O2-no-builtin.map')
 record('explicit-runtime','Original no-library entry with explicitly permitted compiler-rt helper',c.root/'gcc-O2-compiler-rt',list((c.root/'objects/gcc-O2-compiler-rt').glob('*.o')),header_sets['runtime'],'C17 freestanding GCC O2 plus explicitly named LLVM compiler-rt archive','Linux raw exit(60)','None','Kernel-created stack and static case table; selected unsigned division helper uses no allocator',dict(hosted='no: selected custom environment',freestanding='yes: selected compilation',libc_free='yes: libc excluded; compiler-rt explicitly allowed',allocation_free='yes: bounded selected helper',custom_startup='yes: original _start'),c.root/'gcc-O2-compiler-rt.map')
 runtime=next(r['stdout'].strip() for r in c.commands if '--print-libgcc-file-name' in r['argv'])
 rows[-1]['binary_provider']=dict(path=runtime,sha256=digest(runtime),headers='Exact prebuilt archive compilation headers cannot be recovered from this invocation. Selected upstream implementation/declarations were separately inspected; binary reproduction is not claimed.')
 record('callback-provider','Component object and actual original hosted allocating callback producer',c.root/'callback',[c.root/'callback.o'],header_sets['callback'],'Component freestanding GCC O2; hosted harness', 'Component has no direct syscall; producer uses malloc/free and hosted output',json.loads((c.root/'closure/callback-contract.json').read_text()),'Actual callback acquires 8 bytes; host owner frees once',dict(hosted='yes: harness; component alone not a program',freestanding='yes: component compilation',libc_free='component direct imports yes; invocation no',allocation_free='no: actual provider allocates',custom_startup='no: harness CRT'))
 c.require(len(rows)==5 and all(r['headers'] and r['callback_contract'] and r['storage_acquisition'] for r in rows),'five component manifests bind actual declarations and providers')
 c.write('manifest/dependencies.json',json.dumps(rows,indent=2))
