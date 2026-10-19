"""Check an explicit completed parser import; never locate assessor sources."""
import argparse,subprocess,tempfile
from pathlib import Path

def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--parser-source',required=True,type=Path)
 p.add_argument('--parser-include',required=True,type=Path)
 p.add_argument('--implementation',type=Path,default=Path(__file__).with_name('stream_stub.c'))
 p.add_argument('--compiler',default='cc');a=p.parse_args();here=Path(__file__).resolve().parent
 for source in (a.parser_source,a.implementation,a.parser_include/'checkpoint_depth.h'):
  if not source.is_file():p.error('Missing explicit learner input: '+str(source))
 with tempfile.TemporaryDirectory(prefix='pty-parser-') as directory:
  obj=Path(directory)/'parser.o';exe=Path(directory)/'stream'
  common=[a.compiler,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror']
  subprocess.run([*common,'-ffreestanding','-fno-builtin','-I'+str(a.parser_include.resolve()),'-c',str(a.parser_source.resolve()),'-o',str(obj)],check=True)
  undefined=subprocess.check_output(['nm','-u',str(obj)],text=True)
  if undefined.strip():raise RuntimeError('Profile-C parser has undeclared link dependencies: '+undefined)
  subprocess.run([*common,'-I'+str(a.parser_include.resolve()),'-I'+str(here),str(here/'stream_tests.c'),str(a.implementation.resolve()),str(obj),'-o',str(exe)],check=True)
  for case in ('segmentation','reconnect'):subprocess.run([str(exe),case],check=True,timeout=20)
 return 0
if __name__=='__main__':raise SystemExit(main())
