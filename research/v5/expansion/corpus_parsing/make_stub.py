import pathlib,re
ROOT=pathlib.Path(__file__).resolve().parents[4]
header=(ROOT/'labs/parsing_depth/parsing.h').read_text(encoding='utf8')
lines=['#include "parsing.h"','/* Implement the supplied contracts; no assessor implementation is shipped. */']
for match in re.finditer(r'int\s+(\w+)\(([^;]*)\);',header,re.S):
 args=' '.join(match[2].split());names=[re.search(r'(\w+)\s*$',p).group(1) for p in args.split(',')]
 lines.append('int '+match[1]+'('+args+'){'+''.join('(void)'+n+';' for n in names)+'return PD_INVALID;}')
p=ROOT/'labs/parsing_depth/src/parsing.c';p.parent.mkdir(parents=True,exist_ok=True);p.write_text('\n'.join(lines)+'\n',encoding='utf8')
