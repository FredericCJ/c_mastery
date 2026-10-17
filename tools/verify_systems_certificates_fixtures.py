"""Independent graph/substring/transition oracles for the audited contract gaps."""
import argparse,ctypes as C,itertools,json
from pathlib import Path
class Node(C.Structure):_fields_=[('key',C.c_int32),('left',C.c_int16),('right',C.c_int16),('parent',C.c_int16),('height',C.c_uint8),('live',C.c_uint8)]
class Tree(C.Structure):_fields_=[('nodes',Node*64),('root',C.c_int16),('count',C.c_uint8),('balanced',C.c_uint8)]
class Machine(C.Structure):_fields_=[('reg',C.c_uint8*4),('memory',C.c_uint8*16),('pc',C.c_uint8),('halted',C.c_uint8),('commits',C.c_uint32)]
class Instruction(C.Structure):_fields_=[('operation',C.c_uint8),('a',C.c_uint8),('b',C.c_uint8),('c',C.c_uint8)]
class Control(C.Structure):_fields_=[(n,C.c_uint8) for n in ['pc_before','pc_after','reg_write','mem_write','branch_taken','alu']]
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--library',type=Path);args=parser.parse_args()
 if not args.library:print(json.dumps({'executed':False,'scope':'Pass --library for independent differential C execution.'}));return
 lib=C.CDLL(str(args.library.resolve()));ref=C.byref;counts={}
 n=0
 for edges in itertools.product(range(-1,3),repeat=6):
  for root in range(3):
   tree=Tree();tree.root=root;tree.count=3;parents=[-1]*3;seen=set();heights={};valid=True
   for i in range(3):
    tree.nodes[i]=Node(i+1,edges[2*i],edges[2*i+1],-1,1,1)
    for child in edges[2*i:2*i+2]:
     if child>=0:parents[child]=i
   for i in range(3):tree.nodes[i].parent=parents[i]
   def visit(i,lo,hi,parent):
    nonlocal valid
    if i==-1:return 0
    if i in seen or not(lo<i+1<hi) or parents[i]!=parent:valid=False;return 0
    seen.add(i);left=visit(edges[2*i],lo,i+1,i);right=visit(edges[2*i+1],i+1,hi,i);heights[i]=1+max(left,right);return heights[i]
   height=visit(root,0,4,-1);valid=valid and len(seen)==3
   for i,h in heights.items():tree.nodes[i].height=h
   out=C.c_uint(91);status=lib.sc_tree_check(ref(tree),ref(out));assert status==(0 if valid else 1)
   assert out.value==(height if valid else 91);n+=1
 counts['tree']=n;n=0
 for text_n in range(9):
  for text in itertools.product(range(2),repeat=text_n):
   t=(C.c_uint8*text_n)(*text)
   for pattern_n in range(5):
    for pattern in itertools.product(range(2),repeat=pattern_n):
     p=(C.c_uint8*pattern_n)(*pattern);expected=[i for i in range(text_n-pattern_n+1) if text[i:i+pattern_n]==pattern]
     out=(C.c_uint8*65)(*([99]*65));count=C.c_size_t(91);assert lib.sc_matches(t,text_n,p,pattern_n,out,65,ref(count))==0
     assert count.value==len(expected) and list(out)[:count.value]==expected
     if expected:
      out=(C.c_uint8*65)(*([99]*65));count=C.c_size_t(91);assert lib.sc_matches(t,text_n,p,pattern_n,out,len(expected)-1,ref(count))==2 and count.value==91 and list(out)==[99]*65
     n+=1
 counts['matches']=n;n=0
 for a,b,c,x,y in itertools.product(range(4),range(4),range(4),[0,1,127,128,255],[0,1,127,128,255]):
  m=Machine();m.reg[:]=[x,y,(x+y)%256,(x-y)%256];original=list(m.reg);program=(Instruction*2)(Instruction(1,a,b,c),Instruction(5,0,0,0));control=Control()
  expected=original[:];expected[a]=(original[b]-original[c])%256
  assert lib.sc_clock(ref(m),program,2,ref(control))==0 and list(m.reg)==expected and m.pc==1 and m.commits==1 and control.alu==expected[a];n+=1
 for operation,a,b,c in itertools.product(range(8),[0,3,4],[0,1,15,16],[0,1,4]):
  m=Machine();m.reg[:]=[1,2,3,4];m.memory[:]=range(16);before=bytes(m);control=Control(99,99,99,99,99,99);old=bytes(control)
  p=(Instruction*2)(Instruction(operation,a,b,c),Instruction(5,0,0,0))
  valid=operation==0 and a<4 and c==0 or operation==1 and a<4 and b<4 and c<4 or operation==2 and a<4 and b<2 and c==0 or operation in (3,4) and a<4 and b<16 and c==0 or operation==5 and a==b==c==0
  status=lib.sc_clock(ref(m),p,2,ref(control));assert status==(0 if valid else 1)
  if not valid:assert bytes(m)==before and bytes(control)==old
  n+=1
 counts['clock']=n
 print(json.dumps({'executed':True,'cases':sum(counts.values()),'by_mechanism':counts,'limits':'Finite semantic models, not native ISA/physical observations or independent learner assessment.'}))
if __name__=='__main__':main()
