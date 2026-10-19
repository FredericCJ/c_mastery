"""Required-diagnostic compilation anchors. No undefined expression is executed."""
import argparse,pathlib,subprocess,tempfile
p=argparse.ArgumentParser();p.add_argument('--cc',required=True);a=p.parse_args()
cases={
 'constraint_const':'int main(void){const int value=1;value=2;return value;}',
 'constraint_arity':'int f(int x){return x;} int main(void){return f();}',
 'constraint_conflict':'int f(int); double f(int); int main(void){return 0;}',
 'positive':'#include <stdint.h>\nstatic int twice(int x){return x+x;}\nint main(void){_Static_assert(sizeof(uint32_t)==4,"width");return twice(2)!=4;}',
}
with tempfile.TemporaryDirectory(prefix='cmastery-compile-') as tmp:
 for name,source in cases.items():
  path=pathlib.Path(tmp)/(name+'.c');path.write_text(source)
  r=subprocess.run([a.cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic-errors','-c',str(path),'-o',str(path.with_suffix('.o'))],capture_output=True,text=True,timeout=10)
  expected_success=name=='positive'
  if (r.returncode==0)!=expected_success:raise SystemExit(name+' unexpected compiler outcome\n'+r.stderr)
print('4 compilation outcomes: 3 required rejections, 1 defined positive control; no runtime UB claim')
