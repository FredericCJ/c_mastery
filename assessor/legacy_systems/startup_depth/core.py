"""Hosted independent decimal oracle, bounded entry models and transfer scripts."""
import ctypes,json,struct,random
def run(c,root,args):
 c.group='startup-core';public=root/'labs/machine_image/startup_depth';private=root/'assessor/legacy_systems/startup_depth';sources=[private/(n+'.c') for n in ('stack','decimal','transfer')]
 c.run(['gcc','--version']);c.run(['clang','--version'])
 for cc in ('gcc','clang'):
  for profile,flags in [('debug',['-O0']),('release',['-O2']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
   exe=c.root/(cc+'-'+profile);c.run([cc,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(public),public/'tests.c',*sources,'-o',exe]);c.run([exe],contains='STARTUP-CORE-PASS')
 for name,inputs,flags in [('resend',sources,['-DSD_MUTANT_RESEND']),('learner',[public/'startup.c'],[])]:
  exe=c.root/name;c.run(['gcc','-std=c17','-O2','-Wall','-Wextra','-Werror',*flags,'-I'+str(public),public/'tests.c',*inputs,'-o',exe]);c.run([exe],expected=1,contains='STARTUP-CHECK:')
 for cc in ('gcc','clang'):c.run(['python3',public/'run_core.py','--implementation',*sources,'--cc',cc],contains='STARTUP-CORE-PASS')
 c.run(['python3',public/'run_core.py','--implementation',public/'startup.c','--cc','gcc'],expected=1,contains='STARTUP-CHECK:')
 # ABI-explicit host binding; independent word arrays describe expected boundaries.
 class Stack(ctypes.Structure):_fields_=[('argc',ctypes.c_uint64),('argv',ctypes.c_size_t),('envp',ctypes.c_size_t),('auxv',ctypes.c_size_t),('extent',ctypes.c_size_t),('width',ctypes.c_uint)]
 for cc in ('gcc','clang'):
  lib=c.root/(cc+'-core.so');c.run([cc,'-std=c17','-O2','-shared','-fPIC','-I'+str(public),*sources,'-o',lib]);dll=ctypes.CDLL(str(lib));f=dll.sd_stack_decode;f.argtypes=[ctypes.c_void_p,ctypes.c_size_t,ctypes.c_uint,ctypes.POINTER(Stack)];f.restype=ctypes.c_int
  count=0
  for width in (4,8):
   for argc in range(5):
    for env in range(4):
     for aux in range(5):
      words=[argc]+[8192+31*i for i in range(argc)]+[0]+[32768+79*i for i in range(env)]+[0]+[x for i in range(aux) for x in (6+i,4096+127*i)]+[0,0]
      data=b''.join(x.to_bytes(width,'little') for x in words);buffer=ctypes.create_string_buffer(b'!'+data);pointer=ctypes.addressof(buffer)+1;view=Stack()
      c.require(f(pointer,len(data),width,ctypes.byref(view))==0,'valid entry model',dict(width=width,argc=argc,env=env,aux=aux));c.require((view.argc,view.argv,view.envp,view.auxv,view.extent,view.width)==(argc,width,(argc+2)*width,(argc+env+3)*width,len(data),width),'independent entry-model offsets')
      for length in range(len(data)):
       ctypes.memset(ctypes.byref(view),0xa5,ctypes.sizeof(view));before=bytes(view);status=f(pointer,length,width,ctypes.byref(view));c.require(status!=0 and bytes(view)==before,'every truncated table rejects unchanged',length);count+=1
  c.observe('T10.02.a','Original96-byte table and all bounded argument/environment/auxiliary table truncations validated',['startup-core']) if cc=='gcc' else None
  c.require(count>10000,'entry boundary campaign is nonvacuous',count)
  c.write(cc+'-entry-model-count.json',json.dumps({'cases':200,'truncated_prefixes':count,'widths':[4,8],'limits':'Finite constructed tables; real process snapshot separately required'}))
 # Target object audit remains separate from hosted oracle dependencies.
 for name in ('stack','decimal','transfer'):
  obj=c.root/(name+'-arm.o');c.run(['arm-none-eabi-gcc','-std=c17','-mcpu=cortex-m4','-mthumb','-O2','-ffreestanding','-fno-builtin','-I'+str(public),'-c',private/(name+'.c'),'-o',obj]);undefined=c.run(['arm-none-eabi-nm','-u',obj]);c.require(not undefined.strip(),'Arm core object has no implicit runtime helper',name);c.run(['arm-none-eabi-objdump','-d',obj])
 for objective,ordinary,variant in [
  ('02.a',None,'Empty environment and reordered/gapped immutable string map preserve table/address semantics'),
  ('02.b','Explicit unaligned LE byte decoding derives96-byte64-bit table without host pointer casts','Same table decoded as32-bit labels changes widths/offsets while retaining terminators'),
  ('07.a','Script2,EINTR,3,2 submits offsets0,2,2,5 and accepts ABCDEFG exactly once','EAGAIN preserves offset2 and borrower lifetime before resumption'),
  ('07.b','Read EOF differs from stalled write; partial-zero and bounded EINTR scripts retain exact prefixes','Cancellation after accepted2 bytes stops future calls and cannot retract receiver data'),
  ('09.a','Unsigned parser rejects unsupported/overflow spans unchanged;65536+edge formatted values match hosted independent oracle','Nonterminated span format uses exact digit capacity without retaining a NUL contract'),
  ('09.c','Service bounds reject signs/space/non-ASCII/separators and overflow explicitly','Signed INT32_MIN parse/format uses checked unsigned magnitude without signed negation overflow')]:
  if ordinary:c.observe('T10.'+objective,ordinary,['startup-core'])
  c.observe('T10.'+objective,variant,['startup-core'],True)
