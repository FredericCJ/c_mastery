"""Host delta debugging against original correct and flawed teaching targets."""
import subprocess,sys,json
def status(binary,trace):return int(subprocess.check_output([binary,trace],text=True))
def main(good,bad):
 calls=0
 def mismatch(trace):
  nonlocal calls;calls+=2;return status(good,trace)==4 and status(bad,trace)==0
 original='afafafafc';assert mismatch(original);trace=original
 changed=True
 while changed:
  changed=False
  for index in range(len(trace)):
   candidate=trace[:index]+trace[index+1:]
   if mismatch(candidate):trace=candidate;changed=True;break
 assert trace=='c' and not mismatch('')
 print(json.dumps(dict(status='PASS',original=original,minimized=trace,target_invocations=calls,claim='one-deletion-minimal within this alphabet; not globally minimal arbitrary C input')))
if __name__=='__main__':main(*sys.argv[1:])
