"""Independent arbitrary-precision translation artifact; no C implementation import."""
import argparse
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('output');p.add_argument('--mutant',action='store_true');a=p.parse_args()
values=[sum(range(n+1)) for n in range(32)]
if a.mutant:values[17]+=1
lines=['#ifndef LD_TABLE_H','#define LD_TABLE_H']
for n,v in enumerate(values):lines.append(f'#define LD_T{n} {v}u')
lines.append('static const unsigned int ld_table[32]={'+','.join(f'LD_T{n}' for n in range(32))+'};')
lines.append('#endif');Path(a.output).write_text('\n'.join(lines)+'\n',encoding='utf8')
