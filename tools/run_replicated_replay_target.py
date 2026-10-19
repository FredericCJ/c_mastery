"""Actual PPC64 big-endian replay and bounded dependency-audit controls."""
from datetime import datetime,timezone
import argparse
import hashlib
import json
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import time
from common import ROOT,dump,sha
from v5_common import pins,tree_files
from verify_replicated_replay_depth import cases,trace,order_code

REPORT='research/v5/modern/replicated_replay_target_validation.json'
SOURCES=['labs/smp_depth/acpi/big_endian.S','labs/replicated/replay_depth/freestanding/target.c',
 'assessor/replicated/replay_depth/replay_core.c','assessor/replicated/replay_depth/replay_wire.c',
 'assessor/replicated/replicated.c','assessor/replicated/hashing/hashing.c',
 'assessor/api_depth/architecture/core.c','assessor/api_depth/architecture/effects.c']
INCLUDES=['labs/replicated/replay_depth/freestanding','labs/replicated/replay_depth',
 'labs/replicated/hashing','labs/replicated/include','labs/dod/include','labs/storage/include','labs/api_depth/architecture']
def run(report=REPORT):
    runs=ROOT/'reviews/v5/expansion/modern_completion/runs'/('replay-target-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
    fixed=set(SOURCES+['tools/run_replicated_replay_target.py','tools/verify_replicated_replay_depth.py','tools/common.py','tools/v5_common.py','labs/replicated/replay_depth/REPLAY.md','assessor/replicated/replay_depth/ANALYSIS.md','labs/replicated/replay_depth/freestanding/hidden_time.c'])
    for folder in INCLUDES:
        fixed.update(p.relative_to(ROOT).as_posix() for p in tree_files(folder) if p.suffix=='.h')
    before=pins([ROOT/p for p in fixed]);needed=set(fixed);external={};records=[];artifacts=[];observations=[]
    def execute(cmd,label,expected=0):
        start=time.monotonic();cmd=list(map(str,cmd))
        try:p=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=120);code=p.returncode;out=p.stdout+'\n'+p.stderr
        except subprocess.TimeoutExpired as e:code=None;out=str(e)
        log=runs/(str(len(records))+'-'+label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=code,expected_exit=expected,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),seconds=time.monotonic()-start));assert code==expected,(label,code,out);return out
    def artifact(p):artifacts.append(dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)))
    qemu=ROOT/'build/checkpoint-tools/qemu-user-local/usr/bin/qemu-ppc64'
    linker=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21' # preserve the driver symlink spelling
    for tool in ['clang','gcc','nm','readelf']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],'version-'+tool)
    for p in [qemu,linker,Path(sys.executable).resolve()]:external[str(p.absolute())]=sha(p)
    execute([qemu,'--version'],'version-qemu');execute([linker,'--version'],'version-linker');execute(['uname','-a'],'environment')
    selected=list(cases());selected=selected[:64]+selected[-64:]
    rows=['{%d,%d,%d,%d,{%s}}'%(n,l,r,order_code(o),','.join(str(x) for x in bytes.fromhex(trace(n,l,r,o)[0]))) for n,l,r,o in selected]
    header=runs/'replay_vectors.h';header.write_text('static const struct {unsigned n,local,remote,order;unsigned char digest[32];} vectors[]={'+','.join(rows)+'};\n',encoding='utf8');artifact(header)
    def deps(path):
        raw=path.read_text().replace('\\\n',' ');words=shlex.split(raw.split(':',1)[1])
        for word in words:
            p=Path(word);p=(ROOT/p).resolve() if not p.is_absolute() else p.resolve()
            if p.is_relative_to(runs):continue
            if p.is_relative_to(ROOT):needed.add(p.relative_to(ROOT).as_posix())
            else:external.setdefault(str(p),sha(p))
        artifact(path)
    for mode,mutation in [('O0',0),('O3',0),('wrong-endian',11)]:
        build=runs/mode;build.mkdir();objects=[]
        for i,source in enumerate(SOURCES):
            obj=build/(str(i)+'.o');dep=build/(str(i)+'.d')
            cmd=['clang','--target=powerpc64-linux-gnu','-mabi=elfv2','-O0' if mode=='O0' else '-O3','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-pic','-ffunction-sections','-fdata-sections','-MD','-MF',dep]
            if source.endswith('.c'):cmd+=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-DRPR_MUTATION='+str(mutation)]
            for directory in [str(runs)]+INCLUDES:cmd+=['-I',directory]
            execute(cmd+['-c',source,'-o',obj],mode+'-compile-'+str(i));deps(dep);objects.append(obj)
        image=build/'replay';mapfile=build/'link.map'
        execute(['clang','--target=powerpc64-linux-gnu','-mabi=elfv2','-nostdlib','-static','--ld-path='+str(linker),'-Wl,-e,_start','-Wl,--gc-sections','-Wl,-Map='+str(mapfile),*objects,'-o',image],mode+'-link')
        artifact(image);artifact(mapfile)
        elf=execute(['readelf','-h',image],mode+'-elf');assert all(s in elf for s in ['big endian','PowerPC64','abiv2'])
        assert not execute(['nm','-u',image],mode+'-undefined').strip()
        assert 'NEEDED' not in execute(['readelf','-d',image],mode+'-dynamic')
        out=execute([qemu,image],mode+'-execute',1 if mutation else 0)
        assert ('RP_CHECK big-endian canonical-state' if mutation else 'REPLAY_BE_PASS traces128 canonical-full-state') in out
        observations.append(dict(mode=mode,traces=128,semantic_rejection=bool(mutation),passed=True))
    allowed={'rp_step','ar_decide','ar_retry','ar_feedback','memcpy','memset'}
    audits=[]
    for opt in ['-O0','-O3']:
        obj=runs/('core'+opt+'.o')
        execute(['gcc','-std=c17',opt,'-ffreestanding','-fno-builtin',*[v for p in INCLUDES for v in ['-I',p]],'-c',SOURCES[2],'-o',obj],'purity-compile'+opt)
        out=execute(['nm','-u',obj],'purity-symbols'+opt);symbols={line.split()[-1] for line in out.splitlines() if line.strip()};assert symbols<=allowed,symbols
        audits.append(dict(optimization=opt,symbols=sorted(symbols),allowed=sorted(allowed),passed=True));artifact(obj)
    bad=runs/'hidden-time.o';execute(['gcc','-std=c17','-O3','-c','labs/replicated/replay_depth/freestanding/hidden_time.c','-o',bad],'negative-purity-compile')
    out=execute(['nm','-u',bad],'negative-purity-symbols');symbols={line.split()[-1] for line in out.splitlines() if line.strip()};assert 'time' in symbols and not symbols<=allowed;artifact(bad)
    assert needed<=set(before),'unannounced repository dependency '+str(needed-set(before))
    selectedpins={p:before[p] for p in needed};stable=selectedpins==pins([ROOT/p for p in needed]);external_stable=all(Path(p).is_file() and sha(Path(p))==h for p,h in external.items())
    result=dict(status='passed' if stable and external_stable else 'failed',passed=stable and external_stable,producer='tools/run_replicated_replay_target.py',recorded_at=datetime.now(timezone.utc).isoformat(),input_sha256=selectedpins,source_stable=stable,external_input_sha256=external,external_inputs_stable=external_stable,executions=records,generated_artifacts=artifacts,observations=observations,purity_audits=audits,hidden_time_rejected=True,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limitations=['PPC64 Linux-user emulation of a static freestanding core/codec slice; no POSIX durability on that target.','The undefined-symbol audit is a bounded external check, not ISO C purity enforcement or a proof against hidden globals.'])
    dump(ROOT/report,result);print(json.dumps(dict(passed=result['passed'],observations=len(observations),commands=len(records),report=report)));return result['passed']
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();raise SystemExit(0 if run(a.report) else 1)
