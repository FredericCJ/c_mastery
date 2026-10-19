"""Fail-closed routing controls for the exact mandatory legacy successor."""
import argparse,json,hashlib
from pathlib import Path
from unittest.mock import patch
import check_expansion_campaigns as route
import check_performance_delegated_package as package
ROOT=Path(__file__).resolve().parents[1]
def main():
    p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args();checks=[]
    actual=route.required_supplements();assert len(actual)==1 and actual[0]['passed'];checks.append('actual-narrow-package-invoked')
    rows=[{'passed':True}];assert route.current_campaigns_pass(rows,actual,'same','same');checks.append('complete-current-accepted')
    for name,r,s,b,e in [('absent',rows,[],'same','same'),('wrong-id',rows,[{'id':'unrelated','passed':True}],'same','same'),('failed',rows,[dict(actual[0],passed=False)],'same','same'),('family-failure',[{'passed':False}],actual,'same','same'),('source-changed',rows,actual,'before','after'),('duplicate',rows,actual+actual,'same','same')]:
        assert not route.current_campaigns_pass(r,s,b,e);checks.append(name)
    for error in (AssertionError('missing actual observation'),OSError('missing provider'),ValueError('stale source')):
        with patch.object(package,'validate',side_effect=error) as invoked:
            refused=route.required_supplements();assert invoked.call_count==1 and not refused[0]['passed'];assert not route.current_campaigns_pass(rows,refused,'same','same');checks.append(type(error).__name__+'-not-hidden')
    out=a.output.resolve();assert out.is_relative_to((ROOT/'reviews/v5/expansion/integrated/final_author_run/t43_cgroup').resolve()) and not out.exists()
    value={'passed':True,'checks':checks,'source_sha256':{str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in (Path(__file__),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/check_performance_delegated_package.py')},'scope':'Required successor routing and aggregate failure propagation; no new mechanism execution'};out.write_text(json.dumps(value,indent=2)+'\n');print(json.dumps({'passed':True,'checks':len(checks),'output':str(out)}))
if __name__=='__main__':main()
