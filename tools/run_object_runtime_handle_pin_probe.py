"""Compare a rooted copied handle with a pinned raw span across actual packing."""
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
#include "object_runtime_ext.h"
#include "probe_config.h"
#include <stdio.h>
#include <time.h>
static volatile uint64_t checksum;
static uint64_t ns(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))return 0;return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;}
#define REQUIRE(c) do{if(!(c))return 1;}while(0)
int main(void){unsigned mode,length,read_case,trial,round;
 printf("LAYOUT %zu %zu %zu %zu\n",sizeof(struct orx_handle),sizeof(struct orx_ticket),sizeof(struct orx_heap),sizeof(struct orx_cell));
 for(mode=0;mode<2;mode++)for(length=1;length<=15;length+=14)for(read_case=0;read_case<3;read_case++)for(trial=0;trial<16;trial++){
  unsigned reads=read_case==0?1:read_case==1?64:256,moves=0;uint64_t total=0,start=ns(),end;
  for(round=0;round<PROBE_ROUNDS;round++){
   struct orx_heap h;struct orx_handle objects[8],job;struct orx_ticket ticket;const unsigned char *raw=NULL;size_t size=0;unsigned i,j,k,n;unsigned char payload[15],buffer[15];
   orx_heap_init(&h);
   for(i=0;i<8;i++){for(j=0;j<length;j++)payload[j]=(unsigned char)(i+1);REQUIRE(orx_new(&h,payload,length,&objects[i])==OR_OK);}
   job=objects[7];REQUIRE(orx_retain(&h,job)==OR_OK&&orx_release(&h,objects[7])==OR_OK);
   if(mode)REQUIRE(orx_foreign_begin(&h,job,1,&ticket,&raw,&size)==OR_OK&&size==length);
   for(i=0;i<8;i+=2)REQUIRE(orx_release(&h,objects[i])==OR_OK);
   REQUIRE(orx_collect(&h,&n)==OR_OK&&n==4);REQUIRE(orx_pack(&h,&n)==OR_OK);moves+=n;
   for(k=0;k<reads;k++){
    if(!mode){REQUIRE(orx_read(&h,job,buffer,&size)==OR_OK&&size==length);raw=buffer;}
    for(j=0;j<length;j++)total+=raw[j];
   }
   /* Synchronous consumer has returned: no backend has a possible later access. */
   if(mode)REQUIRE(orx_foreign_finish(ticket,1)==OR_OK);
   REQUIRE(orx_release(&h,job)==OR_OK);
   for(i=1;i<7;i+=2)REQUIRE(orx_release(&h,objects[i])==OR_OK);
   REQUIRE(orx_collect(&h,&n)==OR_OK&&n==4);
  }
  end=ns();REQUIRE(start&&end>=start);checksum+=total;
  printf("ACCESS %u %u %u %u %llu %llu %u\n",mode,length,reads,trial,(unsigned long long)(end-start),(unsigned long long)total,moves);
 }
 return checksum==0;
}
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--report', type=Path, default=ROOT/'reviews/v5/expansion/modern_completion/object_runtime_handle_pin_probe.json')
    args = parser.parse_args()
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    build = ROOT/'build/modern_completion'/('object-handle-pin-'+stamp)
    logs = ROOT/'reviews/v5/expansion/modern_completion/runs'/('object-handle-pin-'+stamp)
    build.mkdir(parents=True); logs.mkdir(parents=True)
    source = build/'probe.c'; source.write_text(PROBE, encoding='utf8')
    config = build/'probe_config.h'; config.write_text('#define PROBE_ROUNDS 1000\n', encoding='utf8')
    paths = ['tools/run_object_runtime_handle_pin_probe.py','labs/object_runtime/include/object_runtime.h','labs/object_runtime/include/object_runtime_ext.h','labs/object_runtime/EXTENSIONS.md','assessor/object_runtime/object_runtime.c','assessor/object_runtime/object_runtime_ext.c','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','assessor/ownership/ownership.c']
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
                units = [source, ROOT/'assessor/object_runtime/object_runtime.c', ROOT/'assessor/object_runtime/object_runtime_ext.c', ROOT/'assessor/ownership/ownership.c']
                (project/'CMakeLists.txt').write_text(
                    'cmake_minimum_required(VERSION 3.20)\nproject(handle_pin_probe C)\nset(CMAKE_C_STANDARD 17)\nset(CMAKE_C_EXTENSIONS OFF)\nfind_package(Threads REQUIRED)\nadd_executable(probe '+
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
                    elif fields[0] == 'ACCESS':
                        mode,length,reads,trial,elapsed,total,moves=values
                        assert (total,moves)==(1000*8*length*reads,1000*(3 if mode else 4)) and elapsed>0,line
                        (warmups if trial==0 else samples).append({'mode':'pinned-span' if mode else 'copied-rooted-handle','length':length,'reads_per_job':reads,'trial':trial,'elapsed_ns':elapsed,'jobs':1000,'checksum':total,'moved_cells':moves,'access_payload_bytes_copied':0 if mode else 1000*length*reads})
                assert len(samples)==180 and len(warmups)==12 and layout
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
        'limits': ['1080 retained samples plus72 discarded warmups;1000 complete lifetime/packing/consumer/cleanup jobs each. Same immutable payload, explicit root ownership and final result; copied handles permit movement, pinning restricts one survivor location.',
                  'Synchronous consumer only; finish occurs after all reads. Actual asynchronous cancellation/join correctness remains separately exercised by the family foreign-backend campaign. No unjoined worker or concurrent metadata-read permission is inferred.',
                  'Setup, rooting, pool collection, packing, read validation/copies, pin accounting and final cleanup included. No required check is removed to improve timing. This is a policy/work comparison, not a claim of equal relocation freedom.',
                  'Whole-image size and compiler stack frames are host-specific. No target/physical, universal speedup or cache-counter claim.']}
    if not passed:
        result['error'] = locals().get('failure', 'source changed')
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(result, indent=2)+'\n', encoding='utf8')
    print(json.dumps({'passed': passed, 'report': str(args.report)}))
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
