"""Independent two-external-owner set model for actual fallible-close adapter."""
import argparse,hashlib,json,subprocess
from pathlib import Path
from verify_modern_composition_recovery import expected as original_expected
def expected(code):
    plan=[];x=code
    for _ in range(4):plan.append(x%3);x//=3
    live={0,1};used=closes=0
    def attempt():
        nonlocal used,closes
        errors=0
        for resource in sorted(live,reverse=True):
            result=plan[used] if used<4 else 0;used+=1;closes+=1
            if result:errors+=1
            if result!=1:live.remove(resource)
        return errors
    e=attempt();first=[sum(1<<i for i in live),e,int(bool(live)),6 if live else 3,len(live)+2]
    if live:second_errors=attempt();done=int(not live)
    else:second_errors=99;done=0
    return ','.join(map(str,[code,*first,done,second_errors,sum(1<<i for i in live),int(bool(live)),6 if live else 3,len(live)+2,closes]))
def run(driver):
    answers=[expected(i) for i in range(81)];p=subprocess.run([driver,'oracle'],text=True,capture_output=True,timeout=30);assert p.returncode==0,p.stderr;got=p.stdout.splitlines();assert got==answers,next(((i,a,b) for i,(a,b) in enumerate(zip(got,answers)) if a!=b),len(got))
    old=subprocess.run([str(Path(driver).with_name('modern-publication-original')),'oracle'],text=True,capture_output=True,timeout=30);assert old.returncode==0,old.stderr;original=[json.loads(line) for line in old.stdout.splitlines()];assert original==original_expected()
    return dict(passed=True,outcome_words=81,outcomes=3,max_close_results=4,original_prefix_rows=len(original),trace_sha256=hashlib.sha256(('\n'.join(answers)+'\n').encode()).hexdigest(),limits='Two cleanup attempts over two pending identities; admitted release/retained/released-error results. Original actual twelve-row allocation/open/retained-close ledger separately executed. Real Linux close succeeds; errors are injected after release or before an attempted release.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
