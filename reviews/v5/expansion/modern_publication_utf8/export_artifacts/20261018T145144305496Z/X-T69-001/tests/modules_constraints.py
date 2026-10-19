import subprocess,sys,tempfile
from pathlib import Path
compiler,source,dod,ownership=sys.argv[1:];source=Path(source)
with tempfile.TemporaryDirectory(prefix='cmastery-module-constraints-') as work:
 for name,accept in [('positive',True),('private_reject',False),('key_reject',False),('value_reject',False)]:
  cmd=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(source.parent/'include'),'-I'+str(Path(dod)/'include'),'-I'+str(Path(ownership)/'include'),'-c',str(source/('modules_'+name+'.c')),'-o',str(Path(work)/(name+'.o'))]
  r=subprocess.run(cmd,text=True,capture_output=True);print(' '.join(cmd));print(r.stderr);assert (r.returncode==0)==accept,(name,r.returncode)
