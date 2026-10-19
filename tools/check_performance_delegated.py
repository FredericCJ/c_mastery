"""Post-run strict check of the scoped T43 successor; never rewrites a receipt."""
import argparse, copy, hashlib, importlib.util, json, re, sys
from pathlib import Path
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('producer',Path(__file__).with_name('run_performance_delegated.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)

def expected_commands(out):
    """Closed command contract; output names derive only from the scoped log root."""
    result={'host-policy-readonly':['/usr/bin/python3','-B','-c','from pathlib import Path; import json; print(json.dumps({p:Path(p).read_text() for p in ["/proc/self/cgroup","/proc/sys/kernel/perf_event_paranoid","/sys/fs/cgroup/cgroup.subtree_control"]}))']}
    for cc in ('gcc','clang'):
        result[cc+'-version']=[cc,'--version']
        result[cc+'-workload-build']=[cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic','-O2','-g','-static','-I'+str(ROOT/'labs/performance_followthrough'),'-I'+str(ROOT/'labs/algorithms/include'),str(m.PROVIDER),str(m.ASSM),'-pthread','-o',str(out/(cc+'-workload'))]
        for name,macro in [('normal',None)]+([('no-placement','PF_SKIP_MOVE'),('no-restoration','PF_SKIP_RESTORE')] if cc=='gcc' else []):
            label=cc+'-'+name
            result[label+'-build']=[cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic','-O2','-static',*(['-D'+macro+'=1'] if macro else []),str(m.REF/'delegated.c'),'-o',str(out/(label+'-probe'))]
            result[label+'-guest']=['qemu-system-x86_64','-accel','tcg','-machine','q35','-cpu','max','-smp','2','-m','768M','-nodefaults','-display','none','-serial','stdio','-monitor','none','-nic','none','-snapshot','-no-reboot','-kernel',str(m.KERNEL),'-initrd',str(out/(label+'.cpio')),'-append','console=ttyS0 rdinit=/init panic=-1 quiet t43_owned=1']
    result['learner-build-run']=['/usr/bin/python3','-B',str(m.LAB/'build.py'),'--work',str(out/'learner'),'--cc','gcc','--run','--kernel',str(m.KERNEL),'--busybox',str(m.BUSYBOX)]
    return result
def check(d,files=True):
    assert d['passed'] is True and d['error'] is None
    assert d['scope']=='AX-PR-T43-10-T43.11.a-DELAYED; author observation, no learner or hardware PMU claim'
    assert d['original_workload_source']=='labs/performance_followthrough/observations.c'
    assert all(d[k] is False for k in ('host_policy_changed','physical_observation','independent_learner'))
    assert set(d['input_sha256'])=={str(p.relative_to(ROOT)) for p in m.INPUTS}
    assert len(d['profiles'])==2 and [p['compiler'] for p in d['profiles']]==['gcc','clang']
    labels=['host-policy-readonly','gcc-version','gcc-workload-build','gcc-normal-build','gcc-normal-guest','gcc-no-placement-build','gcc-no-placement-guest','gcc-no-restoration-build','gcc-no-restoration-guest','clang-version','clang-workload-build','clang-normal-build','clang-normal-guest','learner-build-run']
    assert [r['label'] for r in d['commands']]==labels
    commands={r['label']:r for r in d['commands']}
    relative=Path(commands['host-policy-readonly']['log']).parent
    assert relative.parent==m.BASE.relative_to(ROOT) and re.fullmatch(r'[0-9]{8}T[0-9]{12}Z',relative.name)
    expected=expected_commands(ROOT/relative)
    assert set(expected)==set(commands)
    for label,r in commands.items():
        assert r['log']==(relative/(label+'.txt')).as_posix()
        assert r['command']==expected[label],label
    assert all(r['exit_code']==r['expected_exit_code']==(1 if r['label']=='learner-build-run' else 0) for r in commands.values())
    assert [r['name'] for r in d['compiled_controls']]==['no-placement','no-restoration']
    assert [r['compiled_macro'] for r in d['compiled_controls']]==['PF_SKIP_MOVE','PF_SKIP_RESTORE']
    for r in d['compiled_controls']:
        assert r['guest_refused'] is True and r['cleanup'] is True
        assert r['log']==commands['gcc-'+r['name']+'-guest']['log']
    if files:
        for p,h in d['input_sha256'].items():assert m.sha(ROOT/p)==h,p
        for p,h in d['external_sha256'].items():assert m.sha(Path(p))==h,p
        assert set(d['external_sha256'])=={str(m.KERNEL),str(m.KERNEL.parents[3]/'.config'),str(m.BUSYBOX),'/usr/bin/qemu-system-x86_64'}
        for r in commands.values():assert m.sha(ROOT/r['log'])==r['log_sha256'],r['log']
    for profile in d['profiles']:
        cc=profile['compiler'];assert profile['passed'] is True
        assert profile['log']==commands[cc+'-normal-guest']['log']
        assert set(profile['observations'])=={'unlimited','quota'}
        for name,obs in profile['observations'].items():
            assert len(obs['workload'])==10 and obs['cpu_delta']['usage_usec']>0
            if name=='quota':assert obs['cpu_delta']['nr_throttled']>0 and obs['cpu_delta']['throttled_usec']>0
            assert obs['psi']['value'] is None and obs['psi']['errno']==2
        if files:
            raw=(ROOT/profile['log']).read_text();assert m.validate(raw)==profile['observations'];assert m.negative_controls(raw)==profile['negative_controls']
            workload=Path(commands[cc+'-workload-build']['command'][-1]);probe=Path(commands[cc+'-normal-build']['command'][-1]);guest=commands[cc+'-normal-guest']['command'];image=Path(guest[guest.index('-initrd')+1])
            assert m.sha(workload)==profile['workload_sha256'] and m.sha(probe)==profile['probe_sha256'] and m.sha(image)==profile['image_sha256']
    if files:
        refused=(ROOT/commands['learner-build-run']['log']).read_text();assert 'PC_TODO:prepared-delegation-and-exact-workload-observation' in refused and 'PC_PROBE_EXIT=1' in refused and 'PC_PASS:' not in refused
        for r in d['compiled_controls']:
            raw=(ROOT/r['log']).read_text();assert 'PC_PROBE_EXIT=1' in raw and 'PC_FAIL:' in raw and 'PC_CLEANUP removed=3' in raw and 'PC_PASS:' not in raw
            assert ('!strcmp(s,expected)' if r['name']=='no-placement' else '!strcmp(before,after)') in raw
    return True
def main():
    p=argparse.ArgumentParser();p.add_argument('--report',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();d=json.loads(a.report.read_text());check(d);tests=[]
    mutations={
      'missing-profile':lambda x:x['profiles'].pop(),
      'missing-command':lambda x:x['commands'].pop(),
      'missing-placement-control':lambda x:x['compiled_controls'].pop(0),
      'wrong-workload-source':lambda x:x.update(original_workload_source='labs/os_followthrough/guest.c'),
      'zero-throttling':lambda x:x['profiles'][0]['observations']['quota']['cpu_delta'].update(nr_throttled=0),
      'psi-zero':lambda x:x['profiles'][0]['observations']['quota']['psi'].update(value=0),
      'physical-promotion':lambda x:x.update(physical_observation=True),
      'learner-promotion':lambda x:x.update(independent_learner=True),
      'policy-change':lambda x:x.update(host_policy_changed=True),
      'wrong-source-hash':lambda x:x['input_sha256'].update({next(iter(x['input_sha256'])):'0'*64}),
      'wrong-log-hash':lambda x:x['commands'][0].update(log_sha256='0'*64),
      'wrong-workload-image':lambda x:x['profiles'][0].update(workload_sha256='0'*64),
      'wrong-probe-source':lambda x:x['commands'][3]['command'].__setitem__(-3,'/tmp/unrelated.c'),
      'wrong-workload-provider':lambda x:x['commands'][2]['command'].__setitem__(-5,'/tmp/unrelated.S'),
      'missing-c17':lambda x:x['commands'][3]['command'].remove('-std=c17'),
      'missing-mutant-macro':lambda x:x['commands'][5]['command'].remove('-DPF_SKIP_MOVE=1'),
      'changed-output':lambda x:x['commands'][3]['command'].__setitem__(-1,'/tmp/unrelated-probe'),
      'host-filesystem-device':lambda x:x['commands'][4]['command'].extend(['-virtfs','local,path=/,mount_tag=host,security_model=none']),
      'fsdev-extra':lambda x:x['commands'][4]['command'].extend(['-fsdev','local,id=x,path=/,security_model=none']),
      'drive-extra':lambda x:x['commands'][4]['command'].extend(['-drive','file=/tmp/unrelated']),
      'wrong-kernel':lambda x:x['commands'][4]['command'].__setitem__(-5,'/tmp/unrelated-kernel'),
      'wrong-initrd':lambda x:x['commands'][4]['command'].__setitem__(-3,'/tmp/unrelated.cpio'),
      'learner-source-injection':lambda x:x['commands'][-1]['command'].extend(['--source',str(m.REF/'delegated.c')]),
      'unrelated-log-root':lambda x:x['commands'][0].update(log='reviews/unrelated/host-policy-readonly.txt'),
    }
    for name,mutate in mutations.items():
        bad=copy.deepcopy(d);mutate(bad)
        try:check(bad)
        except (AssertionError,KeyError,ValueError):tests.append({'name':name,'passed':True})
        else:raise AssertionError('negative control accepted: '+name)
    raw=(ROOT/d['profiles'][0]['log']).read_text()
    for name in m.negative_controls(raw):tests.append({'name':'raw-'+name,'passed':True})
    mixed=re.sub(r'(PC_LIVE kind=(?:processes|threads) ids=[^\r\n]*)',r'\1 0',raw)
    observed=m.validate(mixed)
    assert all(observed[n]['maximum_observed_positive_ids']==d['profiles'][0]['observations'][n]['maximum_observed_positive_ids'] for n in observed)
    tests.append({'name':'raw-zero-preserved-not-counted-positive','passed':True})
    out=a.output.resolve();assert out.is_relative_to(m.BASE.resolve()) and not out.exists()
    out.write_text(json.dumps({'passed':True,'report':str(a.report.relative_to(ROOT)) if a.report.is_absolute() else str(a.report),'report_sha256':m.sha(a.report),'checker_sha256':m.sha(Path(__file__)),'controls':tests,'scope':d['scope'],'new_c_execution':True,'canonical_integration':'separate exact-criterion attachment'},indent=2)+'\n');print(json.dumps({'passed':True,'controls':len(tests),'output':str(out)}))
if __name__=='__main__':main()
