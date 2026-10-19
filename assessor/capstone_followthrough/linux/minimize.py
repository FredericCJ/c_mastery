import json,pathlib,subprocess,sys
def run(exe,trace):
    p=subprocess.run([exe,trace],text=True,capture_output=True,timeout=5)
    return p.returncode,p.stdout
def main():
    good,bad,kind,dest=sys.argv[1:];code,marker,trace=(41,'CAP_SECURITY:authority','AUBLC') if kind=='authority' else (42,'CAP_SECURITY:retained','AABQLQXQC')
    original=trace;observations=[]
    def fails(t):
        rc,out=run(bad,t);observations.append(dict(trace=t,exit_code=rc,output=out));return rc==code and marker in out
    assert fails(trace) and run(good,trace)[0]==0
    changed=True
    while changed:
        changed=False
        for i in range(len(trace)):
            candidate=trace[:i]+trace[i+1:]
            if fails(candidate):trace=candidate;changed=True;break
    assert fails(trace) and run(good,trace)[0]==0
    assert not any(fails(trace[:i]+trace[i+1:]) for i in range(len(trace)))
    assert trace==('U' if kind=='authority' else 'ABLX')
    pathlib.Path(dest).write_text(json.dumps(dict(passed=True,kind=kind,original=original,minimal=trace,minimality='single-command deletion for this property, not global proof',observations=observations),indent=2)+'\n')
    print('CAP_MINIMIZED '+kind+'='+trace+' corrected-rejects-violation')
if __name__=='__main__':main()
