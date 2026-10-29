"""Measure moving and unchanged nonmoving CVM collectors on equal admitted graphs."""
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
#include "object_runtime_vm.h"
#include "probe_config.h"
#include <stdio.h>
#include <time.h>
static volatile uint64_t checksum;
static uint64_t ns(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))return 0;return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;}
#define REQUIRE(c) do{if(!(c))return 1;}while(0)
int main(void){unsigned count,mode,trial,round;
 printf("LAYOUT %zu %zu %zu %zu %zu\n",sizeof(Cvm),sizeof(struct or_vm),sizeof(struct orv_stats),sizeof(struct orv_derived),sizeof(algo_pool));
 for(count=2;count<=16;count*=2)for(mode=0;mode<2;mode++)for(trial=0;trial<16;trial++){
  unsigned live_total=0,moved=0,work=0,checksum_total=0;uint64_t start=ns(),end;
  for(round=0;round<PROBE_ROUNDS;round++){
   Cvm vm;struct or_vm owner;struct cvm_value value,field;struct orv_stats stats;unsigned i,live=0;
   cvm_init(&vm,1000,1000,100);orv_init(&owner,&vm);
   for(i=0;i<count;i++){REQUIRE(cvm_pair(&vm,cvm_int((int32_t)i),cvm_int((int32_t)(100+i)),&value)==CVM_OK);if(i&1)REQUIRE(cvm_root_push(&vm,value)==CVM_OK);}
   if(mode){REQUIRE(orv_collect(&owner,1000,0,&stats)==OR_OK);moved+=stats.moved;work+=stats.work;}
   else REQUIRE(cvm_collect(&vm)==CVM_OK);
   for(i=0;i<16;i++)live+=vm.heap[i].used!=0;
   REQUIRE(live==count/2);live_total+=live;
   for(i=0;i<vm.root_count;i++){REQUIRE(cvm_field(&vm,vm.roots[i],0,&field)==CVM_OK&&field.integer==(int32_t)(2*i+1));checksum_total+=(unsigned)field.integer;}
  }
  end=ns();REQUIRE(start&&end>=start);checksum+=checksum_total;
  printf("VM %u %u %u %llu %u %u %u %u\n",count,mode,trial,(unsigned long long)(end-start),live_total,moved,work,checksum_total);
 }
 return checksum==0;
}
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--report', type=Path, default=ROOT/'reviews/v5/expansion/modern_completion/object_runtime_vm_probe.json')
    args = parser.parse_args()
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    build = ROOT/'build/modern_completion'/('object-vm-'+stamp)
    logs = ROOT/'reviews/v5/expansion/modern_completion/runs'/('object-vm-'+stamp)
    build.mkdir(parents=True); logs.mkdir(parents=True)
    source = build/'probe.c'; source.write_text(PROBE, encoding='utf8')
    config = build/'probe_config.h'; config.write_text('#define PROBE_ROUNDS 1000\n', encoding='utf8')
    paths = ['tools/run_object_runtime_vm_probe.py','labs/object_runtime/include/object_runtime.h','labs/object_runtime/include/object_runtime_vm.h','labs/object_runtime/VM_ADAPTER.md','assessor/object_runtime/object_runtime.c','assessor/object_runtime/object_runtime_vm.c','labs/runtime/include/cvm.h','assessor/runtime/cvm.c','labs/algorithms/include/algo.h','assessor/algorithms/algo.c']
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
                units = [source, ROOT/'assessor/object_runtime/object_runtime.c', ROOT/'assessor/object_runtime/object_runtime_vm.c', ROOT/'assessor/runtime/cvm.c', ROOT/'assessor/algorithms/algo.c']
                (project/'CMakeLists.txt').write_text(
                    'cmake_minimum_required(VERSION 3.20)\nproject(rc_probe C)\nset(CMAKE_C_STANDARD 17)\nset(CMAKE_C_EXTENSIONS OFF)\nfind_package(Threads REQUIRED)\nadd_executable(probe '+
                    ' '.join('"'+str(x)+'"' for x in units)+')\ntarget_include_directories(probe PRIVATE "'+str(ROOT/'labs/object_runtime/include')+'" "'+str(ROOT/'labs/runtime/include')+'" "'+str(ROOT/'labs/algorithms/include')+'")\ntarget_link_libraries(probe PRIVATE Threads::Threads m)\ntarget_compile_options(probe PRIVATE '+opt+' -Wall -Wextra -Wpedantic -Werror -fstack-usage)\n', encoding='utf8')
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
                    elif fields[0] == 'VM':
                        count,mode,trial,elapsed,live,moved,work,checksum_total=values
                        assert (live,moved,work,checksum_total)==(500*count,500*count*mode,1000*(3*count+16)*mode,1000*(count//2)**2) and elapsed>0,line
                        (warmups if trial==0 else samples).append({'objects':count,'mode':['baseline-nonmoving','explicit-moving'][mode],'trial':trial,'elapsed_ns':elapsed,'lifecycles':1000,'live_after':live,'moved':moved,'semantic_work':work,'value_checksum':checksum_total})
                assert len(samples)==120 and len(warmups)==8 and layout
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
        'limits': ['720 retained samples plus48 discarded warmups;1000 complete heap constructions and collections/sample, four object counts and two collector policies.',
                  'Equal admitted pair graphs, explicit odd-index roots, exact surviving values and counts; moving locations differ by design. Timing includes construction, root admission, collection and value checking.',
                  'Original CVM remains unchanged. Semantic work unit is specified in VM_ADAPTER.md and is not an instruction count; baseline budget has a different internal charge model.',
                  'Single-thread stopped-state measurement; no asynchronous collector, physical cache events or learner assessment.',
                  'Whole-image size and individual compiler stack frames do not by themselves establish a whole-call-chain stack bound.']}


    if not passed:
        result['error'] = locals().get('failure', 'source changed')
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(result, indent=2)+'\n', encoding='utf8')
    print(json.dumps({'passed': passed, 'report': str(args.report)}))
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
