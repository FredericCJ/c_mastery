"""Independent expected values derived from the selected token contract."""
import argparse,hashlib,json,subprocess
p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();r=subprocess.run([a.driver,'oracle'],text=True,capture_output=True);expected=['7 3 11 22','VALUE','7','alpha "q\\"x" + beta'];actual=r.stdout.splitlines();assert r.returncode==0 and actual==expected,(actual,expected)
print(json.dumps(dict(passed=True,fields=7,trace_sha256=hashlib.sha256(('\n'.join(expected)+'\n').encode()).hexdigest())))
