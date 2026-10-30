"""Revalidate ten exact T68 packets and derive named workload costs from raw samples."""
from datetime import datetime,timezone
import json
from pathlib import Path
import subprocess
import sys
import time
from common import ROOT,dump,read_json,sha
from source_equivalence import carried_binding
NAMES=['owning_ring','capacity','policy_effects','logging','receivers','schema','law_traces','format_stream','nested_hygiene','build_cache']
FOLDERS=NAMES
CONTRACTS=['OWNING.md','CAPACITY.md','POLICY.md','LOGGING.md','RECEIVERS.md','SCHEMA.md','LAWS.md','FORMAT.md','HYGIENE.md','CACHE.md']
DIMENSIONS=['Representation','Reachable storage','Lifetime','Mutation/aliasing','Allocation','Movement/copying','Destruction','Dispatch','Concurrency','Failure','Compilation','Binary/stack','Boundary']
REPORT='research/v5/modern/metaprog_measurement_closure_validation.json'
WORK=[
 ('Four owned-byte clones, four pops and four destructor calls per iteration; allocation/cleanup included.',['typed_seconds','manual_seconds','erased_seconds'],[['typed_seconds','manual_seconds','erased_seconds']],'Fresh/empty destinations and identical clone/drop/failure contract; erased storage is actual typed objects, not arbitrary byte buffers.'),
 ('Four pushes then four pops of values0..3 in a capacity4 ring per iteration.',['fixed_seconds','runtime_seconds'],[['fixed_seconds','runtime_seconds']],'Same capacity and success/failure behavior; runtime control initialization is inside the measured sample, once per100000 iterations.'),
 ('One four-element acquire/project/reduce/release traversal per iteration.',['reject_dynamic_seconds','reject_specialized_seconds','saturate_dynamic_seconds','saturate_specialized_seconds'],[['reject_dynamic_seconds','reject_specialized_seconds'],['saturate_dynamic_seconds','saturate_specialized_seconds']],'Compare dynamic/specialized only within one policy. Reject and saturation are different contracts; saturation retains all callback effects.'),
 ('Reset sink count and append one copied four-number record without site metadata per iteration.',['macro_seconds','direct_seconds'],[['macro_seconds','direct_seconds']],'Same typed fields and no metadata/I/O in either path; wrapper evaluation semantics remain part of the contract.'),
 ('One volatile uint32 read into category3 descriptor per iteration.',['generic_seconds','direct_seconds'],[['generic_seconds','direct_seconds']],'Same volatile host object and admitted category; neither series measures a peripheral bus or makes volatile an atomic primitive.'),
 ('One validated version2 three-field19-byte record encoding per iteration.',['seconds'],[],'One complete encoding workload; no unequal-contract reflection-library speed comparison.'),
 ('One complete same-failure operation-trace shrink to a three-operation witness per iteration.',['seconds'],[],'Includes replay allocations, cleanup and observer work. Not a single equality-call latency or proof of global minimality.'),
 ('One preparation of value=4294967295 from a previously parsed program per iteration.',['seconds'],[],'Parsing is outside this series; message validation, copied ownership and decimal formatting are inside. Stream I/O has a separate partial-progress contract.'),
 ('One complete two-iteration zip expansion with six emitted tokens per iteration.',['seconds'],[],'Includes structural validation, namespace authority checks and depth/token/step budgets; not full Rust macro compilation.'),
 ('One complete validated three-node plan including actual signed codec/hash/emitter per iteration.',['seconds'],[],'Expected artifacts are recomputed; this timing excludes filesystem synchronization and parallel scheduling, which have separately observed correctness experiments.')]

def derived(result,series):
    n=result['iterations'];return {key:[dict(batch_cpu_seconds=t,mean_cpu_seconds_per_iteration=t/n,iterations_per_cpu_second=n/t if t>0 else None,resolution_limited=t==0) for t in result[key]] for key in series}

def run():
    inputs={p:sha(ROOT/p) for p in ['tools/run_metaprog_measurement_closure.py','tools/common.py','tools/check_replicated_replay_depth.py','curriculum/v5/exercises/modern-cost-contract.md']};reports={};carried=[];packets=[];external={};records=[];observations=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('metaprog-measurement-closure-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    for i,(name,contract,work) in enumerate(zip(NAMES,CONTRACTS,WORK),1):
        path='research/v5/modern/metaprog_'+name+'_readiness.json';r=read_json(ROOT/path);assert [x['id'] for x in r['criteria']]==[f'X-T68-{i:03}-R{j}' for j in range(1,5)] and all(x['author_readiness']=='ready-bounded' for x in r['criteria'])
        for p,h in r['input_sha256'].items():
            binding=None if sha(ROOT/p)==h else carried_binding(p,h)
            assert sha(ROOT/p)==h or binding,p
            inputs[p]=sha(ROOT/p)
            if binding:carried.append(binding)
        inputs[path]=sha(ROOT/path)
        for p,h in r['current_reports'].items():
            assert sha(ROOT/p)==h;reports[p]=h;external.update(read_json(ROOT/p).get('external_input_sha256',{}))
        main,companion=[read_json(ROOT/p) for p in r['current_reports']];raw=[x for x in main['observations'] if x['kind']=='metrics'];builds=[x for x in companion['observations'] if 'clean_seconds' in x and 'unchanged_seconds' in x]
        assert len(raw)==6 and builds and all(x['unchanged_no_work'] for x in builds)
        source='labs/metaprog/'+name+'/tests/tests.c';contract_path='labs/metaprog/'+name+'/'+contract
        packets.append(dict(name=name,exercise=f'X-T68-{i:03}',criteria=[x['id'] for x in r['criteria']],readiness=path,reports=list(r['current_reports']),contract=contract_path,dimensions=DIMENSIONS,workload=work[0],metrics_source=source,series=work[1],equivalent_groups=work[2],equivalence_limit=work[3],raw_metrics=raw,derived_metrics=[dict(profile=x['profile'],series=derived(x['result'],work[1])) for x in raw],build_observations=builds,observer_policy='Seven batches, zero separate warmup. clock() CPU time includes checks/accumulation and can round to zero; mean per iteration is not tail latency or per-operation WCET. No cross-profile or cross-workload universal speed claim.'))
    for name in NAMES:
        command=[sys.executable,str(ROOT/('tools/check_metaprog_'+name+'.py')),'--self-test'];start=time.monotonic();p=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=150);text=p.stdout+'\n'+p.stderr;log=folder/(name+'-guard.log');log.write_text(text,encoding='utf8');records.append(dict(command=command,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert p.returncode==0,(name,text);r=json.loads(p.stdout);assert r['passed'];observations.append(dict(kind='current-guard',name=name,result=r))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(Path(p).is_file() and sha(Path(p))==h for p,h in external.items())
    result=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',input_sha256=inputs,current_reports=reports,carried_forward=carried,external_input_sha256=external,source_stable=stable,external_inputs_stable=ext,packets=packets,executions=records,observations=observations,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Original T68-MEASUREMENTS is bound to ten actual contracts and40rubrics, not headings. All original gaps remain as facet records.','Derived rates are arithmetic on retained CPU-time samples, not new executions; storage/process experiments do not establish physical power-cut or bus timing.','Only listed equivalent groups support relative comparisons. Policy changes, parse versus stream, and graph versus filesystem are different workloads.','All13cost dimensions include contract reasoning and source/object evidence; counts or quiet sanitizers do not prove universal bounds.','Published C17/C23 and attempted POSIX2024 pages remain access gaps; no independent reviewer or learner mastery is manufactured.'])
    dump(ROOT/REPORT,result);print(json.dumps(dict(passed=result['passed'],commands=len(records),current_packets=len(packets),rubrics=40)));return result['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
