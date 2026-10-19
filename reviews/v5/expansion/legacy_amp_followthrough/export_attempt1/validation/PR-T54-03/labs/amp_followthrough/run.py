"""Compile selected learner/reference policy and execute a mechanism test."""
from pathlib import Path
import argparse,json,subprocess
ROOT=Path(__file__).resolve().parents[2];LAB=Path(__file__).resolve().parent
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,default=LAB);p.add_argument('--providers',type=Path,default=LAB/'providers.json');p.add_argument('--build',type=Path,required=True);p.add_argument('--cc',choices=['gcc','clang'],default='gcc');p.add_argument('--optimization',default='O2');p.add_argument('--group',choices=['images','boot','ring','cache','receipt','doorbell','buffers','recovery','threaded'],required=True);p.add_argument('--define',action='append',default=[]);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=False);provider={k:ROOT/v for k,v in json.loads(a.providers.read_text()).items()};source=a.source.resolve();group=a.group
 if group in ('images','boot','ring','cache','receipt'):files=[LAB/'core/tests.c',source/'core/amp.c'];args=[group]
 elif group=='doorbell':files=[LAB/'core/doorbell_tests.c',source/'core/doorbell.c',source/'core/amp.c'];args=[]
 elif group=='buffers':files=[LAB/'core/buffer_tests.c',source/'core/buffers.c',*[provider[k] for k in ('dma','lease','bridge')]];args=[]
 else:files=[LAB/'core'/('thread_tests.c' if group=='threaded' else 'recovery_tests.c'),source/'core/amp.c'];args=[]
 inc=[LAB/'core',ROOT/'labs/peripheral_depth/dma',ROOT/'labs/interrupt_depth/lease'];exe=a.build/'test';cmd=[a.cc,'-std=c17','-'+a.optimization,'-Wall','-Wextra','-Werror','-pthread',*['-I'+str(x) for x in inc],*['-D'+d for d in a.define],*map(str,files),'-o',str(exe)];subprocess.run(cmd,check=True);return subprocess.run([str(exe),*args],timeout=60).returncode
if __name__=='__main__':raise SystemExit(main())
