"""Post-run strict check of the scoped T43 successor; never rewrites a receipt."""
import argparse, copy, hashlib, importlib.util, json, sys
from pathlib import Path
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('producer',Path(__file__).with_name('run_performance_delegated.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
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
    assert all(r['exit_code']==r['expected_exit_code']==(1 if r['label']=='learner-build-run' else 0) for r in commands.values())
    assert [r['name'] for r in d['compiled_controls']]==['no-placement','no-restoration']
    assert [r['compiled_macro'] for r in d['compiled_controls']]==['PF_SKIP_MOVE','PF_SKIP_RESTORE']
    for r in d['compiled_controls']:
        assert r['guest_refused'] is True and r['cleanup'] is True
        assert r['log']==commands['gcc-'+r['name']+'-guest']['log']
    if files:
        for p,h in d['input_sha256'].items():assert m.sha(ROOT/p)==h,p
        for p,h in d['external_sha256'].items():assert m.sha(Path(p))==h,p
        assert set(d['external_sha256'])=={str(m.KERNEL),str(m.BUSYBOX),'/usr/bin/qemu-system-x86_64'}
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
            assert guest[guest.index('-kernel')+1]==str(m.KERNEL) and guest[guest.index('-nic')+1]=='none' and '-drive' not in guest
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
    }
    for name,mutate in mutations.items():
        bad=copy.deepcopy(d);mutate(bad)
        try:check(bad)
        except (AssertionError,KeyError,ValueError):tests.append({'name':name,'passed':True})
        else:raise AssertionError('negative control accepted: '+name)
    out=a.output.resolve();assert out.is_relative_to(m.BASE.resolve()) and not out.exists()
    out.write_text(json.dumps({'passed':True,'report':str(a.report.relative_to(ROOT)) if a.report.is_absolute() else str(a.report),'report_sha256':m.sha(a.report),'checker_sha256':m.sha(Path(__file__)),'controls':tests,'scope':d['scope'],'new_c_execution':True,'canonical_integration':'separate exact-criterion attachment'},indent=2)+'\n');print(json.dumps({'passed':True,'controls':len(tests),'output':str(out)}))
if __name__=='__main__':main()
