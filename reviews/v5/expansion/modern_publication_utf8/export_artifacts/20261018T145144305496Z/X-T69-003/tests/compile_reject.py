import subprocess,sys
from pathlib import Path
compiler,source=sys.argv[1:]
r=subprocess.run([compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only',source],capture_output=True,text=True,timeout=20)
diagnostic=r.stderr.lower()
need=('empty','no members') if 'empty_struct' in source else ('incompatible',)
if r.returncode==0 or Path(source).name.lower() not in diagnostic or not any(n in diagnostic for n in need):
    raise SystemExit('Expected semantic constraint diagnostic absent: '+repr(r))
print('expected C17 diagnostic:',Path(source).name)
