"""Independent finite oracle: enumerate simple paths, never a marking worklist."""
import itertools,json,pathlib,sys
rows=[]
for edges in itertools.product(range(4),repeat=6):
 for roots in range(8):
  live=0
  for node in range(3):
   found=False
   for length in range(1,4):
    for path in itertools.permutations(range(3),length):
     if path[-1]!=node or not roots & (1<<path[0]): continue
     if all(path[i+1]+1 in edges[2*path[i]:2*path[i]+2] for i in range(length-1)):found=True
   if found:live|=1<<node
  rows.append(' '.join(map(str,(*edges,roots,live))))
pathlib.Path(sys.argv[1]).write_text('\n'.join(rows)+'\n')
print(json.dumps(dict(cases=len(rows),method='All simple vertex paths of length1..3 in each directed three-node two-field graph; roots exhaust all subsets.')))
