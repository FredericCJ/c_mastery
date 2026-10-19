"""Read-only exact-rubric and source-bound T73 evidence guards."""
import argparse
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief,validate
from register_messaging_completion import CAMPAIGN,COMPOSITION,MODELS,COROUTINES,COST,READY


def read(path):return json.loads((ROOT/path).read_text(encoding='utf8'))
def sha(path):return hashlib.sha256((ROOT/path).read_bytes()).hexdigest()
def require(test,message):
    if not test:raise ValueError(message)


def receipt(value):
    require(value.get('passed') is True,'unpassed report')
    pins=value.get('inspected_paths',value.get('input_sha256'))
    require(isinstance(pins,dict) and bool(pins),'missing source pins')
    for path,expected in pins.items():require(sha(path)==expected,'stale source '+path)
    records=value.get('executions',value.get('checks',[]));require(bool(records),'missing execution records')
    for r in records:
        require(bool(r.get('command')),'missing exact command')
        if r.get('log_sha256'):require(sha(r['log'])==r['log_sha256'],'stale execution log')


def family_report(value,name,extra):
    receipt(value);family=read(f'labs/{name}/family.json');anchors=set(family['tests'])
    expected=set(family['campaign']['profiles']);actual={r['compiler']+'-'+r['profile'] for r in value['profiles']}
    require(expected==actual and len(actual)==len(value['profiles']),'missing campaign profile')
    for p in value['profiles']:
        require(p['passed'] and set(p['anchors'])==anchors,'missing passed anchor')
        require(p['ctest_count']==3*len(anchors)+extra,'test count mismatch')
        for anchor in anchors:
            controls=p['controls_by_anchor'].get(anchor,[])
            require(len(controls)==2 and len(set(controls))==2,'missing semantic controls')
            require(all(c.startswith(anchor.replace(name+'.',name+'.mutation.',1)+'.') for c in controls),'mismatched control')
        if p['profile']=='tsan':require(p['thread_instrumentation_confirmed'] is True,'unconfirmed TSan instrumentation')
        if name=='modern_composition':
            require(PUBLICATION_CONTROLS <= set(p['compiled_mutant_controls']), 'missing supplemental causal control')
    rejects=value['learner_stub_rejections']
    require(len(rejects)==len(anchors) and {r['test'] for r in rejects if r['rejected']}==anchors,'missing intended learner rejection')
    logs=[]
    for r in value['executions']:
        command=r['command']
        if command[0]=='cmake':require(r['exit_code']==0,'failed configure/build')
        if command[0]=='ctest' and '-R' not in command and '--output-on-failure' in command:
            require(r['exit_code']==0,'failed CTest run');logs.append((ROOT/r['log']).read_text(encoding='utf8'))
    require(len(logs)==len(actual),'missing actual test run')
    for log in logs:
        for anchor in anchors:
            require(re.search(re.escape(anchor)+r'\s+\.+\s+Passed',log) is not None,'missing actual anchor pass')
            matches=re.findall(re.escape(anchor.replace(name+'.',name+'.mutation.',1))+r'\.\d+\s+\.+\s+Passed',log)
            require(len(matches)==2,'missing actual control rejection')
        if name=='modern_composition':
            for test in PUBLICATION_TESTS | PUBLICATION_CONTROLS:
                require(re.search(re.escape(test)+r'\s+\.+\s+Passed',log) is not None,
                        'missing observed supplemental publication test: '+test)
    return len(anchors),len(actual)


PUBLICATION_TESTS={'modern_composition.publication_'+s for s in
                   ('admission','prefix','close','recovery','limits','original')}
PUBLICATION_CONTROLS={'modern_composition.mutation.publication_'+s for s in
                      ('prefix.1','prefix.2','close.5','close.6')}


def inspect(data,campaign,composition,models,cost,costrun,build,coroutines):
    anchors,profiles=family_report(campaign,'messaging',2)
    family_report(composition,'modern_composition',len(PUBLICATION_TESTS | PUBLICATION_CONTROLS))
    from check_ownership_publication import check as inspect_publication
    inspect_publication(read('research/v5/modern/ownership_publication_validation.json'))
    receipt(models);require(models['tool']['model']=='RC11','wrong model')
    require(len(models['executions'])==26 and all(r['passed'] for r in models['executions']),'model execution gap')
    require(sum(r['expected']=='safety-counterexample' for r in models['executions'])==13,'missing counterexamples')
    receipt(coroutines)
    checks=coroutines['checks'];channels=[r for r in checks if 'channel' in r['label']]
    require(len(channels)==52 and all(r['passed'] for r in channels),'missing coroutine channel evidence')
    require(sum('channel execute 0' in r['label'] and r['returncode']==0 for r in channels)==6,'missing channel positives')
    require(sum('channel execute stub' in r['label'] and r['returncode']==1 and 'CC_CHECK:' in r['stdout'] for r in channels)==6,'missing channel stubs')
    require(sum('channel TSan execute' in r['label'] and r['returncode']==0 for r in channels)==2,'missing channel TSan')
    receipt(costrun);receipt(build)
    for p,h in cost['input_sha256'].items():require(sha(p)==h,'stale cost analysis input')
    require(len(cost['rows'])==10 and {r['objective'] for r in cost['rows']}=={f'T73.{i:02d}.d' for i in range(1,11)},'missing cost objective')
    require(all(len(r['dimensions'])==13 for r in cost['rows']),'missing cost dimension')
    require({r['profile'] for r in build['profiles']}=={'gccO0','gccO3','clangO0','clangO3'},'missing build cost configuration')
    require(all(len(r['changed_objects'])==2 for r in build['profiles']),'incremental dependency mismatch')
    spec=importlib.util.spec_from_file_location('messaging_cost_check',ROOT/'labs/messaging/followthrough/cost_check.py')
    validator=importlib.util.module_from_spec(spec);spec.loader.exec_module(validator)
    observed={}
    for row in costrun['observations']:
        require(row['mode'] in validator.MODES,'unknown measured workload')
        raw={k:v for k,v in row.items() if k not in ('profile','repetition','warmup','thread_latency_summary_ns')}
        try:validator.validate(raw,costrun['workload']['operations'])
        except (AssertionError,KeyError,TypeError) as e:raise ValueError('unequal cost workload') from e
        if not row['warmup']:observed.setdefault((row['profile'],row['mode']),[]).append(row)
    for profile in ('gccO0','gccO3','clangO0','clangO3'):
        for mode in validator.MODES:require(len(observed.get((profile,mode),[]))==(1 if mode in ('representation','fairness') else 7),'missing measured repetition')
    criteria={};roots={}
    for path in sorted((ROOT/'curriculum/v5/exercises').glob('X-T73-*.md')):
        meta,body=parse_brief(path);validate(meta,body);roots[meta['id']]={r['id'] for r in meta['rubric']}
        for row in meta['rubric']:criteria[row['id']]=(row['objective'],row['criterion'])
    require(len(data['criteria'])==len(criteria) and {r['id'] for r in data['criteria']}==set(criteria),'missing exact criterion')
    for row in data['criteria']:
        require((row['objective'],row['criterion'])==criteria[row['id']],'criterion changed')
        require(row['author_readiness']=='ready-bounded' and bool(row['demonstration']),'unjustified readiness')
        require(row['external_review']=='not-run' and row['independent_learner']=='not-assessed','unsupported independence claim')
        require(bool(row['limits']),'missing limits')
        require(set(row['pins'])==set(row['teaching']+row['implementation_and_observation']),'missing rubric evidence pin')
        for p,h in row['pins'].items():require(sha(p)==h,'stale rubric evidence '+p)
    require({r['id'] for r in data['roots']}==set(roots),'missing exercise root')
    for row in data['roots']:require(set(row['criteria'])==roots[row['id']] and row['author_readiness']=='ready-bounded','root overclaim')
    for gate in ('messaging','messaging_model','messaging_versions','messaging_cost'):
        d=read(f'research/v5/modern/{gate}_research_gate.json');require(d['status']=='passage-inspected','uninspected sources')
        for p,h in d['research_input_sha256'].items():require(sha(p)==h,'stale source gate')
    return dict(criteria=len(criteria),exercises=len(roots),anchors=anchors,profiles=profiles,
                tests_per_profile=3*anchors+2,model_cases=26,cost_modes=len(validator.MODES),cost_dimensions=130)


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--self-test',action='store_true');args=parser.parse_args()
    values=[read(p) for p in (READY,CAMPAIGN,COMPOSITION,MODELS,COST,'research/v5/modern/messaging_cost_validation.json','research/v5/modern/messaging_build_cost_validation.json',COROUTINES)]
    result=inspect(*values);rejected=0
    if args.self_test:
        for case in range(12):
            v=copy.deepcopy(values)
            if case==0:v[1]['passed']=False
            elif case==1:v[1]['profiles'].pop()
            elif case==2:v[1]['profiles'][0]['controls_by_anchor']['messaging.completion_winner']=[]
            elif case==3:v[1]['learner_stub_rejections'].pop()
            elif case==4:v[0]['criteria'][0]['independent_learner']='passed'
            elif case==5:v[0]['criteria'][0]['criterion']+=' invented'
            elif case==6:v[3]['executions'].pop()
            elif case==7:v[4]['rows'][0]['dimensions'].pop('lifetime')
            elif case==8:v[5]['observations'][0]['mode']='missing-mode'
            elif case==9:v[1]['inspected_paths'][next(iter(v[1]['inspected_paths']))]='0'*64
            elif case==10:v[2]['profiles'][0]['compiled_mutant_controls'].remove('modern_composition.mutation.publication_close.6')
            else:v[2]['profiles'][0]['ctest_count']-=1
            try:inspect(*v)
            except ValueError:rejected+=1
            else:raise ValueError('negative control accepted '+str(case))
    print(json.dumps(dict(passed=True,**result,negative_controls_rejected=rejected)))


if __name__=='__main__':main()
