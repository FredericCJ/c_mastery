"""Build each original plugin independently and execute the cross-compiler ABI matrix."""
from pathlib import Path
from datetime import datetime, timezone
import argparse, hashlib, json, subprocess, time
ROOT = Path(__file__).resolve().parents[1]
PATHS = ['tools/run_dispatch_plugin_probe.py','labs/dispatch/tests/plugin_compat.c','labs/dispatch/tests/plugin_client_v1.c','labs/dispatch/tests/plugin_client_v2.c','labs/dispatch/tests/plugin_versions.c','labs/dispatch/include/plugin_base.h','labs/dispatch/include/dispatch_plugin.h','labs/dispatch/include/dispatch.h','assessor/dispatch/dispatch_plugin.c','assessor/dispatch/dispatch.c','assessor/dod/dod.c','labs/dod/include/dod.h','fixtures/dispatch/plugin_compat.json']
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser();parser.add_argument('--report',required=True);args=parser.parse_args()
    report=(ROOT/args.report).resolve();assert report.is_relative_to(ROOT/'reviews/v5/expansion')
    stamp='dispatch-plugin-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    build=ROOT/'build/modern_completion'/stamp;logs=report.parent/'runs'/stamp;build.mkdir(parents=True);logs.mkdir(parents=True)
    pins={p:sha(ROOT/p) for p in PATHS};records=[];observations=[]
    def execute(command,label):
        command=list(map(str,command));start=time.monotonic();r=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=60)
        path=logs/(label+'.log');path.write_text(r.stdout+'\n'+r.stderr,encoding='utf8')
        records.append({'command':command,'exit_code':r.returncode,'seconds':time.monotonic()-start,'log':path.relative_to(ROOT).as_posix(),'log_sha256':sha(path)})
        assert r.returncode==0,(label,r.stderr)
        return r.stdout
    spec=json.loads((ROOT/'fixtures/dispatch/plugin_compat.json').read_text())
    flags=['-std=c17','-O2','-fstack-usage','-Wall','-Wextra','-Wpedantic','-Werror','-Ilabs/dispatch/include','-Ilabs/dod/include']
    libraries={}
    for compiler in ['gcc','clang']:
        execute([compiler,'--version'],compiler+'-version');libraries[compiler]=[]
        for case in range(10):
            path=build/f'{compiler}-{case}.so'
            execute([compiler,*flags,'-fPIC','-shared',f'-DDS_PLUGIN_CASE={case}','labs/dispatch/tests/plugin_versions.c','-o',path],f'{compiler}-library-{case}')
            execute(['readelf','-Ws',path],f'{compiler}-symbols-{case}');execute(['readelf','-d',path],f'{compiler}-dependencies-{case}')
            libraries[compiler].append(path)
    for compiler in ['gcc','clang']:
        for client in [1,2]:
            tag=f'{compiler}-client{client}';binary=build/tag;objects=[]
            for index,source in enumerate(['labs/dispatch/tests/plugin_compat.c',f'labs/dispatch/tests/plugin_client_v{client}.c','assessor/dispatch/dispatch_plugin.c','assessor/dispatch/dispatch.c','assessor/dod/dod.c']):
                obj=build/f'{tag}-{index}.o';execute([compiler,*flags,'-c',source,'-o',obj],f'{tag}-compile-{index}');objects.append(obj)
            execute([compiler,*objects,'-ldl','-Wl,--export-dynamic','-o',binary],tag+'-link')
            size=execute(['size',binary],tag+'-size');execute(['objdump','-dr',binary],tag+'-disassembly');execute(['readelf','-d',binary],tag+'-dependencies')
            stack=[]
            for path in build.glob(tag+'-*.su'):
                dest=logs/path.name;dest.write_bytes(path.read_bytes());stack.append({'path':dest.relative_to(ROOT).as_posix(),'sha256':sha(dest)})
            for producer in ['gcc','clang']:
                output=execute([binary,'plugin_compat',*libraries[producer]],tag+'-producer-'+producer)
                policy=spec['clients'][client-1];expected=[f"P {policy['known']} {v} {status} {policy['features'][v]}" for v,status in enumerate(policy['statuses'])]
                assert output.splitlines()[:-1]==expected
                assert 'constructors=10' in output
                observations.append({'host_compiler':compiler,'plugin_compiler':producer,'client':client,'states':output.splitlines(),'size':size,'stack_artifacts':stack})
    assert pins=={p:sha(ROOT/p) for p in PATHS}
    report.write_text(json.dumps({'schema_version':1,'status':'passed','passed':True,'inspected_paths':pins,'scope_source_stable':True,'observations':observations,'executions':records,'limits':['Linux x86-64 System V ABI only; not arbitrary compiler flags, packing, enum widths or calling conventions.','All descriptor object extents and callback contracts are trusted; size metadata is not a sandbox or memory-validity proof.','GNU constructor extension plus Linux loader behavior, distinct from ISO C.','Successful logical close is not a claim that mappings were physically removed.','Author implementation evidence, not independent learner assessment.']},indent=2)+'\n',encoding='utf8')
    print('dispatch plugin:8 independent compiler/client combinations,80 descriptor admissions matched the fixture model')
if __name__=='__main__': main()
