"""Independent CVM1 small-program derivation, not a second VM interpreter."""
from pathlib import Path
import argparse,json,re
p=argparse.ArgumentParser();p.add_argument('log',type=Path);p.add_argument('--adapter',type=int,choices=[1,2],default=1);p.add_argument('--kernel',action='store_true');a=p.parse_args();text=a.log.read_text(errors='replace')
# INT41 costs one fuel; HOST costs one fuel and, if available, one host
# allowance before callback; RET costs one fuel. Failure preserves out=-99.
expected=[(0,0,41+a.adapter,1,0,29,0),(1,10,-99,0,0,30,0),(2,11,-99,1,0,30,0),(3,7,-99,0,0,0,1),(4,7,-99,1,0,0,0),(5,0,41+a.adapter,1,256,29,0)]
rows=[tuple(map(int,m.groups())) for m in re.finditer(r'^KF_VM:(\d+):(\d+):(-?\d+):(\d+):(\d+):(\d+):(\d+)\s*$',text,re.M)]
assert rows==expected,(rows,expected)
tail=re.findall(r'^KF_VM_TAIL:(\d+):(\d+):(\d+):(\d+):(\d+):(\d+)\s*$',text,re.M);assert len(tail)==1
t=list(map(int,tail[0]));assert t[:3]==[1,6,7] and t[3] in (3204,3216) and t[4:]==[1076,0],t
if a.kernel:assert 'KF:PASS:VM_SHARED_CORE' in text
else:
 wire=re.findall(r'^VX_BYTES:([0-9:]+)\s*$',text,re.M);assert len(wire)==1
 assert bytes(map(int,wire[0].split(':')))==bytes.fromhex('43564d31010000080000000129000000160111')
 assert 'VX_STARTUP_PASS data=305419896 bss=0' in text and 'VX_PROBE_PASS same_core_six_rows' in text
print(json.dumps(dict(passed=True,rows=rows,machine_bytes=t[3],adapter=a.adapter,scope='Independent three-instruction cost/status derivation and exact 19-byte CVM1 encoding; kernel image identity separately checked.')))
