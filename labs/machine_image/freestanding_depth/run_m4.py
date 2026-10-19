"""Bounded local QEMU receiver; UART frame success is not guest termination."""
import argparse,json,subprocess
from pathlib import Path
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--elf',required=True,type=Path);p.add_argument('--transport',required=True,choices=('semi','uart'));p.add_argument('--expected',required=True);a=p.parse_args()
 command=['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','stdio','-kernel',str(a.elf)]
 if a.transport=='semi':command+=['-semihosting-config','enable=on,target=native']
 process=subprocess.Popen(command,stdin=subprocess.DEVNULL,stdout=subprocess.PIPE,stderr=subprocess.PIPE);stopped=False
 try:out,err=process.communicate(timeout=3)
 except subprocess.TimeoutExpired:
  stopped=True;process.terminate()
  try:out,err=process.communicate(timeout=2)
  except subprocess.TimeoutExpired:process.kill();out,err=process.communicate(timeout=2)
 expected=(a.expected+'\n').encode();payload=err if a.transport=='semi' else out
 passed=payload==expected and ((not stopped and process.returncode==0) if a.transport=='semi' else stopped)
 result=dict(passed=passed,argv=command,exit=process.returncode,host_stopped_guest=stopped,stdout=out.decode(errors='replace'),stderr=err.decode(errors='replace'),expected_hex=expected.hex(),scope='QEMU MPS2 AN386 local original image, no physical or STM32 observation')
 print(json.dumps(result));return not passed
if __name__=='__main__':raise SystemExit(main())
