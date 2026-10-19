import itertools,re,hashlib
def expected(raw):
 if not raw:return 'E -1 0'
 start=1 if raw[0] in (43,45) else 0
 if start==len(raw):return 'E -1 '+str(start)
 digits=[];limit=32768 if raw[0]==45 else 32767
 for i in range(start,len(raw)):
  if not 48<=raw[i]<=57:return 'E -2 '+str(i)
  digits.append(chr(raw[i]))
  if int(''.join(digits))>limit:return 'E -3 '+str(i)
 value=int(''.join(digits));return 'I '+str(-value if raw[0]==45 else value)
def run(c,root,args):
 c.group='abstractions-generic';pub=root/'labs/api_depth/abstractions';private=root/'assessor/api_depth/abstractions';source=private/'generic.c';includes=['-I'+str(pub),'-I'+str(root/'labs/api_depth/contracts'),'-I'+str(root/'labs/api_depth/include'),'-I'+str(root/'labs/foundations/include')];common=['-std=c17','-Wall','-Wextra','-Werror',*includes]
 for cc in ('gcc','clang'):
  for name,flags in [('debug',['-O0']),('release',['-O2']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
   exe=c.root/(cc+'-'+name+'-generic');c.run([cc,*common,*flags,pub/'generic_tests.c',source,'-o',exe]);c.run([exe],contains='GENERIC-PASS:forms=3;capacity=4;staged_rollback=1;transfer_modes=2;tokens=10;fields=4;')
 for name,flag,selected in [('generic-leak','AS_MUTANT_STAGE_LEAK',source),('generic-range','AS_MUTANT_PARSE_RANGE',source),('generic-learner','UNSOLVED',pub/'generic.c')]:
  exe=c.root/name;c.run(['gcc',*common,'-O2','-D'+flag,pub/'generic_tests.c',selected,'-o',exe]);c.run([exe],expected=1,contains='GENERIC-CHECK:')
 corpus=[bytes(t) for n in range(5) for t in itertools.product(b'0+9-x',repeat=n)]
 corpus += [str(x).encode() for x in range(-32800,32801)]
 corpus += [b'\0',b'12\0',b'+32767',b'+32768',b'-0000000',b'99999999']
 data=c.write('generic-numbers.txt',''.join(str(len(x))+' '+ ' '.join(map(str,x))+'\n' for x in corpus));oracle=c.write('generic-expected.txt','\n'.join(expected(x) for x in corpus)+'\n')
 compare=c.write('generic-compare.py','''import subprocess,pathlib,sys,json
exe,inputs,wanted,receipt=sys.argv[1:]
r=subprocess.run([exe],input=pathlib.Path(inputs).read_text(),capture_output=True,text=True)
want=pathlib.Path(wanted).read_text().splitlines();actual=r.stdout.splitlines()
bad=[i for i,(a,b) in enumerate(zip(want,actual)) if a!=b]
d=dict(argv=[exe],exit=r.returncode,stderr=r.stderr,cases=len(want),observed=len(actual),mismatch_indices=bad,stdout=r.stdout)
pathlib.Path(receipt).write_text(json.dumps(d,indent=2)+'\\n')
print('GENERIC-ORACLE',len(want),len(actual),len(bad))
raise SystemExit(bool(r.returncode or len(want)!=len(actual) or bad))
''')
 for cc in ('gcc','clang'):
  exe=c.root/(cc+'-generic-oracle');c.run([cc,*common,'-O2',private/'generic_oracle.c',source,'-o',exe]);c.run(['python3',compare,exe,data,oracle,c.root/(cc+'-generic-oracle.json')],contains=f'GENERIC-ORACLE {len(corpus)} {len(corpus)} 0');c.require(True,'independent decimal grammar oracle '+cc,dict(cases=len(corpus),input_sha256=hashlib.sha256(data.read_bytes()).hexdigest(),model='Python unbounded decimal prefixes; signed target interval32768/32767'))
 probes={
 'wrong-type':'#include "generic.h"\nint main(void){AsInts b={0};return as_generic_push(&b,1.5);}\n',
 'unselected':'int main(void){return _Generic(1,int:0,default:((int)0).missing);}\n',
 'missing-case':'#include "generic.h"\nint bad(AvTag t){switch(t){case AV_INT:return 1;case AV_TEXT:return 2;case AV_ERROR:return 3;}return 0;}\n'}
 for name,body in probes.items():
  path=c.write(name+'.c',body)
  for cc in ('gcc','clang'):c.run([cc,*common,'-Wswitch-enum','-c',path,'-o',c.root/(cc+'-'+name+'.o')],expected='nonzero',contains={'wrong-type':'compatible','unselected':'member','missing-case':'AV_OWNED'}[name])
 types=c.write('generic-transformations.c','#include <stdio.h>\nint main(void){int array[3]={0};const int value=1;const int *pointer=&value;int *const qualified=array;printf("GENERIC-TYPES:%d,%d,%d,%d,%d\\n",_Generic(array,int*:1,default:0),_Generic(pointer,const int*:1,default:0),_Generic(qualified,int*:1,default:0),_Generic(value,int:1,default:0),_Generic(&array,int(*)[3]:1,default:0));return 0;}\n')
 for cc in ('gcc','clang'):
  for edition in ('c11','c17','c23'):
   exe=c.root/(cc+'-'+edition+'-generic-types');c.run([cc,'-std='+edition,'-Wall','-Wextra','-Werror',types,'-o',exe]);c.run([exe],contains='GENERIC-TYPES:1,1,1,1,1')
 # One list generates names and count; it cannot prove each branch's semantics.
 tags=c.write('generated-tags.h','#define AS_TAGS(X) X(INT,0) X(TEXT,1) X(ERROR,2) X(OWNED,3)\n')
 generated=c.write('generated-tags.c','#include "generated-tags.h"\n#define ENUM(n,v) G_##n=v,\nenum Tag { AS_TAGS(ENUM) };\n#define ROW(n,v) [v]=#n,\nstatic const char *names[]={AS_TAGS(ROW)};\nint main(void){return sizeof(names)/sizeof(names[0])!=4 || G_OWNED!=3;}\n')
 exe=c.root/'generic-generated-tags';c.run(['gcc','-std=c17','-Wall','-Wextra','-Werror',generated,'-o',exe]);c.run([exe])
 for cc in ('gcc','clang'):
  for profile,flags in [('O0',['-O0']),('O2',['-O2']),('O0-lto',['-O0','-flto']),('O2-lto',['-O2','-flto'])]:
   for form in range(4):
    exe=c.root/(cc+'-'+profile+'-form'+str(form));c.run([cc,*common,*flags,'-DAS_FORM='+str(form),private/'generic_cost.c',source,'-o',exe]);output=c.run([exe],contains=f'GENERIC-COST:form={form};total=10000;');sizes=c.run(['size','-A',exe]);assembly=c.run(['objdump','-d',exe]);c.require(True,'actual generic form '+cc+' '+profile+' '+str(form),dict(form=form,profile=profile,output=output,sections=sizes,indirect_call_instructions=len(re.findall(r'\bcall\s+\*',assembly)),limits='Whole linked executable includes hosted harness; annotation separates implementation dispatch from CRT. Heterogeneousform3 has a different type contract; no universal performance equivalence.'))
 for key,ordinary,changed in [('04.a','Threeequivalent boundedpush forms executeoneevaluation andfullpreservation;wrongtype andunselected invalidexpression bothdiagnosed','Actualarraydecay,top-level pointerqualification drop,pointeeconst andpointer-to-array selection observedC11/C17/C23 underbothcompilers withDR481 attribution'),('04.b','Tworealresource-owningelements copied via typedstaging;secondcopy allocationfailure releasesfirststage andpreservesdestination','Move-only transfer rejects beforeownershipchange thenmoves once;oldownerinert,newownerreleasesonce'),('04.c','32actual binaries covertyped/generated/erased/heterogeneous forms inGCC/ClangO0/O2with/withoutLTO;output/text/data/indirectcalls retained','Runtimeheterogeneousinteger/borrowedtext elementcontracts revisit bytes/indirection withoutclaiming equivalenttypes'),('05.a','Integer7,embeddedzero borrowedtext,emptytext,error-2 andinvalidtag tested beforepayloadaccess','Ownedtext clone/move/drop protocols exercisefailure,deepcopy independence andexactallocationbalance'),('05.b','Missing AV_OWNED switchcase diagnosed separately from runtime malformedtag rejection','One-list generatedtag/metadata table executes; semantic wrong-range branch remainsa compiled counterexample to generatedcorrectness'),('05.c','Independent unboundedPython grammar checks signedinterval/boundaries andfirsterroroffset for exactcorpus','Fourfield accumulatingvalidation preservesindependentinvalidbyte,overflow,sign-only positions andsuccessfulfield withoutshortcircuit')]:
  c.observe('T19.'+key,ordinary,['abstractions-generic']);c.observe('T19.'+key,changed,['abstractions-generic'],True)
