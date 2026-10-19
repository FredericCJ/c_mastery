"""Observed cost of actual T75 transport/outbox composition, distinct from virtual latency."""
import argparse
from datetime import datetime,timezone
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import statistics
import subprocess
import sys
import tempfile
import time
from common import ROOT,sha
from run_expansion_family import family_inputs
from run_resilience_scoped import dependency_paths,select_pins
from v5_common import pins,tree_files

MODES=['nominal','burst_loss','tail_unhedged','tail_hedged','correlated_unhedged','correlated_hedged',
       'slow_consumer_restart','cancel_late_owners','one_credit','retry_after']

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--report',default='research/v5/modern/resilience_service_cost_validation.json');args=parser.parse_args()
    if os.name=='nt':
        from v5_common import python_command
        return subprocess.run(python_command('tools/run_resilience_service_cost.py','--report',args.report,platform='wsl'),cwd=ROOT).returncode
    fixed={p.relative_to(ROOT).as_posix() for p in tree_files('labs/resilience')+tree_files('assessor/resilience')}
    fixed.update(['tools/run_resilience_service_cost.py','tools/run_resilience_scoped.py','tools/run_expansion_family.py',
                  'tools/common.py','tools/v5_common.py','tools/verify_resilience_composition.py','tools/verify_resilience_pressure.py',
                  'research/v5/modern/resilience_service_decision.json'])
    for family in ['messaging','execution','communications','hosted_systems']:fixed.update(['labs/'+family+'/family.json','labs/'+family+'/CMakeLists.txt'])
    before=pins(family_inputs('resilience')+[ROOT/p for p in fixed]);required=set(fixed);external={}
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs=ROOT/'reviews/v5/expansion/modern_completion/runs'/('resilience-service-cost-'+stamp);logs.mkdir(parents=True)
    records=[];profiles=[];closures=[];passed=True
    def capture(text,label,**fields):
        path=logs/(f'{len(records):04d}-{label}.log');path.write_text(text,encoding='utf8')
        row={'log':path.relative_to(ROOT).as_posix(),'log_sha256':sha(path),**fields};records.append(row);return row
    def execute(command,label,cwd=ROOT):
        start=time.monotonic();result=subprocess.run(list(map(str,command)),cwd=cwd,text=True,capture_output=True,timeout=300)
        row=capture(result.stdout+'\n'+result.stderr,label,command=list(map(str,command)),cwd=str(cwd),exit_code=result.returncode,seconds=time.monotonic()-start)
        if result.returncode:raise RuntimeError(label+' failed: '+result.stderr)
        return result.stdout,row
    try:
        execute(['uname','-a'],'environment');execute(['getconf','LONG_BIT'],'data-model')
        for tool in ['gcc','clang','cmake','ninja','objdump','size','nm']:
            path=Path(shutil.which(tool)).resolve();external[str(path)]=sha(path);execute([tool,'--version'],tool+'-version')
        external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve())
        with tempfile.TemporaryDirectory(prefix='cmastery-resilience-service-cost-',dir=Path.home()) as temporary:
            for compiler in ['gcc','clang']:
                for optimization in ['0','3']:
                    label=compiler+'-O'+optimization;builds={};objects={};build_records={};pressure_maps={};pressure_objects={}
                    for variant in ['counter','timing']:
                        build=Path(temporary)/(label+'-'+variant);builds[variant]=build
                        flags='-O'+optimization+' -g -fstack-usage'+(' -DRZT_NO_WORK' if variant=='timing' else '')
                        execute(['cmake','-S',ROOT/'labs/resilience','-B',build,'-G','Ninja','-DCMAKE_C_COMPILER='+compiler,
                                 '-DCMAKE_C_FLAGS='+flags,'-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
                                 '-DCMAKE_EXE_LINKER_FLAGS=-Wl,-Map,'+str(build/'composition.map'),
                                 '-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],label+'-'+variant+'-configure')
                        _,clean=execute(['cmake','--build',build,'--target','resilience-composition','--parallel','4'],label+'-'+variant+'-clean')
                        _,pressure_clean=execute(['cmake','--build',build,'--target','resilience-pressure','--parallel','4'],label+'-'+variant+'-pressure-clean')
                        pressure_maps[variant]=capture((build/'composition.map').read_text(),label+'-'+variant+'-pressure-map',exit_code=0)
                        commands=[c for c in json.loads((build/'compile_commands.json').read_text()) if any(t+'.dir/' in c.get('command','') for t in ['resilience-composition','resilience-pressure'])]
                        assert commands,'missing target compilation records'
                        for c in commands:
                            argv=shlex.split(c['command']);c['output']=argv[argv.index('-o')+1]
                        _,unchanged=execute(['cmake','--build',build,'--target','resilience-composition'],label+'-'+variant+'-unchanged')
                        composition=next(c for c in commands if c['file'].endswith('/assessor/resilience/followthrough/composition.c'))
                        obj=(build/composition['output']).resolve();assert obj.is_relative_to(build.resolve()) and obj.is_file();objects[variant]=obj
                        obj.unlink()
                        _,incremental=execute(['cmake','--build',build,'--target','resilience-composition'],label+'-'+variant+'-single-unit-rebuild')
                        build_records[variant]={'clean':clean,'unchanged':unchanged,'single_unit_rebuild':incremental,'pressure_clean':pressure_clean}
                        pressure_source=next(c for c in commands if c['file'].endswith('/assessor/resilience/followthrough/pressure.c'))
                        pressure_objects[variant]=(build/pressure_source['output']).resolve()
                        raw,_=execute(['ninja','-C',build,'-t','deps'],label+'-'+variant+'-dependencies')
                        repo,system,generated=dependency_paths(ROOT,build,commands,raw);required.update(repo)
                        for path in system:external.setdefault(path,sha(Path(path)))
                        compile_log=capture(json.dumps(commands,indent=2),label+'-'+variant+'-compile-database',exit_code=0)
                        closures.append({'configuration':label+'-'+variant,'repository_inputs':sorted(repo),'external_inputs':sorted(system),'generated':generated,'compile_database':compile_log})
                        execute(['ninja','-C',build,'-t','commands','resilience-composition'],label+'-'+variant+'-link-commands')
                        execute([sys.executable,ROOT/'tools/verify_resilience_composition.py','--driver',build/'resilience-composition'],label+'-'+variant+'-oracle')
                        execute([build/'resilience-pressure','test'],label+'-'+variant+'-pressure-native')
                        execute([sys.executable,ROOT/'tools/verify_resilience_pressure.py','--driver',build/'resilience-pressure'],label+'-'+variant+'-pressure-oracle')
                        raw,_=execute(['ldd',build/'resilience-composition'],label+'-'+variant+'-libraries')
                        import re
                        for path in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',raw,re.MULTILINE):
                            if Path(path).is_file():external.setdefault(str(Path(path).resolve()),sha(Path(path).resolve()))
                    counter=builds['counter']/'resilience-composition';timing=builds['timing']/'resilience-composition'
                    raw,layout_record=execute([counter,'layout'],label+'-layout');layout=json.loads(raw)
                    stack=capture('\n'.join(p.relative_to(builds['timing']).as_posix()+'\n'+p.read_text() for p in sorted(builds['timing'].rglob('*.su'))),label+'-stack',exit_code=0)
                    link_map=capture((builds['timing']/'composition.map').read_text(),label+'-map',exit_code=0)
                    _,binary=execute(['size',timing],label+'-size');_,assembly=execute(['objdump','-drwC',objects['timing']],label+'-assembly');_,undefined=execute(['nm','-u',objects['timing']],label+'-undefined')
                    _,pressure_binary=execute(['size',builds['timing']/'resilience-pressure'],label+'-pressure-size')
                    _,pressure_assembly=execute(['objdump','-drwC',pressure_objects['timing']],label+'-pressure-assembly')
                    _,pressure_undefined=execute(['nm','-u',pressure_objects['timing']],label+'-pressure-undefined')
                    mechanism_objects={}
                    for entry in commands:
                        source=Path(entry['file']);part=source.stem
                        if '/assessor/resilience/followthrough/' not in source.as_posix() or part in mechanism_objects:continue
                        obj=(builds['timing']/entry['output']).resolve()
                        assert obj.is_relative_to(builds['timing'].resolve()) and obj.is_file()
                        _,object_size=execute(['size',obj],label+'-'+part+'-object-size')
                        _,object_assembly=execute(['objdump','-drwC',obj],label+'-'+part+'-object-assembly')
                        _,object_symbols=execute(['nm','-u',obj],label+'-'+part+'-object-undefined')
                        mechanism_objects[part]={'source':source.relative_to(ROOT).as_posix(),'object_sha256':sha(obj),
                                                  'size':object_size,'assembly':object_assembly,'undefined_symbols':object_symbols}
                    assert set(mechanism_objects)=={'scope','fractional','timers','retry','pressure','breaker','hedge','journal','outbox','service_io','composition'}
                    modes=[]
                    for mode,name in enumerate(MODES):
                        raw,count_log=execute([counter,'benchmark',mode,1],label+'-'+name+'-counts');counts=json.loads(raw);samples=[];sample_logs=[]
                        for repetition in range(8):
                            raw,row=execute([timing,'benchmark',mode,100],label+'-'+name+'-'+str(repetition));observed=json.loads(raw)
                            assert observed['checksum']==100*counts['checksum']
                            for key,value in counts.items():
                                if key not in ['rounds','nanoseconds','checksum'] and not key.startswith('timer_'):assert observed[key]==value,(label,name,key)
                            assert all(observed[key]==0 for key in ['timer_comparisons','timer_links','timer_ticks','timer_cascade'])
                            if repetition:samples.append(observed['nanoseconds']);sample_logs.append(row)
                        modes.append({'mode':mode,'workload':name,'counts':counts,'count_log':count_log,'warmups':1,'repetitions':7,'requests_per_sample':100,'nanoseconds':samples,
                                      'median_nanoseconds':statistics.median(samples),'min_nanoseconds':min(samples),'max_nanoseconds':max(samples),'sample_logs':sample_logs})
                    profiles.append({'configuration':label,'layout':layout,'layout_log':layout_record,'builds':build_records,'modes':modes,'stack':stack,'map':link_map,'binary':binary,'assembly':assembly,'undefined_symbols':undefined,
                                     'pressure':{'map':pressure_maps['timing'],'binary':pressure_binary,'assembly':pressure_assembly,'undefined_symbols':pressure_undefined},
                                     'mechanism_objects':mechanism_objects})
                    print(label+': 10 operation-count workloads and70 repeated host samples passed',flush=True)
    except (AssertionError,RuntimeError,ValueError,subprocess.TimeoutExpired) as error:
        passed=False;records.append({'exit_code':1,'error':repr(error)});print(repr(error),flush=True)
    selected=select_pins(before,required);stable=selected==pins([ROOT/p for p in selected]);external_stable=all(Path(p).is_file() and sha(Path(p))==h for p,h in external.items())
    passed=passed and stable and external_stable and len(profiles)==4
    data={'schema_version':1,'status':'passed' if passed else 'failed','passed':passed,'source_stable':stable,'external_inputs_stable':external_stable,
          'recorded_at':datetime.now(timezone.utc).isoformat(),'input_sha256':selected,'external_input_sha256':external,'compiler_dependency_closures':closures,
          'profiles':profiles,'executions':records,'physical_validation':'not-run','learner_mastery':'not-assessed',
          'limitations':['Virtual event ticks are not measured network latency.','Host batch timings include fixture setup, malloc and tracking, checks and synchronization; only timer diagnostic counts are disabled.',
                         'One hosted ABI; static frame entries do not include transitive callbacks/library/thread/OS stacks.','Memory-backed device callbacks count writes/flush calls, not physical persistence latency.',
                         'Functional exploration preceded the disclosed cost prediction. No independent learner seal or mastery inferred.']}
    (ROOT/args.report).write_text(json.dumps(data,indent=2)+'\n',encoding='utf8');return 0 if passed else 1

if __name__=='__main__':raise SystemExit(main())
