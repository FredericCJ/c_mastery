"""Independent snapshot-only reachability, strict-order, parent and height oracle."""
import json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
def check(root,nodes,balanced):
 seen=set()
 def visit(p,parent,low,high):
  if p is None or p==-1:return 0,[]
  assert p in nodes and p not in seen;seen.add(p);n=nodes[p];assert n['parent']==parent and low<n['key']<high
  lh,left=visit(n['left'],p,low,n['key']);rh,right=visit(n['right'],p,n['key'],high);h=1+max(lh,rh);assert n['height']==h
  if balanced:assert abs(lh-rh)<=1
  return h,left+[n['key']]+right
 h,keys=visit(root,None if isinstance(root,str) else -1,-2**63,2**63);assert seen==set(nodes);return h,keys
def main():
 d=json.loads((ROOT/'fixtures/algorithms/PR-T36-05.json').read_text())
 for kind in ['rotation','deletion']:
  outputs=[check(x['root'],x['nodes'],x['balanced']) for x in d['input'][kind]];assert [x[0] for x in outputs]==d['expected'][kind+'_heights'] and [x[1] for x in outputs]==d['expected'][kind+'_inorders']
 actual=0;after=0
 if len(sys.argv)>1:
  lines=pathlib.Path(sys.argv[1]).read_text().splitlines();nodes={};header=None
  for line in lines:
   fields=line.split()
   if not fields:continue
   if fields[0]=='TREE':header=fields;nodes={}
   elif fields[0]=='NODE':
    index,key,left,right,parent,height=map(int,fields[1:]);nodes[index]=dict(key=key,left=left,right=right,parent=parent,height=height)
   elif fields[0]=='ENDTREE':
    h,keys=check(int(header[2]),nodes,True);assert len(nodes)==int(header[3]);actual+=1
    if header[1]=='after-delete':assert h==4 and nodes[int(header[2])]['key']==30 and keys==[2,4,8,20,23,25,27,30,33,35,37,40,45] and header[4:]==['2','2'];after+=1
  assert actual==(14 if sys.argv[2]=='rotations' else 1)
  if sys.argv[2]=='rotations':assert after==1
 print(json.dumps(dict(status='pass',fixture_certificates=6,actual_certificates=actual,two_ancestor_deletions=after)))
if __name__=='__main__':main()
