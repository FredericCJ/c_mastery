"""Build a learner-selected gadget in an isolated output tree; never load it."""
from pathlib import Path
import argparse,shutil,subprocess,hashlib,json
HERE=Path(__file__).resolve().parent;ROOT=HERE.parents[3]
p=argparse.ArgumentParser();p.add_argument('--kernel-headers',type=Path,required=True);p.add_argument('--dummy-source',type=Path,required=True);p.add_argument('--directory',type=Path,required=True);p.add_argument('--source',type=Path);p.add_argument('--payload',type=Path);p.add_argument('--foundation',type=Path);p.add_argument('--cc',default='gcc-15');p.add_argument('--cflags',default='');a=p.parse_args();w=a.directory.resolve();w.mkdir(parents=True,exist_ok=True);(w/'include').mkdir(exist_ok=True);(w/'headers').mkdir(exist_ok=True)
inputs={}
def copy(src,dst):
 src=src.resolve();inputs[str(src)]=hashlib.sha256(src.read_bytes()).hexdigest();shutil.copyfile(src,dst)
copy(a.source or HERE/'dg_gadget.c',w/'dg_gadget.c');copy(a.payload or HERE.parent/'diagnostic.c',w/'diagnostic.c');copy(a.foundation or ROOT/'labs/foundations/src/foundation.c',w/'foundation.c');copy(a.dummy_source,w/'dummy_hcd.c');copy(HERE/'Makefile',w/'Makefile')
for f in (HERE/'headers').iterdir():copy(f,w/'headers'/f.name)
for f in [HERE.parent/'diagnostic.h',ROOT/'labs/foundations/include/foundation.h']:copy(f,w/'include'/f.name)
cmd=['make','-C',str(a.kernel_headers.resolve()),'M='+str(w),'CC='+a.cc,'KCFLAGS='+a.cflags,'modules'];r=subprocess.run(cmd,capture_output=True,text=True);(w/'build.log').write_text(r.stdout+r.stderr);(w/'build.json').write_text(json.dumps(dict(command=cmd,returncode=r.returncode,input_sha256=inputs),indent=2)+'\n');print(r.stdout+r.stderr);raise SystemExit(r.returncode)
