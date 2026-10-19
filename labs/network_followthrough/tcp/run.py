"""Native public harness: every prerequisite implementation is explicit."""
from pathlib import Path
import argparse,subprocess,sys
HERE=Path(__file__).resolve().parent;LABS=HERE.parents[1]
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True);p.add_argument('--source',type=Path,default=HERE/'tcp.c');p.add_argument('--cc',default='gcc')
for k in ['ip','scope','stream','http','systems']:p.add_argument('--'+k,type=Path,required=True)
a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
subprocess.run([sys.executable,LABS/'network_followthrough/ip/checksum_slice.py',a.systems,a.build/'checksum.c'],check=True)
exe=a.build/'public-test';cmd=[a.cc,'-std=c17','-O0','-Wall','-Wextra','-Wpedantic','-Werror',*['-I'+str(x) for x in [HERE,LABS/'network_followthrough/ip',LABS/'network_followthrough/scope',LABS/'communications/include',LABS/'b045_systems/include']],a.source,HERE/'tests.c',a.ip,a.scope,a.stream,a.http,a.build/'checksum.c','-o',exe]
subprocess.run(list(map(str,cmd)),check=True);raise SystemExit(subprocess.run([exe]).returncode)
