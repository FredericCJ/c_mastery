"""Public C17 contract runner; all completed producers must be named explicitly."""
import argparse,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent;LABS=HERE.parent
p=argparse.ArgumentParser();p.add_argument('--cc',default='gcc');p.add_argument('--build',type=Path,required=True);p.add_argument('--source',type=Path,default=HERE)
defaults={'b04':LABS/'b04_depth/src/depth.c','b05':LABS/'b05_depth/src/depth.c','systems':LABS/'b045_systems/src/systems.c','bridge':LABS/'modern_bridge/src/bridge.c','hashing':LABS/'replicated/hashing/hashing.c'}
for name,path in defaults.items():p.add_argument('--'+name,type=Path,default=path)
p.add_argument('--part',choices=['transfers','once','reclamation','response','authentication'],default='transfers');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
parts={'transfers':('transfers.c',[a.b04,a.b05,a.systems]),'once':('once.c',[a.bridge]),'reclamation':('list.c',[a.bridge]),'response':('response.c',[a.b05]),'authentication':('auth.c',[a.b05,a.hashing])};filename,providers=parts[a.part]
incs=[HERE/a.part,LABS/'b04_depth/include',LABS/'b05_depth/include',LABS/'b045_systems/include',LABS/'modern_bridge/include',LABS/'replicated/hashing'];exe=a.build/a.part
subprocess.run([a.cc,'-std=c17','-O0','-Wall','-Wextra','-Wpedantic','-Werror',*['-I'+str(x) for x in incs],str(a.source/a.part/filename),*map(str,providers),str(HERE/a.part/'tests.c'),'-o',str(exe)],check=True)
raise SystemExit(subprocess.run([str(exe)]).returncode)
