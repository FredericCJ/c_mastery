"""Compile one explicit assessor mutation and demand semantic rejection.

Mutation definitions stay in the private assessor export. Compilation errors
are infrastructure failures, never accepted as a successful negative control.
"""
import argparse,json,pathlib,re,subprocess,tempfile
p=argparse.ArgumentParser();p.add_argument('--cc',required=True);p.add_argument('--source',required=True);p.add_argument('--test',required=True);p.add_argument('--mutations',required=True);p.add_argument('--case',required=True);p.add_argument('--include',action='append',default=[]);p.add_argument('--extra',action='append',default=[]);p.add_argument('--define',action='append',default=[]);a=p.parse_args()
changes=json.loads(pathlib.Path(a.mutations).read_text())[a.case]
source=pathlib.Path(a.source).read_text()
for before,after in changes:
 pattern=r'\s*'.join(re.escape(t) for t in re.findall(r'[A-Za-z_]\w*|\d+|[^\s]',before))
 if len(re.findall(pattern,source))!=1:raise SystemExit('Mutation anchor must occur exactly once: '+before)
 source=re.sub(pattern,lambda _:after,source,count=1)
with tempfile.TemporaryDirectory(prefix='cmastery-mutant-') as d:
 root=pathlib.Path(d);c=root/'mutant.c';c.write_text(source);exe=root/'mutant'
 command=[a.cc,'-std=c17','-g','-Wall','-Wextra','-Wno-misleading-indentation',str(c),a.test,*a.extra,*['-I'+i for i in a.include],*['-D'+i for i in a.define],'-lm','-o',str(exe)]
 build=subprocess.run(command,capture_output=True,text=True)
 if build.returncode:raise SystemExit('Mutation did not compile:\n'+build.stdout+build.stderr)
 result=subprocess.run([str(exe),a.case],capture_output=True,text=True,timeout=10)
 if result.returncode!=1:raise SystemExit(f'Mutation must fail by a contract assertion, got {result.returncode}:\n'+result.stdout+result.stderr)
 print('Semantic rejection confirmed:',a.case)
