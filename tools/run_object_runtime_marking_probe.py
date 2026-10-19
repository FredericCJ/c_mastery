"""Measure the three active-allocation policies under one graph/lifetime contract."""
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
#include "object_runtime_concurrent.h"
#include "probe_config.h"
#include <stdio.h>
#include <time.h>
static volatile uint64_t checksum;
static uint64_t ns(void) { struct timespec t; if(clock_gettime(CLOCK_MONOTONIC,&t))return 0;return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec; }
#define REQUIRE(c) do{if(!(c))return 1;}while(0)
int main(void) {
 unsigned policy,trial,round;
 const ORCHandle nil={0,0};
 printf("LAYOUT %zu %zu %zu %zu %zu\n",sizeof(ORCHeap),_Alignof(ORCHeap),sizeof(ORCHandle),sizeof(ORCFinish),sizeof(ORCConcurrent));
 for(policy=0;policy<3;policy++)for(trial=0;trial<16;trial++) {
  uint64_t start=ns(),end,total=0;unsigned scans=0,restarts=0;
  for(round=0;round<PROBE_ROUNDS;round++) {
   ORCHeap h;ORCHandle a,b,n;ORCFinish token;unsigned done;uint16_t dead;ORStatus status;
   orcm_init(&h,(ORCPolicy)policy);
   REQUIRE(orcm_allocate(&h,nil,nil,&a)==OR_OK && orcm_allocate(&h,nil,nil,&b)==OR_OK);
   REQUIRE(orcm_release(&h,b)==OR_OK && orcm_begin(&h)==OR_OK);
   REQUIRE(orcm_step(&h,1,&done)==OR_DONE && done==1);
   REQUIRE(orcm_allocate(&h,b,nil,&n)==OR_OK);
   do{status=orcm_step(&h,1,&done);REQUIRE(status==OR_OK||status==OR_DONE);}while(status!=OR_DONE);
   REQUIRE(orcm_prepare(&h,&token)==OR_OK && orcm_commit(&h,token,&dead)==OR_OK && dead==0 && h.graph.live==7);
   scans+=h.scans;restarts+=h.restarts;total+=h.graph.live;
   REQUIRE(orcm_release(&h,a)==OR_OK && orcm_release(&h,n)==OR_OK && orcm_begin(&h)==OR_OK);
   REQUIRE(orcm_step(&h,1,&done)==OR_DONE && done==0);
   REQUIRE(orcm_prepare(&h,&token)==OR_OK && orcm_commit(&h,token,&dead)==OR_OK && dead==7 && orcm_invariant(&h));
  }
  end=ns();REQUIRE(start&&end>=start);checksum+=total;
  printf("MARK %u %u %llu %llu %u %u\n",policy,trial,(unsigned long long)(end-start),(unsigned long long)total,scans,restarts);
 }
 return checksum==0;
}
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--report', type=Path, default=ROOT/'reviews/v5/expansion/modern_completion/object_runtime_marking_probe.json')
    args = parser.parse_args()
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    build = ROOT/'build/modern_completion'/('object-marking-'+stamp)
    logs = ROOT/'reviews/v5/expansion/modern_completion/runs'/('object-marking-'+stamp)
    build.mkdir(parents=True); logs.mkdir(parents=True)
    source = build/'probe.c'; source.write_text(PROBE, encoding='utf8')
    config = build/'probe_config.h'; config.write_text('#define PROBE_ROUNDS 1000\n', encoding='utf8')
    paths = ['tools/run_object_runtime_marking_probe.py', 'labs/object_runtime/include/object_runtime.h',
             'labs/object_runtime/include/object_runtime_marking.h', 'labs/object_runtime/include/object_runtime_concurrent.h',
             'labs/object_runtime/MARKING_EXTENSIONS.md', 'assessor/object_runtime/object_runtime.c',
             'assessor/object_runtime/object_runtime_marking.c', 'assessor/object_runtime/object_runtime_concurrent.c']
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
                units = [source, ROOT/'assessor/object_runtime/object_runtime.c', ROOT/'assessor/object_runtime/object_runtime_marking.c', ROOT/'assessor/object_runtime/object_runtime_concurrent.c']
                (project/'CMakeLists.txt').write_text(
                    'cmake_minimum_required(VERSION 3.20)\nproject(mark_probe C)\nset(CMAKE_C_STANDARD 17)\nset(CMAKE_C_EXTENSIONS OFF)\nfind_package(Threads REQUIRED)\nadd_executable(probe '+
                    ' '.join('"'+str(x)+'"' for x in units)+')\ntarget_include_directories(probe PRIVATE "'+str(ROOT/'labs/object_runtime/include')+'")\ntarget_link_libraries(probe PRIVATE Threads::Threads)\ntarget_compile_options(probe PRIVATE '+opt+' -Wall -Wextra -Wpedantic -Werror -fstack-usage)\n', encoding='utf8')
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
                    elif fields[0] == 'MARK':
                        policy, trial, elapsed, total, scans, restarts = values
                        assert (total, scans, restarts) == (7000, (2+policy)*1000, 1000 if policy == 2 else 0) and elapsed > 0, line
                        (warmups if trial == 0 else samples).append({'policy': ['black', 'gray', 'restart'][policy], 'trial': trial,
                            'elapsed_ns': elapsed, 'rounds': 1000, 'checksum': total, 'scans': scans, 'restarts': restarts})
                assert len(samples) == 45 and len(warmups) == 3 and layout
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
        'limits': ['270 retained samples and18 discarded warmups;1000 complete graph lifecycles/sample with equal roots, allocation, late edge, survivor and cleanup contracts.',
                  'Initialization, checks, finite-counter increments, sweep, full-heap staging and observer overhead included. Scan counts are separate from time; no subtraction of required correctness costs.',
                  'The timing loop uses the serialized core; actual concurrent schedules belong to the semantic campaign. No claimed mutex contention timing, latency bound, parallel tracing speedup or physical target evidence.',
                  'Whole-image size includes original runtime and wrapper translation units. Stack records are compiler observations, not a whole-program recursion or asynchronous-stack proof.']}
    if not passed:
        result['error'] = locals().get('failure', 'source changed')
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(result, indent=2)+'\n', encoding='utf8')
    print(json.dumps({'passed': passed, 'report': str(args.report)}))
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
