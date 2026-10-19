"""Current T74 host/LLVM evidence and separate Cortex-M4 core link audit."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--report', type=Path, default=ROOT/'reviews/v5/expansion/modern_completion/object_runtime_current_campaign.json')
    parser.add_argument('--reuse-host-report', type=Path)
    parser.add_argument('--reuse-statepoints-report', type=Path)
    args = parser.parse_args()
    if sys.platform == 'win32':
        raise SystemExit('Run this hosted Linux/LLVM/target-audit producer in WSL.')
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    build = ROOT/'build/modern_completion'/('runtime-wrapper-'+stamp)
    logs = ROOT/'reviews/v5/expansion/modern_completion/runs'/('runtime-wrapper-'+stamp)
    build.mkdir(parents=True); logs.mkdir(parents=True)
    digest = lambda p: hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
    extras = ['tools/run_object_runtime.py','labs/protocols/targets/runtime.c',
              'research/v5/modern/object_runtime_resources.json','research/v5/modern/object_runtime_sources.md',
              'research/v5/modern/object_runtime_research_gate.json']
    extra_pins = {p:digest(p) for p in extras}; records=[]
    result={'passed':False, 'status':'failed'}

    def run(command):
        r = subprocess.run(list(map(str,command)),cwd=ROOT,capture_output=True,text=True,timeout=600)
        output=r.stdout+r.stderr; p=logs/(str(len(records))+'.log');p.write_text(output,encoding='utf8')
        records.append({'command':list(map(str,command)),'exit_code':r.returncode,'stdout':output,
                        'log':p.relative_to(ROOT).as_posix(),'log_sha256':digest(p.relative_to(ROOT))})
        if r.returncode: raise RuntimeError(output[-2000:])
        return output

    def checked_receipt(path):
        r=json.loads(path.read_text(encoding='utf8'))
        if not r.get('passed') or not r.get('inspected_paths'): raise RuntimeError('Unpassed or unbound report '+str(path))
        for p,h in r['inspected_paths'].items():
            if digest(p)!=h: raise RuntimeError('Stale source pin: '+p)
        return r

    try:
        gate=json.loads((ROOT/'research/v5/modern/object_runtime_research_gate.json').read_text(encoding='utf8'))
        assert gate['status']=='passage-inspected'
        for p,h in gate['research_input_sha256'].items(): assert digest(p)==h,p
        host=args.reuse_host_report or logs/'host.json'
        if not args.reuse_host_report: run([sys.executable,ROOT/'tools/run_expansion_family.py','object_runtime','--report',host])
        result=checked_receipt(host)
        compiler=args.reuse_statepoints_report or logs/'statepoints.json'
        if not args.reuse_statepoints_report: run([sys.executable,ROOT/'tools/run_object_runtime_statepoints.py','--report',compiler])
        sp=checked_receipt(compiler)
        result['host_evidence']={'path':host.resolve().relative_to(ROOT).as_posix(),'sha256':digest(host.resolve().relative_to(ROOT)),'reused_current_observations':bool(args.reuse_host_report)}
        result['compiler_evidence']={'path':compiler.resolve().relative_to(ROOT).as_posix(),'sha256':digest(compiler.resolve().relative_to(ROOT)),'reused_current_observations':bool(args.reuse_statepoints_report)}
        for cc in ['gcc','clang']:
            run([cc,'-std=c2x','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only','-I'+str(ROOT/'labs/object_runtime/include'),ROOT/'assessor/object_runtime/object_runtime.c'])
        for source,name in [(ROOT/'assessor/object_runtime/object_runtime.c','core'),(ROOT/'labs/protocols/targets/runtime.c','runtime')]:
            run(['clang','--target=arm-none-eabi','-mcpu=cortex-m4','-mthumb','-std=c17','-ffreestanding','-fno-builtin','-fno-stack-protector','-Wall','-Wextra','-Wpedantic','-Werror','-O2','-I'+str(ROOT/'labs/object_runtime/include'),'-c',source,'-o',build/(name+'.o')])
        run(['arm-none-eabi-ld',build/'core.o',build/'runtime.o','--entry=model_entry','-Map='+str(build/'audit.map'),'-o',build/'audit.elf'])
        undefined=run(['arm-none-eabi-nm','-u',build/'audit.elf'])
        assert not undefined.strip(),undefined
        symbols=run(['arm-none-eabi-nm',build/'audit.elf'])
        result['target_audit']={'status':'linked-no-unresolved-symbols','target':'Cortex-M4 Thumb','scope':'Original portable object_runtime.c only; explicit byte-loop ABI helpers, no hosted allocator. Actual CVM and LLVM/native-stack adapters are hosted and are not included in this target claim.','execution':'not-run','symbols':symbols}
        result['inspected_paths'].update(extra_pins);result['inspected_paths'].update(sp['inspected_paths'])
        result['scope_source_stable']=all(digest(p)==h for p,h in result['inspected_paths'].items())
        assert result['scope_source_stable'],'Sources changed during wrapper'
        result['scope_input_sha256']=hashlib.sha256(json.dumps(result['inspected_paths'],sort_keys=True).encode()).hexdigest()
        result['scope']='Declared hosted anchors/mutations/oracle plus separate actually executed LLVM ABI subset and Cortex-M4 core build/link audit. No physical execution or independent learner assessment.'
        result['passed']=True;result['status']='passed'
    except Exception as error:
        result.update(passed=False,status='failed',error=str(error))
    result['wrapper_executions']=records
    args.report.parent.mkdir(parents=True,exist_ok=True)
    args.report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps({'passed':result['passed'],'report':str(args.report),'error':result.get('error')}))
    return 0 if result['passed'] else 1


if __name__=='__main__':
    raise SystemExit(main())
