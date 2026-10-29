"""Compile wrong implementations and require behavioral rejection, not build failure."""
import argparse,json,subprocess,tempfile,re
from pathlib import Path
def tokens(text):return re.findall(r'[A-Za-z_]\w*|\d+|[^\s]',text)
def locate(code,anchor,name):
 pattern=r'\s*'.join(re.escape(t) for t in tokens(anchor));spans=[m.span() for m in re.finditer(pattern,code)]
 if len(spans)!=1:raise RuntimeError('Mutation anchor changed: '+name)
 return spans[0]
def function_body(code,signature,name):
 """Span of the body of the unique definition whose `type name(` opening is `signature`."""
 start,after=locate(code,signature,name);depth=1;i=after
 while i<len(code) and depth:
  depth+=(code[i]=='(')-(code[i]==')');i+=1
 brace=re.compile(r'\s*\{').match(code,i)
 if not brace:raise RuntimeError('Mutation anchor changed: '+name)
 begin=brace.end();depth=1;i=begin
 while i<len(code) and depth:
  depth+=(code[i]=='{')-(code[i]=='}');i+=1
 if depth:raise RuntimeError('Mutation anchor changed: '+name)
 return begin,i
def gut(signature,body,name):
 def mutate(code):
  begin,end=function_body(code,signature,name);return code[:begin]+body+code[end:]
 return mutate
def regex_mutation(regex,replacement,name):
 def mutate(code):
  mutated,count=re.subn(regex,replacement,code)
  if count!=1:raise RuntimeError('Mutation anchor changed: '+name)
  return mutated
 return mutate
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--lab',type=Path,required=True);p.add_argument('--cc',required=True);a=p.parse_args()
 code=(a.source/'model.c').read_text();results=[]
 mutants=[('user-range','ranges',gut('int cmk_user_range(',' return 1; }','user-range')),
          ('stale-handle','handles',gut('int cmk_handle_valid(',' return o; }','stale-handle')),
          ('writable-code','elf',regex_mutation(r'\(s\.flags\s*&\s*3\)\s*==\s*3','0','writable-code'))]
 with tempfile.TemporaryDirectory(prefix='cmk-mutations-') as temp:
  root=Path(temp)
  for name,group,mutate in mutants:
   mutated=mutate(code)
   for variant,body in [('control',code),('mutant',mutated)]:
    src=root/(name+'-'+variant+'.c');src.write_text(body);exe=root/(name+'-'+variant)
    built=subprocess.run([a.cc,'-std=c17','-O2','-I'+str(a.lab/'include'),str(src),str(a.lab/'tests/contracts.c'),'-o',str(exe)],capture_output=True,text=True)
    if built.returncode:raise RuntimeError(built.stderr)
    observed=subprocess.run([str(exe),group],capture_output=True,text=True)
    expected=observed.returncode==0 if variant=='control' else observed.returncode!=0
    if not expected:raise RuntimeError(name+' '+variant+' wrong result: '+observed.stdout+observed.stderr)
   results.append({'mutation':name,'baseline_passed':True,'compiled_mutant_rejected':True})
 print(json.dumps({'passed':True,'controls':results}))
if __name__=='__main__':main()
