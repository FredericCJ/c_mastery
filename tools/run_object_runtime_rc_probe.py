"""Measure ordinary and deferred count updates under equal terminal lifetimes."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
PROBE = r'''
#define _POSIX_C_SOURCE 200809L
#include "object_runtime_rc_concurrent.h"
#include "probe_config.h"
#include <stdio.h>
#include <time.h>
static volatile uint64_t checksum;
static uint64_t ns(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))return 0;return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;}
#define REQUIRE(c) do{if(!(c))return 1;}while(0)
int main(void){unsigned shape,mode,trial,round;
 printf("LAYOUT %zu %zu %zu %zu %zu\n",sizeof(struct orr_heap),sizeof(struct or_counts),sizeof(struct orr_trial),sizeof(struct orr_concurrent),sizeof(struct orr_handle));
 for(shape=0;shape<2;shape++)for(mode=0;mode<3;mode++)for(trial=0;trial<16;trial++){
  unsigned zero_retired=0,cycle_retired=0,flushed=0,drains=0;uint64_t start=ns(),end;
  for(round=0;round<PROBE_ROUNDS;round++){
   struct orr_heap h;struct orr_handle a[8];struct orr_trial t;enum or_status s;unsigned i,n;uint16_t dead;
   orr_init(&h);for(i=0;i<8;i++)REQUIRE(orr_new(&h,&a[i])==OR_OK);
   for(i=0;i<7;i++)REQUIRE(orr_store(&h,a[i],0,a[i+1],0,&dead)==OR_OK);
   if(shape)REQUIRE(orr_store(&h,a[7],0,a[0],0,&dead)==OR_OK);
   for(i=0;i<8;i++){
    REQUIRE(orr_release(&h,a[i],mode!=0,&dead)==OR_OK);
    for(n=0;n<8;n++)zero_retired+=(dead>>n)&1;
   }
   do{
    s=orr_flush(&h,mode==1?1:8,&n,&dead);flushed+=n;drains++;
    REQUIRE(s==OR_OK||s==OR_DONE);for(n=0;n<8;n++)zero_retired+=(dead>>n)&1;
   }while(s!=OR_DONE);
   REQUIRE(orr_prepare(&h,&t)==OR_OK&&orr_commit(&h,&t,&dead)==OR_OK);
   for(n=0;n<8;n++)cycle_retired+=(dead>>n)&1;
   REQUIRE(h.counts.live==0&&orr_invariant(&h)&&orr_close(&h)==OR_OK);
  }
  end=ns();REQUIRE(start&&end>=start);checksum+=zero_retired+cycle_retired;
  printf("RC %u %u %u %llu %u %u %u %u\n",shape,mode,trial,(unsigned long long)(end-start),zero_retired,cycle_retired,flushed,drains);
 }
 return checksum==0;
}
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--report', type=Path, default=ROOT/'reviews/v5/expansion/modern_completion/object_runtime_rc_probe.json')
    args = parser.parse_args()
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    build = ROOT/'build/modern_completion'/('object-rc-'+stamp)
    logs = ROOT/'reviews/v5/expansion/modern_completion/runs'/('object-rc-'+stamp)
    build.mkdir(parents=True); logs.mkdir(parents=True)
    source = build/'probe.c'; source.write_text(PROBE, encoding='utf8')
    config = build/'probe_config.h'; config.write_text('#define PROBE_ROUNDS 1000\n', encoding='utf8')
    paths = ['tools/run_object_runtime_rc_probe.py','labs/object_runtime/include/object_runtime.h','labs/object_runtime/include/object_runtime_rc.h','labs/object_runtime/include/object_runtime_rc_concurrent.h','labs/object_runtime/REFERENCE_COUNT_EXTENSIONS.md','assessor/object_runtime/object_runtime.c','assessor/object_runtime/object_runtime_rc.c']
    digest = lambda p: hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
    pins = {p: digest(p) for p in paths}; executions = []; observations = []; passed = False

    def run(cmd, label):
        start = time.monotonic()
        r = subprocess.run(list(map(str, cmd)), cwd=build, capture_output=True, text=True, timeout=90)
        output = r.stdout+r.stderr
        p = logs/(f'{len(executions):03}-{label}.log'); p.write_text(output, encoding='utf8')
        executions.append({'command': list(map(str, cmd)), 'cwd': str(build), 'exit_code': r.returncode,
                           'seconds': time.monotonic()-start, 'stdout': output, 'log': p.relative_to(ROOT).as_posix(),
                           'log_sha256': hashlib.sha256(p.read_bytes()).hexdigest()})
        if r.returncode:
            raise RuntimeError(output[-1500:])
        return output

    try:
        for compiler in ['gcc', 'clang']:
            run([compiler, '--version'], compiler+'-version')
            for opt in ['-O0', '-O2', '-O3']:
                tag = compiler+opt; project = build/tag; project.mkdir(); output = project/'out'
                units = [source, ROOT/'assessor/object_runtime/object_runtime.c', ROOT/'assessor/object_runtime/object_runtime_rc.c']
                (project/'CMakeLists.txt').write_text(
                    'cmake_minimum_required(VERSION 3.20)\nproject(rc_probe C)\nset(CMAKE_C_STANDARD 17)\nset(CMAKE_C_EXTENSIONS OFF)\nfind_package(Threads REQUIRED)\nadd_executable(probe '+
                    ' '.join('"'+str(x)+'"' for x in units)+')\ntarget_include_directories(probe PRIVATE "'+str(ROOT/'labs/object_runtime/include')+'" "'+str(ROOT/'labs/ownership/include')+'")\ntarget_link_libraries(probe PRIVATE Threads::Threads)\ntarget_compile_options(probe PRIVATE '+opt+' -Wall -Wextra -Wpedantic -Werror -fstack-usage)\n', encoding='utf8')
                run(['cmake', '-S', project, '-B', output, '-G', 'Ninja', '-DCMAKE_C_COMPILER='+compiler], tag+'-configure')
                clean = len(executions); run(['cmake', '--build', output, '--parallel', '4'], tag+'-clean')
                unchanged = len(executions); text = run(['cmake', '--build', output, '--parallel', '4'], tag+'-unchanged')
                assert 'no work to do' in text
                config.write_text('#define PROBE_ROUNDS 1000\n/* rebuild '+tag+' */\n', encoding='utf8')
                changed = len(executions); text = run(['cmake', '--build', output, '--parallel', '4'], tag+'-one-header')
                assert 'probe.c.o' in text
                exe = output/'probe'; text = run([exe], tag+'-execute'); samples = []; warmups = []; layout = None
                for line in text.splitlines():
                    fields = line.split(); values = list(map(int, fields[1:]))
                    if fields[0] == 'LAYOUT':
                        layout = values
                    elif fields[0] == 'RC':
                        shape,mode,trial,elapsed,zero,cycle,flushed,drains=values
                        assert (zero,cycle,flushed,drains)==(8000*(1-shape),8000*shape,8000 if mode else 0,8000 if mode==1 else 1000) and elapsed>0,line
                        (warmups if trial==0 else samples).append({'shape':'cycle' if shape else 'chain','mode':['immediate','deferred-budget1','deferred-budget8'][mode],'trial':trial,'elapsed_ns':elapsed,'lifecycles':1000,'zero_count_reclaimed':zero,'trial_reclaimed':cycle,'decrements_flushed':flushed,'flush_calls':drains})
                assert len(samples)==90 and len(warmups)==6 and layout
                size = run(['size', exe], tag+'-size'); run(['objdump', '-d', exe], tag+'-assembly')
                stack = []
                for index, p in enumerate(sorted(output.rglob('*.su'))):
                    q = logs/(tag+'-'+str(index)+'.su'); q.write_bytes(p.read_bytes())
                    stack.append({'path': q.relative_to(ROOT).as_posix(), 'sha256': hashlib.sha256(q.read_bytes()).hexdigest()})
                observations.append({'compiler': compiler, 'optimization': opt, 'layout': layout, 'samples': samples,
                    'discarded_first_samples': warmups, 'image_size': size, 'stack_artifacts': stack,
                    'build_execution_indices': {'clean': clean, 'unchanged': unchanged, 'one_header_changed': changed}})
        passed = all(digest(p) == v for p, v in pins.items())
    except Exception as error:
        failure = str(error)
    result = {'schema_version': 1, 'status': 'passed' if passed else 'failed', 'passed': passed, 'inspected_paths': pins,
        'scope_source_stable': all(digest(p) == v for p, v in pins.items()), 'observations': observations, 'executions': executions,
        'limits': ['540 retained samples plus36 discarded warmups;1000 complete8-object lifetimes/sample, split chain/cycle shapes and three decrement policies.',
                  'For each shape all policies have identical roots/edges, admitted capacity, terminal empty heap and exact reclaimed identities. Timing includes admission, updates, flush, trial, invariant check and close.',
                  'Core timing only; actual concurrent retain/release and revision publication are tested separately under mutex and TSan. No lock-free, fairness, real-time or physical claim.',
                  'Whole-image size and individual compiler frame reports are host-specific and are not full call-chain stack bounds.']}

    if not passed:
        result['error'] = locals().get('failure', 'source changed')
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(result, indent=2)+'\n', encoding='utf8')
    print(json.dumps({'passed': passed, 'report': str(args.report)}))
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
