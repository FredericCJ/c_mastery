"""Observe C semantic laws, real retained closures and Rust diagnostic comparisons."""
from pathlib import Path
from datetime import datetime,timezone
import argparse,hashlib,json,os,subprocess,time
ROOT=Path(__file__).resolve().parents[1]
PATHS=['tools/run_dispatch_laws_probe.py','labs/dispatch/tests/laws.c','labs/dispatch/tests/deferred.c','labs/dispatch/tests/trait_comparison.rs','labs/dispatch/include/dispatch_laws.h','labs/dispatch/include/dispatch_deferred.h','labs/dispatch/include/dispatch_extensions.h','labs/dispatch/include/capture.h','labs/dispatch/include/dispatch.h','labs/ownership/include/ownership.h','labs/dod/include/dod.h','assessor/dispatch/dispatch_laws.c','assessor/dispatch/dispatch_deferred.c','assessor/dispatch/dispatch_extensions.c','assessor/dispatch/capture.c','assessor/dispatch/dispatch.c','assessor/dod/dod.c','assessor/ownership/ownership.c','fixtures/dispatch/laws.json']
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);p.add_argument('--rustc',default='rustc');p.add_argument('--rust-library-path');a=p.parse_args();report=(ROOT/a.report).resolve();assert report.is_relative_to(ROOT/'reviews/v5/expansion');stamp='dispatch-laws-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');build=ROOT/'build/modern_completion'/stamp;logs=report.parent/'runs'/stamp;build.mkdir(parents=True);logs.mkdir(parents=True);records=[];observations=[];pins={p:sha(ROOT/p) for p in PATHS};env=os.environ.copy()
 if a.rust_library_path:env['LD_LIBRARY_PATH']=a.rust_library_path
 def run(cmd,label,expected=0,rust=False):
  cmd=list(map(str,cmd));begin=time.monotonic();r=subprocess.run(cmd,cwd=ROOT,env=env if rust else None,capture_output=True,text=True,timeout=90);path=logs/(label+'.log');path.write_text(r.stdout+'\n'+r.stderr,encoding='utf8');records.append({'command':cmd,'exit_code':r.returncode,'expected_exit_code':expected,'seconds':time.monotonic()-begin,'log':path.relative_to(ROOT).as_posix(),'log_sha256':sha(path),'environment_override':{'LD_LIBRARY_PATH':a.rust_library_path} if rust and a.rust_library_path else {}});assert r.returncode==expected,(label,r.returncode,r.stderr);return r
 for compiler in ['gcc','clang']:
  run([compiler,'--version'],compiler+'-version');flags=['-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-fstack-usage','-Ilabs/dispatch/include','-Ilabs/ownership/include','-Ilabs/dod/include','-pthread']
  for kind in ['laws','deferred']:
   sources=['labs/dispatch/tests/'+kind+'.c','assessor/dispatch/dispatch_'+kind+'.c','assessor/ownership/ownership.c']
   if kind=='laws':sources+=['assessor/dispatch/dispatch_extensions.c','assessor/dispatch/capture.c','assessor/dispatch/dispatch.c','assessor/dod/dod.c']
   tag=compiler+'-'+kind;objects=[]
   for i,source in enumerate(sources):
    obj=build/f'{tag}-{i}.o';run([compiler,*flags,'-c',source,'-o',obj],tag+f'-compile-{i}');objects.append(obj)
   binary=build/tag;run([compiler,*objects,'-pthread','-ldl','-o',binary],tag+'-link');size=run(['size',binary],tag+'-size').stdout;run(['objdump','-dr',binary],tag+'-disassembly');stack=[]
   for su in build.glob(tag+'-*.su'):
    dest=logs/su.name;dest.write_bytes(su.read_bytes());stack.append({'path':dest.relative_to(ROOT).as_posix(),'sha256':sha(dest)})
   if kind=='laws':layout=run([binary,'layout'],tag+'-layout').stdout.strip()
   else:
    layout_source=build/(tag+'-layout.c');layout_source.write_text('#include "dispatch_deferred.h"\n#include <stdio.h>\nint main(void){printf("%zu %zu %zu\\n",sizeof(DSDeferred),sizeof(DSDeferredToken),sizeof(OwShared));return 0;}\n',encoding='utf8')
    layout_binary=build/(tag+'-layout');run([compiler,*flags,layout_source,'-o',layout_binary],tag+'-layout-compile');layout=run([layout_binary],tag+'-layout').stdout.strip()
   for case in ['copy_laws','subobjects','capture_admission','closure_histories'] if kind=='laws' else ['retained_closure']:run([binary,case],tag+'-'+case)
   observations.append({'compiler':compiler,'kind':kind,'layout':layout,'size':size,'stack_artifacts':stack})
 version=run([a.rustc,'--version'], 'rust-version',rust=True).stdout.strip();source='labs/dispatch/tests/trait_comparison.rs';binary=build/'rust-positive'
 run([a.rustc,'--edition=2021',source,'-o',binary],'rust-positive-compile',rust=True);positive=run([binary],'rust-positive-run',rust=True).stdout.strip();assert positive=='length=1 copy=3 marker=0 trait_ref=16',positive
 diagnostics=[]
 for mode,code in [('wrong_signature','E0053'),('missing_method','E0046'),('no_trait_import','E0599'),('escaping_borrow','E0515'),('once_twice','E0382')]:
  result=run([a.rustc,'--edition=2021','--cfg',mode,source,'-o',build/('rust-'+mode)],'rust-'+mode,1,True);assert code in result.stderr;diagnostics.append({'configuration':mode,'diagnostic':code})
 assert pins=={p:sha(ROOT/p) for p in PATHS}
 report.write_text(json.dumps({'schema_version':1,'status':'passed','passed':True,'inspected_paths':pins,'scope_source_stable':True,'observations':observations,'rust':{'version':version,'positive':positive,'diagnostics':diagnostics,'compiler_path':a.rustc},'executions':records,'limits':['Rust2021 language mode and the recorded privately provisioned compiler; no system installation or host setting change.','Rust accepts the intentionally wrong length/copy relation; traits check signatures, not arbitrary semantic laws.','Observed marker and trait-reference sizes are target/compiler facts, not a C ABI guarantee.','No dangling C pointer is dereferenced and no deliberately incompatible function call is executed.','C costs include full linked dependencies; no elapsed-time advantage for a dispatch form is claimed.']},indent=2)+'\n',encoding='utf8');print('dispatch laws:two C compiler controls and five intended Rust diagnostics passed; type-correct semantic counterexample executed')
if __name__=='__main__':main()
