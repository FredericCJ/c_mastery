"""Measure complete finalization and cleanup under two bounded drain budgets."""
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
#include "object_runtime_finalizers.h"
#include "probe_config.h"
#include <stdio.h>
#include <time.h>
static volatile uint64_t checksum;
static uint64_t ns(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))return 0;return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;}
#define REQUIRE(c) do{if(!(c))return 1;}while(0)
static ORStatus failed(ORFHeap *h,ORFHandle a,void *ctx){(void)h;(void)a;(*(unsigned*)ctx)++;return OR_FULL;}
int main(void){unsigned count,budget,trial,round;
 printf("LAYOUT %zu %zu %zu\n",sizeof(ORFHeap),sizeof(ORWeak),sizeof(ORFHandle));
 for(count=1;count<=16;count+=15)for(budget=1;budget<=16;budget+=15)for(trial=0;trial<16;trial++){
  unsigned calls=0,released=0,drains=0;uint64_t start=ns(),end;
  for(round=0;round<PROBE_ROUNDS;round++){
   ORFHeap h;ORFHandle a,out;ORStatus status,s;unsigned i,n;uint16_t dead;
   orf_init(&h);
   for(i=0;i<count;i++){REQUIRE(orf_new(&h,failed,&calls,1,0,&a)==OR_OK);REQUIRE(orf_root(&h,a,0)==OR_OK);}
   REQUIRE(orf_collect(&h,&dead)==OR_OK&&dead==0);
   for(i=0;i<count;i++){REQUIRE(orf_run_one(&h,&out,&status)==OR_OK&&status==OR_FULL);}
   do{s=orf_cleanup(&h,budget,&n);released+=n;drains++;REQUIRE(s==OR_OK||s==OR_DONE);}while(s!=OR_DONE);
   REQUIRE(orf_collect(&h,&dead)==OR_OK&&dead==(uint16_t)((1u<<count)-1));
   REQUIRE(h.callbacks_run==count&&h.resources_closed==count&&orf_close(&h)==OR_OK);
  }
  end=ns();REQUIRE(start&&end>=start);checksum+=calls+released;
  printf("FINAL %u %u %u %llu %u %u %u\n",count,budget,trial,(unsigned long long)(end-start),calls,released,drains);
 }
 return checksum==0;
}
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--report', type=Path, default=ROOT/'reviews/v5/expansion/modern_completion/object_runtime_finalizer_probe.json')
    args = parser.parse_args()
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    build = ROOT/'build/modern_completion'/('object-finalizer-'+stamp)
    logs = ROOT/'reviews/v5/expansion/modern_completion/runs'/('object-finalizer-'+stamp)
    build.mkdir(parents=True); logs.mkdir(parents=True)
    source = build/'probe.c'; source.write_text(PROBE, encoding='utf8')
    config = build/'probe_config.h'; config.write_text('#define PROBE_ROUNDS 1000\n', encoding='utf8')
    paths = ['tools/run_object_runtime_finalizer_probe.py','labs/object_runtime/include/object_runtime.h','labs/object_runtime/include/object_runtime_finalizers.h','labs/object_runtime/FINALIZER_EXTENSIONS.md','assessor/object_runtime/object_runtime.c','assessor/object_runtime/object_runtime_finalizers.c']
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
                units = [source, ROOT/'assessor/object_runtime/object_runtime.c', ROOT/'assessor/object_runtime/object_runtime_finalizers.c']
                (project/'CMakeLists.txt').write_text(
                    'cmake_minimum_required(VERSION 3.20)\nproject(finalizer_probe C)\nset(CMAKE_C_STANDARD 17)\nset(CMAKE_C_EXTENSIONS OFF)\nfind_package(Threads REQUIRED)\nadd_executable(probe '+
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
                    elif fields[0] == 'FINAL':
                        count,budget,trial,elapsed,calls,released,drains=values
                        assert (calls,released,drains)==(1000*count,1000*count,1000*((count+budget-1)//budget)) and elapsed>0,line
                        (warmups if trial==0 else samples).append({'objects':count,'cleanup_budget':budget,'trial':trial,'elapsed_ns':elapsed,'cycles':1000,'callbacks':calls,'resources_closed':released,'cleanup_calls':drains})
                assert len(samples)==60 and len(warmups)==4 and layout
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
        'limits': ['360 retained samples plus24 discarded warmups;1000 complete allocate/unroot/collect/fail-finalizers/cleanup/reclaim/close cycles each.',
                  'Cleanup budgets1 and16 have the same terminal resource/object/callback contract. Work includes every required admission, callback result, cleanup and reclamation check; no unsafe fast path.',
                  'Single-threaded finite synthetic resources only. No host file descriptor, physical observation, universal speedup or compiler-generated root claim.',
                  'Whole-image size and per-function compiler stack frames are host-specific; frames are not whole-call-chain stack bounds.']}

    if not passed:
        result['error'] = locals().get('failure', 'source changed')
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(result, indent=2)+'\n', encoding='utf8')
    print(json.dumps({'passed': passed, 'report': str(args.report)}))
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
