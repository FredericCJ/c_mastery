"""Copied public C17 runner. Completed providers are always explicit inputs."""
import argparse,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent;ROOT=HERE.parents[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('kind',choices=['contracts','boundaries','device','runtime']);p.add_argument('--source',type=Path);p.add_argument('--model',type=Path);p.add_argument('--cvm',type=Path);p.add_argument('--cc',default='gcc');p.add_argument('--build',type=Path,required=True);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=False)
 includes=['-I'+str(HERE/'contracts'),'-I'+str(HERE/'device'),'-I'+str(HERE/'runtime'),'-I'+str(ROOT/'labs/kernel/include'),'-I'+str(ROOT/'labs/runtime/include')]
 if a.kind in ('contracts','boundaries'):files=[HERE/'contracts'/('tests.c' if a.kind=='contracts' else 'boundaries.c'),a.source or HERE/'contracts/contracts.c',a.model or ROOT/'labs/kernel/learner/model.c'];groups=('tags','reservations','frames','allocation','lifecycle') if a.kind=='contracts' else ('permissions','scheduling','images','archives','handles','lines')
 elif a.kind=='device':files=[HERE/'device/tests.c',a.source or HERE/'device/device.c'];groups=('pci','queue','geometry','ordering','durability')
 else:
  if not a.cvm:p.error('runtime requires --cvm pointing to your explicit portable VM core')
  files=[HERE/'runtime/hosted.c',HERE/'runtime/probe.c',a.cvm];groups=(None,)
 exe=a.build/'probe';subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra',*includes,*map(str,files),'-o',str(exe)],check=True)
 for group in groups:subprocess.run([str(exe),*([group] if group else [])],check=True)
if __name__=='__main__':main()
