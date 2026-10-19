"""Actual canonical PR-T59-07 initialization and isolated hosted/target compilation."""
import datetime
import json
import pathlib
import subprocess
import sys
from run_cacheable_boot_depth import ROOT,LAB,PROVIDER,PRIVATE,REF,sha,inputs

def main():
    report=ROOT/'reviews/v5/expansion/cacheable_boot_depth/export_validation.json'
    build=ROOT/'build/cacheable-boot-export'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S');build.mkdir(parents=True)
    before=inputs()
    for p in ['cmastery/cli.py','cmastery/project_assets.py','curriculum/projects/PR-T59-07.md','tools/export_cacheable_boot_depth.py']:
        before[p]=sha(ROOT/p)
    commands=[]
    def run(name,args,negative=None):
        args=[str(a) for a in args];r=subprocess.run(args,cwd=ROOT,text=True,capture_output=True,timeout=180);out=r.stdout+r.stderr
        passed=(r.returncode!=0 and negative in out) if negative else r.returncode==0
        commands.append(dict(id=name,command=args,returncode=r.returncode,stdout=out,passed=passed))
        assert passed,(name,out)
        return out
    workspace=build/'learner'
    run('actual-init',[sys.executable,'-m','cmastery','init','PR-T59-07',workspace])
    public=workspace/'labs/cacheable_boot_depth'
    exported={p.relative_to(LAB).as_posix():sha(p) for p in LAB.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
    assert all(sha(public/p)==h for p,h in exported.items())
    assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*'))
    for n in ['h745_cache.c','selected_startup.c']:
        assert sha(public/'imports'/n)==sha(PROVIDER/n)
        assert sha(public/'imports'/n)!=sha(PRIVATE/n)
    for name,assessor,providers,negative in [('learner',None,PRIVATE,'CB_CHECK:'),('completed',ROOT/'assessor',PRIVATE,None),('family-root',REF,PRIVATE,None),('missing-prerequisites',REF,None,'CB_CHECK:')]:
        folder=build/name;args=['cmake','-S',public,'-B',folder,'-G','Ninja','-DCMAKE_BUILD_TYPE=Debug']
        if assessor:args+=['-DCMASTERY_ASSESSOR_ROOT='+str(assessor)]
        if providers:args+=['-DCB_PROVIDER_SOURCE='+str(providers)]
        run(name+'.configure',args);run(name+'.build',['cmake','--build',folder,'--parallel','4']);run(name+'.test',['ctest','--test-dir',folder,'--output-on-failure'],negative)
    run('absent-provider',['cmake','-S',public,'-B',build/'absent','-DCB_PROVIDER_SOURCE='+str(build/'absent-source')],'Explicit prerequisite source missing')
    for core,cpu in [(0,'cortex-m7'),(1,'cortex-m4')]:
        port=workspace/'targets/h745_port.c';assert sha(port)==sha(PROVIDER/'h745_port.c')
        sources=[REF/'boot.c',PRIVATE/'selected_startup.c',PRIVATE/'h745_cache.c',port,public/'target.c',public/'workload.c'];objects=[]
        flags=['arm-none-eabi-gcc','-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-mthumb','-mcpu='+cpu,'-ffreestanding','-fno-builtin','-fno-stack-protector','-I'+str(public),'-I'+str(public/'imports'),'-DCB_CORE='+str(core),'-DFW_H745_M7='+str(1-core)]
        for i,src in enumerate(sources):
            obj=build/(str(core)+'-'+str(i)+'.o');objects.append(obj);extra=['-Wno-misleading-indentation'] if src.stem=='selected_startup' else []
            run(cpu+'.compile.'+str(i),[*flags,*extra,'-c',src,'-o',obj])
        elf=build/(cpu+'.elf');run(cpu+'.link', [*flags,'-nostdlib',*objects,public/'reset.S','-Wl,-T,'+str(public/('m7.ld' if core==0 else 'm4.ld')),'-o',elf])
        assert not run(cpu+'.undefined',['arm-none-eabi-nm','-u',elf]).strip()
        sections=run(cpu+'.sections',['arm-none-eabi-readelf','-SW',elf]);assert any('.peer' in l and 'NOBITS' in l and '30040000' in l for l in sections.splitlines())
    assert all(sha(ROOT/p)==h for p,h in before.items())
    result=dict(passed=True,source_stable=True,input_sha256=before,commands=commands,exported_artifacts=exported,actual_project='PR-T59-07',private_sources_copied=False,target_execution='not-run',physical_observation='not-run')
    report.parent.mkdir(parents=True,exist_ok=True);report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=True,commands=len(commands),exported=len(exported))))
if __name__=='__main__':main()
