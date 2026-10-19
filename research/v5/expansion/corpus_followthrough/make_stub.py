"""Create only the intentionally incomplete public C implementation."""
import pathlib,re
ROOT=pathlib.Path(__file__).resolve().parents[4]
header=(ROOT/'labs/corpus_index/index.h').read_text()
lines=['#include "index.h"','/* Complete each mechanism from the public contract. */']
for m in re.finditer(r'CiStatus\s+(\w+)\(([^;]*)\);',header,re.S):
 args=m[2].split(',');params=[' '.join(p.split())+' arg'+str(i) for i,p in enumerate(args)]
 lines+=['CiStatus '+m[1]+'('+','.join(params)+'){'+''.join('(void)arg'+str(i)+';' for i in range(len(args)))+'return CI_INVALID;}']
target=ROOT/'labs/corpus_index/src/index.c';target.parent.mkdir(parents=True,exist_ok=True)
target.write_text('\n'.join(lines)+'\n')
