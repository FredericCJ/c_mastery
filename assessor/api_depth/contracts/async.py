import itertools,json,subprocess
def model(trace):
 state='running';committed=0;calls=0;outcome=99;results=[]
 for event in trace:
  if state=='done': result=1
  elif event in 'CT':
   state='cancel';result=6 if committed else 4
  elif event=='K':
   result=1 if committed else 0;committed=1
  elif event=='A' and state!='cancel': result=1
  else:
   result=0;outcome=0 if event=='S' else (6 if committed else 5)
   state='done';calls=1
  results.append(result)
 return ''.join(str(x)+',' for x in results)+'|'+','.join(map(str,[{'running':1,'cancel':2,'done':3}[state],committed,calls,calls,outcome,1,calls]))
def run(c,root,args):
 c.group='contracts-async';pub=root/'labs/api_depth/contracts';private=root/'assessor/api_depth/contracts';source=private/'async.c';v1=private/'evolution_v1.c';foundation=root/'assessor/foundations/foundation.c';registry=root/'assessor/api_depth/api_depth.c';common=['-std=c17','-Wall','-Wextra','-Werror','-I'+str(pub),'-I'+str(root/'labs/api_depth/include'),'-I'+str(root/'labs/foundations/include')]
 traces=[''.join(t) for n in range(6) for t in itertools.product('CTKSA',repeat=n)];expected=[model(t) for t in traces];data=c.write('async-traces.txt','\n'.join(t or '_' for t in traces)+'\n');expect=c.write('async-expected.txt','\n'.join(expected)+'\n')
 # A tiny host-side runner retains child argv/exit and compares each line against
 # the independently derived transition specification, not the C implementation.
 runner=c.write('compare_async.py','''import pathlib,subprocess,sys,json
exe,inputs,oracle,receipt=sys.argv[1:]
p=subprocess.run([exe],input=pathlib.Path(inputs).read_text(),capture_output=True,text=True)
want=pathlib.Path(oracle).read_text().splitlines();got=p.stdout.splitlines()
bad=[i for i,(a,b) in enumerate(zip(want,got)) if a!=b]
result=dict(argv=[exe],exit=p.returncode,stderr=p.stderr,cases=len(want),observed=len(got),mismatch_indices=bad,stdout=p.stdout)
pathlib.Path(receipt).write_text(json.dumps(result,indent=2)+'\\n')
print('ASYNC-ORACLE',len(want),len(got),len(bad))
raise SystemExit(bool(p.returncode or len(got)!=len(want) or bad))
''')
 for cc in ('gcc','clang'):
  for name,flags in [('debug',['-O0']),('release',['-O2']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
   exe=c.root/(cc+'-'+name+'-async');c.run([cc,*common,*flags,pub/'async_tests.c',source,v1,foundation,registry,'-o',exe]);c.run([exe],contains='ASYNC-PASS:scope_cases=3;completion_modes=2;effects=transitive;ownership=balanced')
   oracle=c.root/(cc+'-'+name+'-async-oracle');c.run([cc,*common,*flags,private/'async_oracle.c',source,foundation,'-o',oracle]);c.run(['python3',runner,oracle,data,expect,c.root/(cc+'-'+name+'-oracle.json')],contains='ASYNC-ORACLE 3906 3906 0')
 for label,flag,selected,selected_v1,reg in [('request-is-ack','AC_MUTANT_EARLY_CANCEL',source,v1,registry),('committed-rollback','AC_MUTANT_ROLLBACK',source,v1,registry),('failure-output','AC_MUTANT_FAILURE_OUTPUT',source,v1,registry),('learner','UNSOLVED',pub/'async.c',v1,registry),('v1-learner','UNSOLVED',source,pub/'evolution_v1.c',registry),('descendant-drop','API_MUTANT=4',source,v1,registry),('parent-early-drop','API_MUTANT=3',source,v1,registry)]:
  exe=c.root/('async-'+label)
  if label=='committed-rollback':
   c.run(['gcc',*common,'-O2','-D'+flag,private/'async_oracle.c',selected,foundation,'-o',exe]);c.run(['python3',runner,exe,data,expect,c.root/('mutant-'+label+'.json')],expected=1,contains='ASYNC-ORACLE 3906 3906')
  else:
   c.run(['gcc',*common,'-O2','-D'+flag,pub/'async_tests.c',selected,selected_v1,foundation,reg,'-o',exe]);c.run([exe],expected=1,contains='ASYNC-CHECK:')
 # Old client gets exactly one installed header and an archive, no private path.
 for cc in ('gcc','clang'):
  prefix=c.root/(cc+'-evolution-install');prefix.mkdir();(prefix/'evolution_v1.h').write_bytes((pub/'evolution_v1.h').read_bytes());consumer=c.write(cc+'-consumer/client.c',(pub/'evolution_consumer.c').read_text());obj=c.root/(cc+'-old-consumer.o');c.run([cc,'-std=c17','-Wall','-Wextra','-Werror','-I'+str(prefix),'-c',consumer,'-o',obj])
  for version in (1,2):
   old=c.root/(cc+f'-v{version}-sync.o');c.run([cc,*common,'-O2','-c',v1,'-o',old]);archive=prefix/f'libv{version}.a';members=[old]
   if version==2:
    for name,path in [('async',source),('foundation',foundation)]:
     member=c.root/(cc+'-'+name+'.o');c.run([cc,*common,'-O2','-c',path,'-o',member]);members.append(member)
   c.run(['ar','rcs',archive,*members]);exe=c.root/(cc+f'-consumer-v{version}');c.run([cc,obj,archive,'-o',exe]);c.run([exe]);c.run(['nm','-u',old])
  relocated=c.root/(cc+'-evolution-relocated');prefix.rename(relocated);exe=c.root/(cc+'-consumer-relocated');c.run([cc,obj,relocated/'libv2.a','-o',exe]);c.run([exe])
 # Strict transform closure includes its actual recorder, actual final image,
 # object relocations, disassembly and compiler stack-use reports.
 arm=['arm-none-eabi-gcc','-std=c17','-mcpu=cortex-m4','-mthumb','-O2','-ffreestanding','-fno-builtin','-fstack-usage','-I'+str(pub)]
 objects=[]
 for path in (v1,private/'effects_target.c'):
  obj=c.root/(path.stem+'-m4.o');c.run([*arm,'-c',path,'-o',obj]);objects.append(obj);c.run(['arm-none-eabi-objdump','-dr',obj]);c.run(['arm-none-eabi-nm','-u',obj])
 image=c.root/'effects-m4.elf';c.run(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-Wl,-e,_start','-Wl,-Map='+str(c.root/'effects-m4.map'),*objects,'-o',image]);undefined=c.run(['arm-none-eabi-nm','-u',image]);c.require(not undefined.strip(),'whole reachable transform+recorder image has empty runtime closure',undefined);c.run(['arm-none-eabi-objdump','-d',image]);stack={p.name:p.read_text() for p in c.root.glob('*m4.su')};c.require(len(stack)==2,'actual stack usage files',stack)
 for key,ordinary,changed in [('04.a','Actual ApiRegistry deferred removal retains callback context until callback returns; owned payload release counter follows acknowledgment','Queued descendant retains its own payload claim; next dispatch, explicit scope-close and close-inside-parent each release exactly twice without use after free'),('08.a','Actual allocating logging callback disproves transitive no-allocation despite allocation-free transform; bounded recorder restores closed call-graph claim','Actual Cortex-M4 relocations, final empty undefined-symbol closure, disassembly and stack records; no ISR latency or physical execution inferred'),('08.b','Synchronous stack-context consumer versus deferred stable control and owned input snapshot executed','Inline completion after all state/ticket initialization and terminal publication; recipient immediately drops transferred box; reentrant operation rejected during reporting'),('08.c','3906 independently modeled traces include deadline request/cancel request/ack/completion/commit; exactly one report/release with duplicate rejection','Irreversible synthetic effect causes cancellation acknowledgment to report indeterminate, preserving no rollback claim'),('10.a','Separate v1 synchronous borrow and v2 deferred ownership symbols retain semantic compatibility; old archive client executed','Failure-output initialization mutant compiles and is rejected by the old caller preservation contract'),('10.b','GCC/Clang old consumer compiled in external directory with only installed v1 header and linked both archive versions','Install prefix physically relocated and same consumer object linked/executed without source/private paths'),('10.c','Executable migration covers allocator failure, copied input, cancel acknowledgment, ownership transfer and exact release','Owned snapshot remains unchanged across source mutation; receiver may retain across further operations or release immediately; borrowing beyond callback explicitly invalid')]:
  c.observe('T18.'+key,ordinary,['contracts-async']);c.observe('T18.'+key,changed,['contracts-async'],True)
