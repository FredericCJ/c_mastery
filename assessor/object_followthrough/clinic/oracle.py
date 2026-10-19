"""Independent abstract owner model; never executes an invalid pointer operation."""
import json,pathlib,subprocess,sys
binary,out=sys.argv[1:];p=subprocess.run([binary],capture_output=True,text=True,timeout=30)
pathlib.Path(out).write_text(json.dumps(dict(returncode=p.returncode,stdout=p.stdout,stderr=p.stderr),indent=2)+'\n',encoding='utf8')
assert p.returncode==0,p.stderr
actual=[list(map(int,s.split(','))) for s in p.stdout.splitlines()];expected=[]
for history in range(81):
    code=history;live=False;releases=0
    for step in range(4):
        event=code%3;code//=3;value=99
        if event==0:
            status=-1 if live else 0;live=True
        elif event==1:
            status=0 if live else -1
            if live:releases+=1
            live=False
        else:
            status=0 if live else -1
            if live:value=7
        expected.append([history,step,event,status,int(live),releases,value])
assert actual==expected,(actual,expected)
print(json.dumps(dict(status='pass',histories=81,observations=len(actual))))
