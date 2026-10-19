"""Compile wrong implementations and require behavioral rejection, not build failure."""
import argparse,json,subprocess,tempfile,re
from pathlib import Path
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--lab',type=Path,required=True);p.add_argument('--cc',required=True);a=p.parse_args()
 code=(a.source/'model.c').read_text();results=[]
 mutants=[('user-range','ranges',r'(int cmk_user_range\([^)]*\)\s*\{)[^}]+}',r'\1 return 1; }'),
          ('stale-handle','handles',r'(int cmk_handle_valid\([^)]*\)\s*\{)[^}]+}',r'\1 return o; }'),
          ('writable-code','elf',r'\(s\.flags\s*&\s*3\)\s*==\s*3','0')]
 with tempfile.TemporaryDirectory(prefix='cmk-mutations-') as temp:
  root=Path(temp)
  for name,group,pattern,replacement in mutants:
   mutated,count=re.subn(pattern,replacement,code)
   if count!=1:raise RuntimeError('Mutation anchor changed: '+name)
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
