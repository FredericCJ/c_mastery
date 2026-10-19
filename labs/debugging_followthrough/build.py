"""Ordinary export build; reference sources require explicit paths."""
import argparse,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];LAB=Path(__file__).resolve().parent
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--compiler',default='gcc');p.add_argument('--diagnosis-source',default=str(LAB/'diagnosis.c'));p.add_argument('--ledger-source',default=str(ROOT/'labs/architecture_frame/learner.c'));p.add_argument('--frame-source',default=str(ROOT/'labs/interrupt_depth/frame/frame.c'));p.add_argument('--firmware-source',default=str(ROOT/'labs/firmware_io/src/firmware_io.c'));a=p.parse_args()
 return subprocess.call([a.compiler,'-std=c17','-O2','-Wall','-Wextra','-Werror','-pedantic',*['-I'+str(ROOT/s) for s in ('labs/debugging_followthrough','labs/architecture_frame','labs/interrupt_depth/frame','labs/firmware_io/include')],str(LAB/'tests.c'),a.diagnosis_source,a.ledger_source,a.frame_source,a.firmware_source,'-o',a.output])
if __name__=='__main__':raise SystemExit(main())
