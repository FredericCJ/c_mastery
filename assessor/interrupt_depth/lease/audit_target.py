"""Actual object barriers and closed image; report every virtual timing sample."""
from pathlib import Path
import argparse,json,re,subprocess,statistics
p=argparse.ArgumentParser();p.add_argument('build',type=Path);p.add_argument('log',type=Path);a=p.parse_args()
elf=a.build/'image.elf';assert not subprocess.check_output(['arm-none-eabi-nm','-u',elf],text=True).strip()
assembly=subprocess.check_output(['arm-none-eabi-objdump','-drwC',a.build/'2.o'],text=True)
for name,want in [('xd_compiler',[]),('xd_dmb',['dmb']),('xd_dsb',['dsb']),('xd_double_dsb',['dsb','dsb'])]:
    m=re.search(r'<'+name+r'>:\n(.*?)(?=\n\S|\Z)',assembly,re.S);assert m,name
    got=re.findall(r'\b(dmb|dsb|isb)\s+sy',m[1]);assert got==want,(name,got,want)
rows=[]
for line in a.log.read_text().splitlines():
    if line.startswith('XD_TIME '):rows.append([int(x,16) for x in line.split()[1:]])
assert len(rows)==31 and [r[0] for r in rows]==list(range(31))
assert all(0<r[1]<1000000 and 0<r[2]<1000000 for r in rows)
print(json.dumps(dict(passed=True,rows=rows,median_single=statistics.median(r[1] for r in rows),median_double=statistics.median(r[2] for r in rows),median_paired_difference=statistics.median(r[2]-r[1] for r in rows),
 unit='25MHz QEMU virtual timer tick; icount shift0, 128 volatile stores/barriers per sample',
 analysis='Adjacent identical DSB SY with no intervening access may collapse to one for these completion obligations; no cache maintenance or device-stop requirement is removed.',
 limits=['Observed emulator instruction-cost effect, not physical bus latency or WCET.','Compiler memory clobber is source-constrained; disassembly alone does not prove compiler ordering for arbitrary clients.']),indent=2))
