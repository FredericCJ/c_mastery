"""Compare explicit learner record and mapping imports under the same bounds."""
import argparse,subprocess,tempfile
from pathlib import Path
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--mapping-source',required=True,type=Path);p.add_argument('--mapping-include',required=True,type=Path);p.add_argument('--implementation',type=Path,default=Path(__file__).with_name('record_stub.c'));p.add_argument('--compiler',default='cc');a=p.parse_args();here=Path(__file__).resolve().parent
 for path in (a.mapping_source,a.mapping_include/'map.h',a.implementation):
  if not path.is_file():p.error('Missing explicit learner input: '+str(path))
 with tempfile.TemporaryDirectory(prefix='mapping-workload-') as directory:
  for case in ('tests','bench'):
   exe=Path(directory)/case;command=[a.compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-O2','-I'+str(here),str(here/(case+'.c')),str(a.implementation.resolve()),'-Wl,--wrap=pread,--wrap=__pread_chk','-o',str(exe)]
   if case=='bench':command+=['-I'+str(a.mapping_include.resolve()),str(a.mapping_source.resolve())]
   subprocess.run(command,check=True);subprocess.run([str(exe)],check=True,timeout=30)
 return 0
if __name__=='__main__':raise SystemExit(main())
