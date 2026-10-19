import subprocess,sys
from pathlib import Path
compiler,here,dod,ownership=sys.argv[1:]
here=Path(here)
flags=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only','-I'+str(here.parent/'include'),'-I'+str(Path(dod)/'include'),'-I'+str(Path(ownership)/'include')]
for name in ['nominal_positive','nominal_receiver_reject','nominal_wrapper_reject','nominal_callback_reject']:
 source=here/(name+'.c');r=subprocess.run([compiler,*flags,str(source)],text=True,capture_output=True,timeout=20)
 if name=='nominal_positive':
  if r.returncode:raise SystemExit('positive nominal control failed: '+r.stderr)
 elif r.returncode==0 or source.name not in r.stderr or not any(word in r.stderr.lower() for word in ['incompatible','not compatible','not compatible with any','compatible type']):raise SystemExit('intended nominal constraint diagnostic absent: '+r.stderr)
 print(name,'positive compiled' if r.returncode==0 else 'constraint rejected',r.stderr.strip())
