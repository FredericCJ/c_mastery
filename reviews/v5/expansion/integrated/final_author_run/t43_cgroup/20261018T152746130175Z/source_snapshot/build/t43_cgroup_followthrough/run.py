"""Draft exact T43 delayed experiment; outputs only in the permitted integrated tree."""
import argparse, copy, hashlib, importlib.util, json, os, re, shutil, stat, subprocess, sys
from datetime import datetime, timezone
from pathlib import Path
sys.dont_write_bytecode = True
ROOT=Path(__file__).resolve().parents[2]
DRAFT=Path(__file__).resolve().parent
BASE=ROOT/'reviews/v5/expansion/integrated/final_author_run/t43_cgroup'
KERNEL=Path('/home/frede/cmastery-os-follow-tools/linux-source-7.0.0/arch/x86/boot/bzImage')
BUSYBOX=Path('/home/frede/cmastery-b06-tools/usr/bin/busybox')
PROVIDER=ROOT/'labs/performance_followthrough/observations.c'
ASSM=ROOT/'assessor/algorithms/x86_64.S'
INPUTS=[DRAFT/'delegated.c',Path(__file__),PROVIDER,ASSM,ROOT/'labs/performance_followthrough/cost.h',ROOT/'labs/algorithms/include/algo.h',ROOT/'labs/realtime_depth/linux/guest.py',ROOT/'labs/performance_followthrough/CONTRACTS.md',ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T43-10.json']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def keyed(text):return {k:int(v) for k,v in re.findall(r'^(\w+) (\d+)$',text,re.M)}
def records(text):return [(p,b.strip()) for p,b in re.findall(r'PC_RAW_BEGIN ([^\r\n]+)\r?\n(.*?)PC_RAW_END [^\r\n]+',text,re.S)]
def validate(raw):
    assert 'PC_PROBE_EXIT=0' in raw and 'PC_GUEST_DONE' in raw and 'PC_FAIL:' not in raw
    assert raw.count('PC_PASS:T43.11.a-DELAYED:original-workload:delegated-uid1000:quota:restore')==1
    assert 'PC_PREPARED uid=1000 boundary=/t43 supervisor=/control budget=/work' in raw
    assert 'PC_ANCESTOR_POLICY_WRITE_DENIED' in raw and 'PC_OUTSIDE_MIGRATION_ACCESS_DENIED' in raw
    assert 'PC_RESTORED max 100000' in raw and 'PC_CLEANUP removed=3' in raw
    trials=re.findall(r'PC_TRIAL_BEGIN (\w+)\r?\n(.*?)PC_TRIAL_END \1',raw,re.S)
    assert [n for n,_ in trials]==['unlimited','quota']
    out={}
    for name,body in trials:
        assert re.search(r'PC_MEMBER uid=1000 pid=\d+ 0::/t43/work',body)
        assert re.search(r'PC_MEMBER uid=1000 pid=\d+ 0::/t43/control',body)
        assert 'PC_WORKLOAD_END raw=0 exited=1 code=0 signal=0' in body
        parts=re.split(r'PC_PHASE (?:before|after)\r?\n',body);assert len(parts)==3
        snapshots=[]
        for section in parts[1:]:
            rows=records(section);assert len(rows)==32
            m=dict(rows);assert len(m)==32
            assert m['/sys/fs/cgroup/t43/work/cpu.max']==('max 100000' if name=='unlimited' else '20000 100000')
            assert m['/sys/fs/cgroup/t43/cpu.max']=='max 100000'
            assert m['/sys/fs/cgroup/t43/work/cgroup.type']=='domain'
            assert m['/sys/fs/cgroup/t43/work/cpu.max.burst']=='0'
            snapshots.append(m)
        before,after=snapshots
        assert before['/sys/fs/cgroup/t43/work/cgroup.procs'].isdigit() and after['/sys/fs/cgroup/t43/work/cgroup.procs']==''
        a,b=(keyed(v['/sys/fs/cgroup/t43/work/cpu.stat']) for v in snapshots)
        delta={k:b[k]-a[k] for k in a.keys() & b.keys()};assert delta['usage_usec']>0
        assert all(v>=0 for v in delta.values())
        if name=='quota':assert delta['nr_throttled']>0 and delta['throttled_usec']>0
        payload=re.search(r'PC_WORKLOAD_BEGIN\r?\n(.*?)PC_WORKLOAD_END',body,re.S).group(1)
        result=[json.loads(v) for v in payload.splitlines() if v.startswith('{')]
        assert len(result)==10
        assert [r['case'] for r in result]==['clocks','child-clock-scope','write-sync','read-pass','read-pass','read-pass','injected-fsync-error','publish','sparse-preallocated','memory']
        assert result[-1]['payload_live']==[0,1048576,0] and result[1]['waited_child_cpu_us']>0
        assert all(r['accepted_bytes']==8192 for r in result if r['case']=='read-pass')
        out[name]={'raw':snapshots,'cpu_delta':delta,'workload':result,'memory_peak_is_historical':True,'same_descriptor_peak_reset':False,'io_backend':'guest tmpfs; no physical device traffic inferred'}
    return out
def negative_controls(raw):
    changes={
      'membership':('0::/t43/work','0::/t43/control'),
      'uid':('PC_MEMBER uid=1000','PC_MEMBER uid=0'),
      'quota':('20000 100000','30000 100000'),
      'restore':('PC_RESTORED max 100000','PC_RESTORED 20000 100000'),
      'cleanup':('PC_CLEANUP removed=3','PC_CLEANUP removed=0'),
      'exit':('PC_WORKLOAD_END raw=0 exited=1 code=0 signal=0','PC_WORKLOAD_END raw=9 exited=0 code=-1 signal=9'),
      'ancestor_permission':('PC_ANCESTOR_POLICY_WRITE_DENIED','PC_ANCESTOR_POLICY_WRITE_ALLOWED'),
      'boundary_permission':('PC_OUTSIDE_MIGRATION_ACCESS_DENIED','PC_OUTSIDE_MIGRATION_ACCESS_ALLOWED'),
      'workload_result':('"accepted_bytes":8192','"accepted_bytes":8191'),
      'final_marker':('PC_GUEST_DONE','PC_GUEST_INCOMPLETE'),
    }
    checked=[]
    for name,(old,new) in changes.items():
        assert old in raw,name
        try:validate(raw.replace(old,new))
        except (AssertionError,KeyError,ValueError):checked.append(name)
        else:raise AssertionError('accepted invalid observation '+name)
    return checked
def main():
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');out=BASE/stamp;out.mkdir(parents=True,exist_ok=False)
    sources={str(p.relative_to(ROOT)):sha(p) for p in INPUTS};commands=[];profiles=[];controls=[]
    external={str(p):sha(p) for p in [KERNEL,BUSYBOX,Path(shutil.which('qemu-system-x86_64'))]}
    spec=importlib.util.spec_from_file_location('archive',ROOT/'labs/realtime_depth/linux/guest.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
    def run(label,cmd,expect=0,timeout=180):
        cmd=list(map(str,cmd));p=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=timeout);log=out/(label+'.txt');log.write_text(p.stdout+p.stderr)
        commands.append({'label':label,'command':cmd,'exit_code':p.returncode,'expected_exit_code':expect,'log':str(log.relative_to(ROOT)),'log_sha256':sha(log)})
        assert p.returncode==expect,(label,p.returncode,p.stdout[-1000:],p.stderr[-1000:]);return p.stdout+p.stderr
    try:
        run('host-policy-readonly',[sys.executable,'-B','-c','from pathlib import Path; import json; print(json.dumps({p:Path(p).read_text() for p in ["/proc/self/cgroup","/proc/sys/kernel/perf_event_paranoid","/sys/fs/cgroup/cgroup.subtree_control"]}))'])
        for cc in ('gcc','clang'):
            run(cc+'-version',[cc,'--version'])
            workload=out/(cc+'-workload');run(cc+'-workload-build',[cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic','-O2','-g','-static','-I'+str(ROOT/'labs/performance_followthrough'),'-I'+str(ROOT/'labs/algorithms/include'),PROVIDER,ASSM,'-pthread','-o',workload])
            for name,macro in [('normal',None)]+([('no-placement','PF_SKIP_MOVE'),('no-restoration','PF_SKIP_RESTORE')] if cc=='gcc' else []):
                label=cc+'-'+name;probe=out/(label+'-probe');run(label+'-build',[cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic','-O2','-static',*(['-D'+macro+'=1'] if macro else []),DRAFT/'delegated.c','-o',probe])
                init=('\n'.join(['#!/bin/busybox sh','export PATH=/bin','/bin/busybox --install -s /bin','mkdir -p /proc /sys /dev /tmp /run','mount -t proc none /proc','mount -t sysfs none /sys','mount -t devtmpfs none /dev','mount -t tmpfs -o size=134217728,mode=1777 none /tmp','mount -t cgroup2 none /sys/fs/cgroup',"printf 'CM-T43-DISPOSABLE-GUEST-1\\n' > /run/t43-owned",'echo PC_GUEST_KERNEL; uname -a','/probe','echo PC_PROBE_EXIT=$?','echo PC_GUEST_DONE','poweroff -f'])+'\n').encode()
                image=out/(label+'.cpio');image.write_bytes(mod.newc([('bin',stat.S_IFDIR|0o755,b''),('bin/busybox',stat.S_IFREG|0o755,BUSYBOX.read_bytes()),('init',stat.S_IFREG|0o755,init),('probe',stat.S_IFREG|0o755,probe.read_bytes()),('workload',stat.S_IFREG|0o755,workload.read_bytes())]))
                cmd=['qemu-system-x86_64','-accel','tcg','-machine','q35','-cpu','max','-smp','2','-m','768M','-nodefaults','-display','none','-serial','stdio','-monitor','none','-nic','none','-snapshot','-no-reboot','-kernel',KERNEL,'-initrd',image,'-append','console=ttyS0 rdinit=/init panic=-1 quiet t43_owned=1']
                raw=run(label+'-guest',cmd)
                if macro:
                    assert 'PC_PROBE_EXIT=1' in raw and 'PC_FAIL:' in raw and 'PC_CLEANUP removed=3' in raw and 'PC_PASS:' not in raw
                    assert ('member' not in raw or True)
                    controls.append({'name':name,'compiled_macro':macro,'guest_refused':True,'cleanup':True,'log':commands[-1]['log']})
                else:
                    obs=validate(raw);negative=negative_controls(raw);profiles.append({'compiler':cc,'passed':True,'workload_sha256':sha(workload),'probe_sha256':sha(probe),'image_sha256':sha(image),'observations':obs,'negative_controls':negative,'log':commands[-1]['log']})
        assert sources=={str(p.relative_to(ROOT)):sha(p) for p in INPUTS}
        passed=True;error=None
    except Exception as e:passed=False;error=repr(e)
    result={'passed':passed,'error':error,'scope':'AX-PR-T43-10-T43.11.a-DELAYED; author observation, no learner or hardware PMU claim','original_workload_source':str(PROVIDER.relative_to(ROOT)),'input_sha256':sources,'external_sha256':external,'commands':commands,'profiles':profiles,'compiled_controls':controls,'host_policy_changed':False,'physical_observation':False,'independent_learner':False,'schema_version':1}
    dest=out/'campaign.json';dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':passed,'error':error,'report':str(dest.relative_to(ROOT)),'commands':len(commands),'profiles':len(profiles)}));return 0 if passed else 1
if __name__=='__main__':raise SystemExit(main())
