"""Execute one original custom-entry child and inspect real argument/load-bias observations."""
import json,os,pathlib,re,subprocess,sys
binary,tracer,library,out=sys.argv[1:];environment=os.environ.copy();environment['LD_LIBRARY_PATH']=library
symbols=subprocess.check_output(['nm','--defined-only',binary],text=True);table={m[2]:int(m[0],16) for s in symbols.splitlines() if len(m:=s.split())==3};kind=subprocess.check_output(['readelf','-hW',binary],text=True);pie='DYN (' in kind
observations=[]
cases=[([],dict(CE_TEST='A'),0),([''],dict(CE_TEST='B',EXTRA='v'),0),(['ABCDEFGHI'],dict(CE_TEST='A'),0),(['AB','changed'],dict(CE_TEST='B',EXTRA='v'),0),(['x'*32],dict(CE_TEST='C'),0),(['x'*33],dict(CE_TEST='A'),71),(['a','b','c'],dict(CE_TEST='A'),70),(['a'],dict(OTHER='v'),72)]
for index,(arguments,env,expected) in enumerate(cases):
    trace=pathlib.Path(out).with_suffix('.'+str(index)+'.trace');cmd=[tracer,'-qq','-e','trace=write,exit,exit_group','-o',str(trace),'--',binary,*arguments]
    # Tracer needs its private library directory; the target is static and its
    # initial environment is deliberately explicit and semantically checked.
    runenv=dict(env,LD_LIBRARY_PATH=library);p=subprocess.run(cmd,env=runenv,capture_output=True,text=True,timeout=30);text=trace.read_text();assert p.returncode==expected,(cmd,p.returncode,p.stderr)
    row=dict(arguments=arguments,environment=runenv,exit_code=p.returncode,stdout=p.stdout,stderr=p.stderr,trace=str(trace))
    if expected==0:
        assert p.stdout==''.join(arguments),(arguments,p.stdout)
        m=re.fullmatch(r'CE:([0-9a-f]{16}):([0-9a-f]{16}):([0-9a-f]{16}):([0-9a-f]{16}):([0-9a-f]{16})\n',p.stderr);assert m,p.stderr
        argc,envc,test,data,entry=[int(x,16) for x in m.groups()];bias=data-table['cpw_data'];assert argc==len(arguments)+1 and envc==len(runenv) and test==ord(env['CE_TEST']);assert entry-table['_start']==bias and bias>=0 and bias%4096==0 and (bias!=0 if pie else bias==0)
        assert 'EBADF' in text and 'exit(0)' in text;row.update(argc=argc,envc=envc,test=test,load_bias=bias,runtime_data=data,runtime_entry=entry)
    else:assert not p.stdout and not p.stderr and 'exit('+str(expected)+')' in text,(arguments,p.stdout,p.stderr,text)
    observations.append(row)
pathlib.Path(out).write_text(json.dumps(dict(status='pass',pie=pie,symbols=table,observations=observations,limits=['Real changed argc/argv/env inputs; no target ABI change is claimed.','Fixed/PIE observations apply to images with no dynamic relocation stage or interpreter.','Current Linux behavior is distinct from the inspected historical ABI draft and tagged kernel source.']),indent=2)+'\n')
print(json.dumps(dict(status='pass',cases=len(observations),pie=pie)))
