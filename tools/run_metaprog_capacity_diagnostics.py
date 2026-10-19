"""Constraint versus policy diagnostics,32-bit final link/execution and equal-work costs."""
from datetime import datetime,timezone
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
def run():
    paths=['tools/run_metaprog_capacity_diagnostics.py','tools/common.py','labs/metaprog/capacity/include/capacity.h','assessor/metaprog/capacity/capacity.c','assessor/metaprog/capacity/capacity_template.inc','labs/metaprog/capacity/targets/i386.c','labs/metaprog/capacity/targets/i386.S','labs/metaprog/capacity/CMakeLists.txt','labs/metaprog/capacity/tests/tests.c','labs/metaprog/capacity/expect_failure.py','labs/metaprog/include/metaprog.h','assessor/metaprog/metaprog.c','assessor/metaprog/ring_template.inc'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];observations=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('metaprog-capacity-diagnostics-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    env=os.environ.copy();env['LD_LIBRARY_PATH']='/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu'+(':'+env['LD_LIBRARY_PATH'] if env.get('LD_LIBRARY_PATH') else '')
    rust=Path('/home/frede/cmastery-b06-tools/usr/bin/rustc-1.93');linker=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21';qemu=ROOT/'build/checkpoint-tools/qemu-user-local/usr/bin/qemu-i386'
    for p in [rust,linker,qemu]:external[str(p.resolve())]=sha(p.resolve())
    def execute(command,label,expected=0,marker=None):
        command=list(map(str,command));start=time.monotonic();p=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=120,env=env);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=command,exit_code=p.returncode,expected_exit=expected,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker));assert (p.returncode==0 if expected==0 else p.returncode>0) and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja','nm','size','objdump','readelf']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    execute([rust,'--version','--verbose'],'rust-version');execute([linker,'--version'],'linker-version');execute([qemu,'--version'],'qemu-version')
    for executable in [rust,linker,qemu]:
        linked=execute(['ldd',executable],executable.name+'-runtime')
        for item in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',linked,re.MULTILINE):
            p=Path(item).resolve()
            if p.is_file():external[str(p)]=sha(p)
    with tempfile.TemporaryDirectory(prefix='cmastery-capacity-probes-') as name:
        tmp=Path(name);inc=['-I',ROOT/'labs/metaprog/capacity/include','-I',ROOT/'assessor/metaprog/capacity','-I',ROOT/'labs/metaprog/include'];flags=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*inc]
        def source(name,text):
            p=tmp/name;p.write_text(text,encoding='utf8');q=folder/name;shutil.copyfile(p,q);artifacts.append(dict(path=q.relative_to(ROOT).as_posix(),sha256=sha(q)));return p
        prefix='#include "capacity.h"\n#define MPC_MUTATION 0\n'
        for cc in ['gcc','clang']:
            for n in [0,1,3,4,8]:
                typ='Mpc'+str(n) if n in [1,4,8] else 'Mpc4';body=prefix+f'#define MPC_PREFIX test\n#define MPC_RING {typ}\n#define MPC_CAPACITY {n}\n#include "capacity_template.inc"\n';p=source(cc+'-capacity'+str(n)+'.c',body);negative=n in [0,3];execute([cc,*flags,'-fsyntax-only',p],cc+'-constant'+str(n),1 if negative else 0,'MPC_POWER_OF_TWO' if negative else None);observations.append(dict(kind='constant',compiler=cc,capacity=n,rejected=negative))
            p=source(cc+'-nonconstant.c','#include <stddef.h>\nvoid check(size_t n){_Static_assert(n>0,"MPC_RUNTIME_NOT_ICE");}\n');execute([cc,*flags,'-fsyntax-only',p],cc+'-nonconstant',1,'constant');observations.append(dict(kind='diagnostic',compiler=cc,case='nonconstant-ICE',passed=True))
            warning=source(cc+'-warning.c','int main(void){int unused=1;return 0;}\n')
            execute([cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-fsyntax-only',warning],cc+'-warning-accepted',marker='unused');execute([cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror=unused-variable','-fsyntax-only',warning],cc+'-warning-policy',1,'unused');observations.append(dict(kind='diagnostic',compiler=cc,case='warning-versus-policy',passed=True))
            vla=source(cc+'-vla.c','int f(unsigned n){if(!n)return 0;int a[n];a[0]=7;return a[0];}\nint main(void){return f(3)!=7;}\n');exe=tmp/(cc+'vla');execute([cc,'-std=c17','-Wall','-Wextra','-Wpedantic',vla,'-o',exe],cc+'-vla-accepted');execute([exe],cc+'-vla-run');execute([cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror=vla','-fsyntax-only',vla],cc+'-vla-policy',1,'variable length array');observations.append(dict(kind='diagnostic',compiler=cc,case='VLA-supported-policy-refused',passed=True))
            for opt in ['0','3']:
                obj=tmp/(cc+opt+'.o');execute([cc,*flags,'-O'+opt,'-c',ROOT/'assessor/metaprog/capacity/capacity.c','-o',obj],cc+opt+'-object');text=execute(['nm','-u',obj],cc+opt+'-helpers');helpers=[s.split()[-1] for s in text.splitlines() if s.strip()];assert 'mp_storage_bound' in helpers and set(helpers)<={'mp_storage_bound','memcpy','memset','__stack_chk_fail'}
                dis=execute(['objdump','-dr',obj],cc+opt+'-disassembly');assert 'mpc4_push' in dis and 'mpc_push' in dis;sizes=execute(['size',obj],cc+opt+'-size').splitlines()[1].split();observations.append(dict(kind='object',compiler=cc,optimization=opt,text=int(sizes[0]),helpers=helpers,masked_and_seen=bool(re.search(r'\band\b',dis)),limits='Raw disassembly retained; a mask occurrence alone is not a universal cost proof.'))
        positive=source('const_good.rs','struct Ring<const N:usize>{values:[u32;N]}\nfn main(){let r=Ring::<3>{values:[1,2,3]};assert_eq!(r.values.len(),3);}\n');negative=source('const_bad.rs','struct Bad<const N:f64>;\nfn main(){}\n');execute([rust,'--edition=2021',positive,'-o',tmp/'rust-good'],'rust-positive');execute([tmp/'rust-good'],'rust-run');execute([rust,'--edition=2021',negative,'-o',tmp/'rust-bad'],'rust-negative',1,'forbidden');observations.append(dict(kind='rust-const',positive_executed=True,floating_type_rejected=True))
        # Availability is observed, not quietly inferred from a second compiler.
        multilib=source('hosted32.c','#include <stdio.h>\n#include <stdint.h>\nint main(void){printf("%zu\\n",sizeof(void*));return 0;}\n');command=['gcc','-m32',multilib,'-o',tmp/'hosted32'];p=subprocess.run(list(map(str,command)),capture_output=True,text=True);log=folder/'hosted32-availability.log';log.write_text(p.stdout+p.stderr,encoding='utf8');observations.append(dict(kind='hosted32-availability',available=p.returncode==0,command=list(map(str,command)),exit_code=p.returncode,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),limits='Probe result is distinct from the no-libc target below.'))
        targetflags=['clang','--target=i386-linux-gnu','-std=c17','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-pie','-ffunction-sections','-fdata-sections',*inc]
        sources=[ROOT/'labs/metaprog/capacity/targets/i386.c',ROOT/'assessor/metaprog/capacity/capacity.c',ROOT/'assessor/metaprog/metaprog.c',ROOT/'labs/metaprog/capacity/targets/i386.S']
        for mutation in [0,3,4]:
            objects=[]
            for i,path in enumerate(sources):
                obj=tmp/f'i386-{mutation}-{i}.o';execute([*targetflags,'-DMOD_MUTATION='+str(mutation),'-c',path,'-o',obj],f'i386-{mutation}-{i}-compile');objects.append(obj)
            binary=tmp/f'i386-{mutation}';execute(['clang','--target=i386-linux-gnu','-nostdlib','-static','--ld-path='+str(linker),'-Wl,--gc-sections','-Wl,-e,_start',*objects,'-o',binary],f'i386-{mutation}-link')
            undefined=execute(['nm','-u',binary],f'i386-{mutation}-undefined');assert not undefined.strip();header=execute(['readelf','-h',binary],f'i386-{mutation}-header');assert 'ELF32' in header and 'Intel 80386' in header;dynamic=execute(['readelf','-d',binary],f'i386-{mutation}-dynamic');assert 'NEEDED' not in dynamic
            execute([qemu,binary],f'i386-{mutation}-execute',0 if mutation==0 else 1,'MPC32_PASS size_t32 checked-product runtime3' if mutation==0 else 'MPC32_FAIL');observations.append(dict(kind='i386',mutation=mutation,size_t_bits=32,executed=True,no_undefined=True,no_needed=True,expected_refusal=mutation!=0,limits='QEMU Linux-user no-libc ABI execution; not a hosted multilib or physical board run.'))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/'labs/metaprog/capacity','-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');observations.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=observations,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Constraint diagnostics, warning policies and runtime guards are separate observations.','Actual32-bit target uses a minimal original assembly/Linux ABI adapter and QEMU user execution; no hosted libc/physical claim.','Published C17/C23 remain inaccessible.']);dump(ROOT/'research/v5/modern/metaprog_capacity_diagnostics_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(observations))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
