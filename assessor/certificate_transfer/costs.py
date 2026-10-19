"""Summarize emitted code/frame evidence without claiming physical stack readings."""
import json,pathlib,sys
def summarize(directory):
 rows=[]
 for cc in ['gcc','clang']:
  for mode in ['O0','O3']:
   parts={}
   for kind,names in [('iterative',{'ct_tree','push'}),('recursive',{'at_check','visit'})]:
    base=directory/(cc+'.'+mode+'.cost.'+kind);frames={};sizes={}
    for line in base.with_suffix(base.suffix+'.su').read_text().splitlines():
     label,size,qual=line.split('\t');name=label.rsplit(':',1)[1]
     if name in names:frames[name]=dict(bytes=int(size),qualifier=qual)
    for line in pathlib.Path(str(base)+'.size.log').read_text().splitlines():
     fields=line.split()
     if len(fields)==4 and fields[3] in names:sizes[fields[3]]=int(fields[1],16)
    assert ('ct_tree' if kind=='iterative' else 'at_check') in frames
    parts[kind]=dict(function_frames=frames,function_code_bytes=sizes,selected_function_code_bytes=sum(sizes.values()))
   it=parts['iterative']['function_frames'];rec=parts['recursive']['function_frames']
   # .su includes each function's own emitted frame; these bounds deliberately
   # overcount fixed outgoing areas rather than pretending to measure high water.
   parts['iterative']['conservative_call_frame_sum']=sum(v['bytes'] for v in it.values())
   parts['recursive']['conservative_64_node_plus_nil_call_frame_sum']=rec['at_check']['bytes']+65*rec['visit']['bytes']
   rows.append(dict(profile=cc+'.'+mode,**parts))
 return dict(passed=True,profiles=rows,limits=['Exact uninstrumented x86-64 C17 profiles; code bytes exclude shared rodata/alignment/link layout.','Conservative compiler-reported function-frame sums, not physical or dynamic stack measurements; caller workspace and test harness excluded.','Recursive count uses64 nodes plus a terminal NIL call; inlining/compiler transformations remain visible in disassembly; no portable byte-count assertion.'])
if __name__=='__main__':
 d=summarize(pathlib.Path(sys.argv[1]));pathlib.Path(sys.argv[2]).write_text(json.dumps(d,indent=2)+'\n');print('CT_COST_PASS profiles=4')
