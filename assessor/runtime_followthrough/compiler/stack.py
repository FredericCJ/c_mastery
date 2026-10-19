"""Audit actual Clang call expressions and actual GCC own-function frames."""
import json,pathlib,sys
def walk(node):
 yield node
 for child in node.get('inner',[]):yield from walk(child)
def main():
 tree=json.loads(pathlib.Path(sys.argv[1]).read_text());definitions={}
 for node in tree.get('inner',[]):
  if node.get('kind')=='FunctionDecl' and any(x.get('kind')=='CompoundStmt' for x in node.get('inner',[])):definitions[node['name']]=node
 calls={};external=set()
 for name,node in definitions.items():
  destinations=set()
  for expression in walk(node):
   if expression.get('kind')!='CallExpr':continue
   refs=[x['referencedDecl'] for x in walk(expression['inner'][0]) if x.get('kind')=='DeclRefExpr'];assert len(refs)==1 and refs[0]['kind']=='FunctionDecl','unresolved indirect call'
   destinations.add(refs[0]['name'])
  calls[name]=destinations&definitions.keys();external|=destinations-definitions.keys()
 def depth(name,active):
  if name in active:raise ValueError('VC_RECURSION:'+name)
  return 1+max([depth(n,active|{name}) for n in calls[name]] or [0])
 try:longest=max(depth(n,set()) for n in definitions)
 except ValueError as e:print(str(e));return 41
 frames={}
 for line in pathlib.Path(sys.argv[2]).read_text().splitlines():
  location,size,kind=line.split('\t');name=location.rsplit(':',1)[1];assert kind=='static',line;frames[name]=int(size)
 assert set(definitions)<=frames.keys()
 def cost(name):return frames[name]+max([cost(n) for n in calls[name]] or [0])
 print(json.dumps(dict(passed=True,own_functions=len(definitions),maximum_own_call_depth=longest,own_call_chain_bytes=max(cost(n) for n in definitions),frames=frames,calls={n:sorted(v) for n,v in calls.items()},excluded_external_calls=sorted(external),limits=['Actual source AST proves only this own-function call graph is acyclic; no universal compiler termination claim.','GCC static own frames exclude actual external verifier/libc frames and caller-owned workspace.'])))
 return 0
if __name__=='__main__':raise SystemExit(main())
