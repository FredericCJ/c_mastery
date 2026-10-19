"""Independent accepted-prefix and logical traffic expectations."""
import argparse,hashlib,json,subprocess
def check(driver):
 def run(name):
  p=subprocess.run([driver,name],text=True,capture_output=True);assert p.returncode==0,(name,p.stdout,p.stderr);return p.stdout
 expected=[f'{writing} {chunk} {4+(4096+chunk-1)//chunk} 0' for writing in [0,1] for chunk in [1,127,4095,4096]]
 raw=run('oracle');assert raw.splitlines()==expected
 count=run('original_transfer');assert 'histories=40954' in count and 40954==2*(3*4095+2*4096)
 traffic=json.loads(run('amplification'));assert traffic==dict(commit=[0,34,3],accepted_commit_bytes=139264,checkpoint=[65,34,3],accepted_checkpoint_read=266240,accepted_checkpoint_write=139264,useful_changed_bytes=1)
 return dict(passed=True,short_transfer_histories=40954,nonblocking_rows=8,traffic=traffic,trace_sha256=hashlib.sha256(raw.encode()).hexdigest(),limits='Counts describe this fixed hot-root single-byte update and wrapper-observed accepted bytes; neither filesystem/device traffic nor physical power-loss durability is inferred.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(check(a.driver)))
