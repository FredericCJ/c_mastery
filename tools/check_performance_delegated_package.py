"""Exact original delayed-row, learner-export and source-authority guard."""
import argparse,copy,hashlib,importlib.util,json,re,subprocess,sys
from pathlib import Path
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1];BASE=ROOT/'research/v5/expansion/performance_delegated';SUPPORT=BASE/'completion_dispositions.json'
from check_performance_delegated import check as campaign_check
import run_performance_delegated as campaign
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads((ROOT/p).read_text())

def export_commands(exports):
    work=Path(exports['workspace']);out=work.parent
    assert work.name=='learner' and out.parent==campaign.BASE.relative_to(ROOT) and re.fullmatch(r'exports_[0-9]{8}T[0-9]{12}Z',out.name)
    args=['/usr/bin/python3','-B',str(ROOT/work/'labs/performance_delegated/build.py'),'--cc','gcc','--run','--kernel',str(campaign.KERNEL),'--busybox',str(campaign.BUSYBOX)]
    expected=[['/usr/bin/python3','-B','-m','cmastery','init','PR-T43-10',str(ROOT/work)],[*args,'--work',str(ROOT/out/'learner-build')],[*args,'--work',str(ROOT/out/'reference-build'),'--source',str(ROOT/'assessor/performance_delegated/delegated.c')]]
    assert [r['label'] for r in exports['commands']]==['init','learner','explicit-reference']
    for r,cmd in zip(exports['commands'],expected):
        assert r['command']==cmd
        assert r['log']==(out/(r['label']+'.txt')).as_posix()
    return True
def validate(d):
    assert d['schema_version']==1 and d['required'] is True and d['new_c_execution'] is True
    assert len(d['updates'])==1;row=d['updates'][0];assert row['id']=='AX-PR-T43-10-T43.11.a-DELAYED'
    card=read('curriculum/v5/expansion/legacy_assessments/PR-T43-10.json');criterion=next(c for c in card['criteria'] if c['id']=='AX-PR-T43-10-T43.11.a');variant=next(v for v in criterion['variants'] if v['id']==row['id'])
    assert d['original_criterion']==criterion and d['original_variant']==variant and row['original_variant_task']==variant['task'] and row['minimum_days']==variant['minimum_days']==7
    baseline=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:curriculum/v5/expansion/legacy_assessments/PR-T43-10.json'],cwd=ROOT,text=True));oldcriterion=next(c for c in baseline['criteria'] if c['id']==criterion['id']);pure=copy.deepcopy(criterion);pure.pop('successor_support',None);assert pure==oldcriterion
    old=read(d['predecessor_support']);prior=next(r for r in old['updates'] if r['id']==row['id']);assert prior==d['predecessor_row']
    for dim in ('external_review','independent_learner','physical_observation'):assert row['dimensions'][dim]==prior['dimensions'][dim]
    assert set(row['dimensions'])=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'}
    assert row['dimensions']['implementation']['state']=='complete-original-workload-prepared-unprivileged-guest-adapter'
    assert len(d['current_reports'])==3
    actual,exports,guard=map(read,d['current_reports']);campaign_check(actual);assert guard['passed'] and guard['report_sha256']==sha(ROOT/d['current_reports'][0]);assert len(guard['controls'])==44 and all(r['passed'] for r in guard['controls']);assert guard['checker_sha256']==sha(ROOT/'tools/check_performance_delegated.py')
    assert exports['passed'] is True and exports['project']=='PR-T43-10' and [r['label'] for r in exports['commands']]==['init','learner','explicit-reference']
    export_commands(exports)
    assert [r['returncode'] for r in exports['commands']]==[0,1,0]
    for r in exports['commands']:assert r['returncode']==r['expected_exit_code'] and sha(ROOT/r['log'])==r['log_sha256']
    assert 'PC_TODO:' in (ROOT/exports['commands'][1]['log']).read_text() and 'PC_PASS:T43.11.a-DELAYED:' in (ROOT/exports['commands'][2]['log']).read_text()
    campaign.validate((ROOT/exports['commands'][2]['log']).read_text())
    for name,digest in exports['exported_files'].items():assert 'assessor' not in Path(name).parts and sha(ROOT/exports['workspace']/name)==digest
    for name,digest in exports['source_sha256'].items():assert sha(ROOT/name)==digest
    for name,digest in d['source_sha256'].items():assert sha(ROOT/name)==digest,name
    resource=read('research/v5/expansion/performance_delegated/resources.json')['resources'];assert len(resource)==1 and resource[0]['id']=='R-PERF-DELEGATED-CGROUP';r=resource[0]
    assert r['assignable'] is True and '7.0.14' in r['edition'] and all(r['inspection'][k] for k in ('date','method','scope','limits'))
    assert sha(ROOT/r['url'])==r['inspection']['local_sha256']=='e42ff6aaf6277c94e12e0a1a2fd5b8a97ad8fa408d137a84e81fd3d00c616acd'
    gate=read('research/v5/expansion/performance_delegated/research_gate.json');assert gate['status']=='passage-inspected'
    for name,digest in gate['research_input_sha256'].items():assert sha(ROOT/name)==digest
    assert any(s['path']=='research/v5/expansion/performance_delegated/completion_dispositions.json' and s['criterion_ids']==[row['id']] for s in card['additional_successor_support'])
    return True
def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',type=Path,required=True);a=p.parse_args();d=json.loads(SUPPORT.read_text());validate(d);controls=[]
    if a.self_test:
        mutations={'missing-row':lambda x:x['updates'].clear(),'wrong-id':lambda x:x['updates'][0].update(id='AX-PR-T43-10-T43.11.a'),'changed-task':lambda x:x['updates'][0].update(original_variant_task='Observe any unrelated workload'),'changed-delay':lambda x:x['updates'][0].update(minimum_days=0),'wrong-original-variant':lambda x:x['original_variant'].update(task='Different task'),'review-promotion':lambda x:x['updates'][0]['dimensions']['external_review'].update(state='passed'),'learner-promotion':lambda x:x['updates'][0]['dimensions']['independent_learner'].update(state='passed'),'physical-promotion':lambda x:x['updates'][0]['dimensions']['physical_observation'].update(state='passed'),'missing-report':lambda x:x['current_reports'].pop(),'stale-source':lambda x:x['source_sha256'].update({next(iter(x['source_sha256'])):'0'*64})}
        for name,mutate in mutations.items():
            bad=copy.deepcopy(d);mutate(bad)
            try:validate(bad)
            except (AssertionError,KeyError,ValueError,IndexError):controls.append({'name':name,'passed':True})
            else:raise AssertionError('Accepted invalid package '+name)
        original=read(d['current_reports'][1])
        changes={'export-wrong-project':lambda x:x['commands'][0]['command'].__setitem__(5,'PR-T49-09'),'export-learner-answer':lambda x:x['commands'][1]['command'].extend(['--source',str(ROOT/'assessor/performance_delegated/delegated.c')]),'export-reference-other-source':lambda x:x['commands'][2]['command'].__setitem__(-1,'/tmp/unrelated.c'),'export-extra-option':lambda x:x['commands'][2]['command'].extend(['--extra','value']),'export-unscoped-workspace':lambda x:x.update(workspace='build/unrelated/learner')}
        for name,mutate in changes.items():
            bad=copy.deepcopy(original);mutate(bad)
            try:export_commands(bad)
            except (AssertionError,KeyError,ValueError,IndexError):controls.append({'name':name,'passed':True})
            else:raise AssertionError('Accepted invalid export '+name)
    out=a.output.resolve();allowed=ROOT/'reviews/v5/expansion/integrated/final_author_run/t43_cgroup';assert out.is_relative_to(allowed) and not out.exists();out.write_text(json.dumps({'passed':True,'support':str(SUPPORT.relative_to(ROOT)),'support_sha256':sha(SUPPORT),'checker_sha256':sha(Path(__file__)),'controls':controls,'scope':'Exact original delayed author observation; historical, independent and physical dimensions preserved'},indent=2)+'\n');print(json.dumps({'passed':True,'controls':len(controls),'output':str(out)}))
if __name__=='__main__':main()
