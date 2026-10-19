"""Check an original Linux learner image's declared output, exit and metadata."""
import argparse,hashlib,json,struct,subprocess
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--image',required=True,type=Path);p.add_argument('--decimal',action='store_true');a=p.parse_args();path=a.image.resolve();data=path.read_bytes()
if data[:6]!=b'\x7fELF\x02\x01':raise SystemExit('Expected ELF64LE')
h=struct.unpack_from('<HHIQQQIHHHHHH',data,16)
if h[1]!=62 or h[8]!=56 or h[9]>128 or h[4]>len(data) or h[9]*56>len(data)-h[4]:raise SystemExit('Invalid or unsupported program-header model')
ph=[struct.unpack_from('<IIQQQQQQ',data,h[4]+i*56) for i in range(h[9])]
if any(x[0] in (2,3) for x in ph):raise SystemExit('Raw fixed-address image must have no PT_DYNAMIC/PT_INTERP')
result=subprocess.run([path,'synthetic','arguments'],capture_output=True,timeout=10)
expected=bytes(65+i%26 for i in range(4096))+(b'4294967295' if a.decimal else b'')
passed=result.returncode==42 and result.stdout==expected and not result.stderr
print(json.dumps(dict(passed=passed,exit=result.returncode,stdout_bytes=len(result.stdout),stderr_bytes=len(result.stderr),sha256=hashlib.sha256(data).hexdigest(),limits='Output/metadata gate only; object provenance, ABI invariants and failure scripts require separate evidence.')))
raise SystemExit(not passed)
