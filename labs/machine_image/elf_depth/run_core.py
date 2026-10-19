"""Visible ELF contract check; explicit implementation path, never assessor discovery."""
import argparse,json,struct,subprocess,tempfile
from pathlib import Path
from cases import image

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--implementation',type=Path,required=True);p.add_argument('--cc',default='cc');a=p.parse_args()
    here=Path(__file__).resolve().parent;source=a.implementation.resolve()
    with tempfile.TemporaryDirectory(prefix='cm-elf-') as directory:
        exe=Path(directory)/'inspect';subprocess.run([a.cc,'-std=c17','-Wall','-Wextra','-Werror','-O2','-I'+str(here),str(here/'inspect.c'),str(source),'-o',str(exe)],check=True)
        valid=image();invalid=bytearray(valid);struct.pack_into('<Q',invalid,96,0x60)
        framed=b''.join(struct.pack('<I',len(b))+b for b in (valid,invalid,valid[:63]))
        run=subprocess.run([str(exe)],input=framed,capture_output=True,check=True,timeout=10);rows=[json.loads(line) for line in run.stdout.splitlines()]
        expected=[{'status':0,'header':[64,1,2,62,0x400000,0,1,0],'sections':[],'programs':[[1,5,0x1000,0x400000,0,0x30,0x50,0x1000]],'symbols':[]},{'status':3},{'status':2}]
        if rows!=expected:print(json.dumps({'passed':False,'observed':rows}));return 1
        print(json.dumps({'passed':True,'cases':3,'scope':'Visible ordinary segment/range/truncation contract only'}));return 0
if __name__=='__main__':raise SystemExit(main())
