"""Observe installed compiler dialect/header capabilities, never assume them."""
import argparse,json,pathlib,subprocess,tempfile
p=argparse.ArgumentParser();p.add_argument('--cc',default='cc');a=p.parse_args()
cases={
 'c17-generic':('-std=c17','int main(void){return _Generic(1,int:0,default:1);}'),
 'c23-nullptr':('-std=c23','int main(void){int *p=nullptr;return p!=nullptr;}'),
 'c23-checked-header':('-std=c23','#include <stdckdint.h>\nint main(void){int result;return ckd_add(&result,1,2);}'),
}
rows=[]
with tempfile.TemporaryDirectory(prefix='cmastery-capabilities-') as tmp:
 for name,(flag,text) in cases.items():
  source=pathlib.Path(tmp)/(name+'.c');source.write_text(text)
  r=subprocess.run([a.cc,flag,'-pedantic-errors','-c',str(source),'-o',str(source.with_suffix('.o'))],capture_output=True,text=True,timeout=10)
  rows.append({'feature':name,'command':[a.cc,flag,'-pedantic-errors','-c',str(source)],'compilation_available':r.returncode==0,'exit_code':r.returncode,'diagnostic':r.stderr,'claim':'bounded compile probe; no complete dialect/library or runtime support claim'})
print(json.dumps(rows,indent=2))
