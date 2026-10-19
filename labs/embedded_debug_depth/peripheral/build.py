"""Build the diagnostic composition with explicit learner-completed producers.

Usage: python build.py --source peripheral.c --uart /work/uart/uart.c
 --spi /work/spi/spi.c --dma /work/dma/dma.c --serial /work/serial/serial_depth.c
 --uart-include /work/uart --spi-include /work/spi --dma-include /work/dma
 --serial-include /work/serial/include --output /tmp/diagnostic
No repository assessor discovery or copying occurs.
"""
import argparse,subprocess
from pathlib import Path
PUBLIC=Path(__file__).resolve().parent
def command(cc,flags,source,providers,includes,test,output,defines=()):
 return [cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-I'+str(PUBLIC),*['-I'+str(p) for p in includes],*defines,str(source),*[str(p) for p in providers],str(PUBLIC/test),'-o',str(output)]
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source',required=True);p.add_argument('--cc',default='gcc');p.add_argument('--output',required=True)
 for n in ('uart','spi','dma','serial'):p.add_argument('--'+n,required=True);p.add_argument('--'+n+'-include',required=True)
 a=p.parse_args();raise SystemExit(subprocess.call(command(a.cc,['-O0','-g'],a.source,[getattr(a,n) for n in ('uart','spi','dma','serial')],[getattr(a,n+'_include') for n in ('uart','spi','dma','serial')],'imports.c',a.output)))
