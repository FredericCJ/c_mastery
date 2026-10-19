"""Bounds only the newly executed child; never changes parent/host settings."""
import json,resource,subprocess,sys
def child():
 resource.setrlimit(resource.RLIMIT_STACK,(65536,65536))
 resource.setrlimit(resource.RLIMIT_CORE,(0,0))
 resource.setrlimit(resource.RLIMIT_CPU,(2,2))
def main():
 rows=[]
 for depth in (1,128):
  p=subprocess.run([sys.argv[1],str(depth)],preexec_fn=child,capture_output=True,text=True,timeout=4)
  entered='ENTRY:stack' in p.stdout
  rows.append({'depth':depth,'exit_code':p.returncode,'entered':entered,'stdout':p.stdout,'stderr':p.stderr,'disposition':'completed' if p.returncode==0 else 'signal_after_verified_setup' if entered and p.returncode<0 else 'inconclusive'})
 passed=rows[0]['exit_code']==0 and rows[1]['entered'] and rows[1]['exit_code']<0
 print(json.dumps({'passed':passed,'records':rows,'signal_is_not_a_portable_UB_oracle':True,'parent_limits_unchanged':True}))
 assert passed
if __name__=='__main__':main()
