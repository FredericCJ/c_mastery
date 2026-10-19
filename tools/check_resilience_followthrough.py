"""Guard current T75 successor evidence; never infers whole-domain mastery."""
import argparse
import copy
import hashlib
import json
import re
import tempfile
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
CAMPAIGN='reviews/v5/expansion/modern_completion/resilience_transport_campaign.json'
COST='research/v5/modern/resilience_timer_cost_validation.json'
SERVICE_COST='research/v5/modern/resilience_service_cost_validation.json'
EXPECTED_MODELS={'budget_oracle':16524,'fractional_oracle':51188,'scope_oracle':1364,
                 'deadline_oracle':44220,'migration_oracle':864,'pair_oracle':5824}

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def pins(values):
    assert isinstance(values,dict) and values,'missing source pins'
    for path,expected in values.items():
        absolute=(ROOT/path).resolve()
        assert absolute.is_relative_to(ROOT.resolve()) and absolute.is_file(),path
        assert sha(absolute)==expected,'stale source '+path

def logs(records):
    for row in records:
        if 'log' in row:
            path=(ROOT/row['log']).resolve()
            assert path.is_relative_to(ROOT.resolve()) and path.is_file(),row
            assert sha(path)==row['log_sha256'],'stale execution log'
        assert 'error' not in row,'recorded failure'

def check_campaign(data,check_files=True):
    assert data.get('passed') and data.get('status')=='passed' and data.get('scope_source_stable'),'failed/unstable campaign'
    family=json.loads((ROOT/'labs/resilience/family.json').read_text(encoding='utf8'))
    anchors=set(family['tests']);wanted=set(family['campaign']['profiles'])
    actual={p['compiler']+'-'+p['profile'] for p in data['profiles']}
    assert actual==wanted and len(data['profiles'])==len(wanted),'missing or repeated profile'
    closures=data['compiler_dependency_closures']
    assert {c['configuration'] for c in closures}==wanted|{'learner'} and len(closures)==len(wanted)+1,'missing actual compiler closure'
    for closure in closures:
        assert closure['repository_inputs'] and closure['sources'] and closure['external_inputs'],'empty compiler dependency evidence'
        assert set(closure['sources'])<=set(closure['repository_inputs'])<=set(data['inspected_paths']),'unpinned compiler input'
        for generated in closure['generated_link_inputs'].values():
            assert generated['producer_source'] in closure['sources'] and generated['recipe'] and len(generated['sha256'])==64,'unbound generated link input'
    assert data['external_inputs_stable'] and data['external_input_sha256'],'unbound/unstable toolchain dependencies'
    assert not(set(data['excluded_uncompiled_changes']) & set(data['inspected_paths'])),'ignored actual input change'
    if check_files:
        for row in data['generated_artifacts']:assert sha(ROOT/row['path'])==row['sha256'],'changed generated compilation evidence'
    for profile in data['profiles']:
        assert profile['passed'] and set(profile['anchors'])==anchors,'profile failed/anchor absent'
        assert profile['ctest_count']>=3*len(anchors),'unexecuted controls'
        for anchor in anchors:
            control_count={'resilience.pressure_owned':4,'resilience.breaker_window':3,'resilience.hedge_owned':3,'resilience.receiver_journal':3,'resilience.service_outbox':3}.get(anchor,2)
            expected={anchor.replace('resilience.','resilience.mutation.',1)+'.'+str(i) for i in range(1,control_count+1)}
            controls=profile['controls_by_anchor'][anchor]
            assert len(controls)==len(set(controls)) and expected<=set(controls),'missing or repeated semantic controls'
            assert expected<=set(profile['compiled_mutant_controls']),'not compiled'
        records=[r for r in data['executions'] if r.get('log','').endswith('-'+profile['compiler']+'-'+profile['profile']+'-ctest.log')]
        assert len(records)==1 and records[0]['exit_code']==0,'missing positive CTest execution'
        output=(ROOT/records[0]['log']).read_text(encoding='utf8')
        for name in anchors|set(profile['compiled_mutant_controls']):
            assert re.search(r'Test\s+#\d+:\s+'+re.escape(name)+r'\s+\.+\s+Passed',output),'missing named test pass '+name
        if profile['profile']=='tsan':assert profile['thread_instrumentation_confirmed'] is True,'uninstrumented TSan'
    rejections=data['learner_stub_rejections']
    assert len(rejections)==len(anchors) and {r['test'] for r in rejections}==anchors and all(r['rejected'] for r in rejections),'not all compiling stubs rejected'
    oracle_rows=[r for r in data['executions'] if r.get('log','').endswith('-independent-oracle.log')]
    assert len(oracle_rows)==len(wanted),'missing per-profile C oracle execution'
    for row in oracle_rows:
        assert row['exit_code']==0,'oracle failed'
        observed=json.loads((ROOT/row['log']).read_text(encoding='utf8').strip())
        assert all(observed.get(key)==value for key,value in EXPECTED_MODELS.items()),'different model scope'
        assert observed['timers']['histories']==58821 and observed['timers']['pairs']==13056,'timer oracle scope'
        assert {key:observed['retry'][key] for key in ['random','plan','history','population']}=={'random':312,'plan':6000,'history':10080,'population':60},'retry oracle scope'
        assert observed['pressure']['policy_histories']==46650 and observed['pressure']['pump_histories']==74904,'pressure oracle scope'
        assert len(observed['pressure']['policy_workloads'])==5,'missing policy comparison'
        assert observed['breaker']['structured_histories']==162908 and observed['breaker']['mixed_tenant_histories']==5000,'breaker oracle scope'
        assert observed['hedge']['histories']==7388 and observed['hedge']['population_requests']==16384,'hedge oracle scope'
        assert len(observed['hedge']['population_comparison'])==4,'missing hedge/work comparison'
        assert observed['journal']['request_and_wire_cases']==2916 and observed['journal']['durability_cuts']==414,'journal oracle scope'
        assert observed['outbox']['structured_histories']==74896 and observed['outbox']['mixed_histories']==5000,'durable service history scope'
        assert observed['service_io']['wire_cases']==11714 and observed['service_io']['structured_cases']==6714,'framed service wire scope'
        assert observed['composition']=={'structured_histories':4096,'mixed_histories':5000,'composition_cases':9096},'transport composition scope'
    if check_files:pins(data['inspected_paths']);logs(data['executions'])

def check_cost(data,check_files=True):
    assert data.get('passed') and data.get('status')=='passed' and data.get('source_stable'),'failed/unstable cost run'
    assert {p['configuration'] for p in data['profiles']}=={'gcc-O0','gcc-O3','clang-O0','clang-O3'} and len(data['profiles'])==4,'missing cost profile'
    for profile in data['profiles']:
        assert len(profile['modes'])==15 and {(m['workload'],m['kind']) for m in profile['modes']}=={(w,k) for w in range(5) for k in range(3)},'missing equivalent mode'
        for mode in profile['modes']:
            assert mode['repetitions']==7 and mode['warmups']==1 and len(mode['nanoseconds'])==7,'missing repeated timing'
            assert all(type(n) is int and n>0 for n in mode['nanoseconds']),'invalid timing'
            assert mode['latency']['samples']==8192,'missing per-call observations'
            assert mode['counters']['sizeof']==profile['sizeof_runtime_selected_object'],'representation mismatch'
            if mode['kind']==2 and mode['workload'] in [1,4]:
                assert mode['counters']['comparisons']==(15 if mode['workload']==1 else 120),'coarse/fine boundary lost'
    assert data['physical_validation']=='not-run' and data['learner_mastery']=='not-assessed','overclaimed evidence'
    if check_files:pins(data['input_sha256']);logs(data['executions'])

def check_service_cost(data,check_files=True):
    assert data.get('passed') and data.get('status')=='passed' and data.get('source_stable') and data.get('external_inputs_stable'),'failed/unstable service costs'
    assert {p['configuration'] for p in data['profiles']}=={'gcc-O0','gcc-O3','clang-O0','clang-O3'} and len(data['profiles'])==4,'missing service cost profile'
    closures=data['compiler_dependency_closures']
    assert len(closures)==8 and {c['configuration'] for c in closures}=={p+'-'+variant for p in ['gcc-O0','gcc-O3','clang-O0','clang-O3'] for variant in ['counter','timing']},'missing cost compiler closure'
    for closure in closures:
        assert closure['repository_inputs'] and set(closure['repository_inputs'])<=set(data['input_sha256']),'unbound service cost input'
        assert closure['external_inputs'] and set(closure['external_inputs'])<=set(data['external_input_sha256']),'unbound cost environment'
    for profile in data['profiles']:
        assert len(profile['modes'])==10 and {m['mode'] for m in profile['modes']}==set(range(10)),'missing cost workload'
        assert profile['layout']['scope']>0 and profile['layout']['scope_align']>0,'missing actual layout'
        for variant in ['counter','timing']:
            assert set(profile['builds'][variant])=={'clean','unchanged','single_unit_rebuild','pressure_clean'},'missing build observation'
        assert profile['layout']['pressure_pump']>profile['layout']['owned_queue'] and set(profile['pressure'])=={'map','binary','assembly','undefined_symbols'},'missing owned pressure cost evidence'
        assert set(profile['mechanism_objects'])=={'scope','fractional','timers','retry','pressure','breaker','hedge','journal','outbox','service_io','composition'},'missing actual mechanism object audits'
        for obj in profile['mechanism_objects'].values():assert obj['source'] in data['input_sha256'] and len(obj['object_sha256'])==64,'unbound audited object'
        for mode in profile['modes']:
            assert mode['repetitions']==7 and mode['warmups']==1 and mode['requests_per_sample']==100 and len(mode['nanoseconds'])==7,'missing repeated service measurement'
            assert all(type(n) is int and n>0 for n in mode['nanoseconds']),'invalid service duration'
            c=mode['counts'];assert c['tokens']+c['dispatched']==(1 if mode['mode']==8 else 8),'work credit conservation'
            assert c['applied']<=1 and c['acknowledged']==c['applied'],'effect/ACK mismatch'
            assert c['deliveries']==c['applied']*(2 if mode['mode']==6 else 1),'notification comparison'
            assert c['peak_heap_bytes']<=c['allocated_bytes'] and c['allocations'] in [1,2,3],'allocation evidence'
        modes={m['mode']:m['counts'] for m in profile['modes']}
        assert modes[3]['reply_tick']<modes[2]['reply_tick'] and modes[3]['quiet_tick']==modes[2]['quiet_tick'],'lost latency/quiescence distinction'
        assert modes[4]['winner']==modes[5]['winner']==-1 and modes[5]['dispatched']>modes[4]['dispatched'],'lost correlated work comparison'
        assert modes[9]['reply_tick']==3,'Retry-After rounding'
    assert data['physical_validation']=='not-run' and data['learner_mastery']=='not-assessed','overclaimed service evidence'
    if check_files:pins(data['input_sha256']);logs(data['executions'])


def self_test(campaign,cost,service_cost):
    mutations=[
        ('campaign',lambda d:d.update(passed=False)),
        ('campaign',lambda d:d['profiles'].pop()),
        ('campaign',lambda d:d['learner_stub_rejections'].pop()),
        ('campaign',lambda d:d['profiles'][0]['controls_by_anchor']['resilience.timer_structures'].clear()),
        ('campaign',lambda d:d['profiles'][0]['controls_by_anchor']['resilience.retry_transport'].__setitem__(1,'resilience.mutation.retry_transport.1')),
        ('campaign',lambda d:d['profiles'][0]['compiled_mutant_controls'].remove('resilience.mutation.retry_transport.2')),
        ('campaign',lambda d:d['profiles'][0]['controls_by_anchor']['resilience.pressure_owned'].pop()),
        ('campaign',lambda d:d['profiles'][0]['controls_by_anchor']['resilience.breaker_window'].pop()),
        ('campaign',lambda d:d['profiles'][0]['controls_by_anchor']['resilience.hedge_owned'].pop()),
        ('campaign',lambda d:d['profiles'][0]['controls_by_anchor']['resilience.receiver_journal'].pop()),
        ('campaign',lambda d:d['profiles'][0]['controls_by_anchor']['resilience.service_outbox'].pop()),
        ('campaign',lambda d:d['profiles'][0]['controls_by_anchor']['resilience.service_io'].pop()),
        ('campaign',lambda d:d['profiles'][0]['controls_by_anchor']['resilience.service_transport'].pop()),
        ('campaign',lambda d:d['compiler_dependency_closures'].pop()),
        ('campaign',lambda d:d['compiler_dependency_closures'][0]['repository_inputs'].append('not-captured.c')),
        ('campaign',lambda d:d.update(external_inputs_stable=False)),
        ('campaign',lambda d:d['profiles'][3].update(thread_instrumentation_confirmed=False)),
        ('cost',lambda d:d['profiles'][0]['modes'].pop()),
        ('cost',lambda d:d['profiles'][0]['modes'][0]['nanoseconds'].pop()),
        ('cost',lambda d:d['profiles'][0]['modes'][-1]['counters'].update(comparisons=15)),
        ('cost',lambda d:d.update(physical_validation='passed')),
        ('service',lambda d:d['profiles'].pop()),
        ('service',lambda d:d['compiler_dependency_closures'].pop()),
        ('service',lambda d:d['profiles'][0]['modes'].pop()),
        ('service',lambda d:d['profiles'][0]['modes'][0]['nanoseconds'].pop()),
        ('service',lambda d:d['profiles'][0]['modes'][6]['counts'].update(deliveries=1)),
        ('service',lambda d:d['profiles'][0]['modes'][3]['counts'].update(quiet_tick=3)),
        ('service',lambda d:d['profiles'][0]['builds']['timing'].pop('single_unit_rebuild')),
        ('service',lambda d:d['profiles'][0]['mechanism_objects'].pop('pressure')),
        ('service',lambda d:d.update(physical_validation='passed')),
    ]
    for kind,mutate in mutations:
        data=copy.deepcopy({'campaign':campaign,'cost':cost,'service':service_cost}[kind]);mutate(data)
        try:{'campaign':check_campaign,'cost':check_cost,'service':check_service_cost}[kind](data,check_files=False)
        except (AssertionError,KeyError):pass
        else:raise AssertionError('negative evidence guard admitted '+kind)
    return len(mutations)


def closure_self_test():
    from run_resilience_scoped import dependency_paths,select_pins
    count=0
    with tempfile.TemporaryDirectory() as temporary:
        base=Path(temporary);root=base/'repo';build=base/'build';root.mkdir();build.mkdir()
        source=root/'core.c';header=root/'api.h';unrelated=root/'unrelated.c';external=base/'system.h'
        for p in [source,header,unrelated,external]:p.write_text('original\n',encoding='utf8')
        commands=[{'file':str(source),'directory':str(build)}]
        raw='object.o: #deps 3\n'+''.join('    '+str(p)+'\n' for p in [source,header,external])
        repo,environment,generated=dependency_paths(root,build,commands,raw)
        assert not generated
        assert repo=={'core.c','api.h'} and environment=={str(external.resolve())};count+=1
        before={p.name:sha(p) for p in [source,header,unrelated]};unrelated.write_text('changed\n',encoding='utf8')
        selected=select_pins(before,repo,root);assert all(sha(root/p)==h for p,h in selected.items());count+=1
        source.write_text('changed\n',encoding='utf8');assert any(sha(root/p)!=h for p,h in selected.items());count+=1
        for action in [lambda:dependency_paths(root,build,commands,''),
                       lambda:dependency_paths(root,build,[],raw),
                       lambda:dependency_paths(root,build,commands,raw+'    '+str(root/'missing.h')+'\n'),
                       lambda:select_pins(before,repo|{'missing.h'},root)]:
            try:action()
            except ValueError:count+=1
            else:raise AssertionError('Invalid dependency evidence admitted')
        obj=build/'object.o';obj.write_bytes(b'actual compiled object fixture')
        object_commands=[dict(commands[0],output=str(obj),command='cc -c '+str(source)+' -o '+str(obj))]
        repo,environment,generated=dependency_paths(root,build,object_commands,raw+'    '+str(obj)+'\n')
        assert generated[str(obj.resolve())]['sha256']==sha(obj) and generated[str(obj.resolve())]['producer_source']=='core.c';count+=1
        try:dependency_paths(root,build,commands,raw+'    '+str(obj)+'\n')
        except ValueError:count+=1
        else:raise AssertionError('Unknown generated link input admitted')
    return count

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--self-test',action='store_true');args=parser.parse_args()
    campaign=json.loads((ROOT/CAMPAIGN).read_text(encoding='utf8'));cost=json.loads((ROOT/COST).read_text(encoding='utf8'));service_cost=json.loads((ROOT/SERVICE_COST).read_text(encoding='utf8'))
    check_campaign(campaign);check_cost(cost);check_service_cost(service_cost)
    negative=self_test(campaign,cost,service_cost) if args.self_test else 0
    closure_checks=closure_self_test() if args.self_test else 0
    print(json.dumps({'passed':True,'profiles':8,'foundation_model_cases_per_profile':sum(EXPECTED_MODELS.values()),
                      'timer_model_cases_per_profile':71877,'retry_model_cases_per_profile':16452,'timer_cost_modes':60,'negative_controls':negative,
                      'pressure_model_cases_per_profile':121554,
                      'breaker_model_cases_per_profile':167908,
                      'hedge_model_cases_per_profile':23772,
                      'journal_model_cases_per_profile':3330,
                      'outbox_model_cases_per_profile':79896,
                      'service_wire_cases_per_profile':11714,
                      'composition_cases_per_profile':9096,'service_cost_workloads':40,'service_timing_samples':280,
                      'dependency_closure_checks':closure_checks,
                      'scope':'Current owned deadlines/fractional credit/timer/retry/pressure/breaker/hedge/journal/outbox/local-I/O/transport-composition successors and measured host costs. Exact objective readiness is separate; independent assessment and physical evidence are not inferred.'}))
    return 0

if __name__=='__main__':raise SystemExit(main())
