"""Unbounded mathematical sizes and independent finite logical mutation histories."""
import json,pathlib,subprocess,sys
kind,binary,out=sys.argv[1:4];prefix=json.loads(sys.argv[4]) if len(sys.argv)>4 else []
p=subprocess.run([*prefix,binary],capture_output=True,text=True,timeout=90);pathlib.Path(out).write_text(json.dumps(dict(command=[*prefix,binary],returncode=p.returncode,stdout=p.stdout,stderr=p.stderr),indent=2)+'\n');assert p.returncode==0,p.stderr
actual=[list(map(int,line.split(','))) for line in p.stdout.splitlines()]
if kind=='size':
    assert len(actual)==320
    for prefix,width,count,limit,status,value in actual:
        wanted=prefix+width*count;accepted=width>0 and wanted<=limit
        assert status==(0 if accepted else -1) and value==(wanted if accepted else 99),(prefix,width,count,limit,status,value)
else:
    expected=[]
    for history in range(625):
        code=history;values=[];generation=1;retired=0;capacity=4
        for step in range(4):
            event=code%5;code//=5;status=-1
            if event==4:retired=1;values=[];capacity=0;status=0
            elif not retired:
                if event==0 and len(values)<capacity:values.append(step+1);status=0
                elif event==1 and generation<3:generation+=1;capacity=4;status=0
                elif event==2 and values and generation<3:values.pop(0);generation+=1;status=0
            expected.append([history,step,event,status,generation,retired,capacity,len(values),*(values+[0]*(4-len(values)))])
    assert actual==expected,next(((i,a,b) for i,(a,b) in enumerate(zip(actual,expected)) if a!=b),('counts',len(actual),len(expected)))
print(json.dumps(dict(status='pass',kind=kind,observations=len(actual),limits='Finite declared model only; pointer lifetime reasoning is separate')))
