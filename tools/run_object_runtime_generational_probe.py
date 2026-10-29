"""Measure preflight composition and card width/placement under explicit traffic models."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import shutil
import time

ROOT = Path(__file__).resolve().parents[1]
PROBE = r'''
#define _POSIX_C_SOURCE 200809L
#include "object_runtime_generational.h"
#include "object_runtime_card_epoch.h"
#include "probe_config.h"
#include <stdio.h>
#include <time.h>
static volatile uint64_t checksum;
static uint64_t ns(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))return 0;return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;}
#define REQUIRE(c) do{if(!(c))return 1;}while(0)
int main(void){unsigned mode,density,trial,round,width,padded,i;
 printf("LAYOUT %zu %zu %zu %zu %zu %zu\n",sizeof(struct or_nursery),sizeof(struct or_cards),sizeof(struct or_large_heap),sizeof(struct orc_cards),_Alignof(struct orc_cards),sizeof(struct orcc_meta));
 for(mode=0;mode<3;mode++)for(density=1;density<=4;density+=3)for(trial=0;trial<16;trial++){
  unsigned copied_total=0,fields=0,dead_total=0;uint64_t start=ns(),end;
  for(round=0;round<PROBE_ROUNDS;round++){
   struct or_nursery h={0};struct or_cards cards;unsigned n,scanned,stage;uint8_t f[16];
   h.used=4;h.old_used=4;for(i=0;i<4;i++){h.young[0][i].value=(int32_t)(i+1);h.old[i].value=(int32_t)(101+i);h.old[i].age=1;if(i<density)h.old[i].edge[0]=(uint8_t)(i+1);}
   REQUIRE(or_nursery_cards_init(&h,&cards)==OR_OK);
   for(stage=0;stage<2;stage++){
    if(mode==0){scanned=2u*h.old_used;REQUIRE(or_minor(&h,8,8,&n)==OR_OK);}else REQUIRE(or_minor_preflight(&h,mode==2?&cards:NULL,8,8,&n,&scanned)==OR_OK);
    REQUIRE(n==density);copied_total+=n;fields+=scanned;
   }
   REQUIRE(h.used==0&&h.old_used==4+density);
   REQUIRE(or_major_preflight(&h,NULL,&n,f)==OR_OK&&n==4+density&&h.used==0&&h.old_used==0);dead_total+=n;
  }
  end=ns();REQUIRE(start&&end>=start);checksum+=copied_total+dead_total;
  printf("GEN %u %u %u %llu %u %u %u\n",mode,density,trial,(unsigned long long)(end-start),copied_total,fields,dead_total);
 }
 for(width=1;width<=8;width*=2)for(padded=0;padded<2;padded++)for(trial=0;trial<16;trial++){
  struct orc_cards c;unsigned fields=0,transitions=0,lines=0;uintptr_t previous=0,first=0,second=0;uint64_t start,end;
  REQUIRE(orcc_init(&c,width,(int)padded)==OR_OK);start=ns();
  for(round=0;round<PROBE_ROUNDS;round++){
   unsigned object=round%2?7:0,card=object/width,scanned;uint16_t roots;struct orcc_snapshot snapshot;
   uintptr_t address=(uintptr_t)(padded?(void *)&c.metadata.padded[card].meta:(void *)&c.metadata.packed[card]);uintptr_t line=address/64;
   if(round&&previous!=line){transitions++;}
   previous=line;if(!round){first=line;lines=1;}else if(line!=first&&line!=second){second=line;lines++;}
   REQUIRE(orcc_store(&c,object,0,1)==OR_OK&&orcc_prepare(&c,card,&snapshot)==OR_OK);
   REQUIRE(orcc_scan_commit(&c,&snapshot,&roots,&scanned)==OR_OK&&roots==1);fields+=scanned;
   REQUIRE(orcc_store(&c,object,0,0)==OR_OK&&orcc_prepare(&c,card,&snapshot)==OR_OK);
   REQUIRE(orcc_scan_commit(&c,&snapshot,&roots,&scanned)==OR_OK&&roots==0);fields+=scanned;
  }
  end=ns();REQUIRE(start&&end>=start&&orcc_close(&c)==OR_OK);checksum+=fields;
  printf("CARD %u %u %u %llu %u %u %u\n",width,padded,trial,(unsigned long long)(end-start),fields,lines,transitions);
 }
 return checksum==0;
}
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--report', type=Path, default=ROOT/'reviews/v5/expansion/modern_completion/object_runtime_generational_probe.json')
    args = parser.parse_args()
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    build = ROOT/'build/modern_completion'/('object-generational-'+stamp)
    logs = ROOT/'reviews/v5/expansion/modern_completion/runs'/('object-generational-'+stamp)
    build.mkdir(parents=True); logs.mkdir(parents=True)
    source = build/'probe.c'; source.write_text(PROBE, encoding='utf8')
    config = build/'probe_config.h'; config.write_text('#define PROBE_ROUNDS 1000\n', encoding='utf8')
    paths = ['tools/run_object_runtime_generational_probe.py','labs/object_runtime/include/object_runtime.h','labs/object_runtime/include/object_runtime_generational.h','labs/object_runtime/include/object_runtime_card_epoch.h','labs/object_runtime/GENERATIONAL_EXTENSIONS.md','labs/object_runtime/CARD_EPOCH_EXTENSIONS.md','assessor/object_runtime/object_runtime.c','assessor/object_runtime/object_runtime_generational.c','assessor/object_runtime/object_runtime_card_epoch.c']
    digest = lambda p: hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
    pins = {p: digest(p) for p in paths}; executions = []; observations = []; hardware = []; passed = False

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
                units = [source, ROOT/'assessor/object_runtime/object_runtime.c', ROOT/'assessor/object_runtime/object_runtime_generational.c', ROOT/'assessor/object_runtime/object_runtime_card_epoch.c']
                (project/'CMakeLists.txt').write_text(
                    'cmake_minimum_required(VERSION 3.20)\nproject(gen_probe C)\nset(CMAKE_C_STANDARD 17)\nset(CMAKE_C_EXTENSIONS OFF)\nfind_package(Threads REQUIRED)\nadd_executable(probe '+
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
                    elif fields[0] == 'GEN':
                        mode,density,trial,elapsed,copied,scans,dead=values
                        assert (copied,scans,dead)==(2*density*1000,(8 if mode==2 and density==1 else 16)*1000,(4+density)*1000) and elapsed>0,line
                        (warmups if trial==0 else samples).append({'mechanism':'minor-major','mode':['whole-state','preflight-full','preflight-cards'][mode],'old_edge_density':density,'trial':trial,'elapsed_ns':elapsed,'rounds':1000,'copied':copied,'root_fields_scanned':scans,'reclaimed':dead})
                    elif fields[0] == 'CARD':
                        width,padded,trial,elapsed,scans,lines,transitions=values
                        assert (scans,lines,transitions)==(4*width*1000,2 if padded and width<8 else 1,999 if padded and width<8 else 0) and elapsed>0,line
                        (warmups if trial==0 else samples).append({'mechanism':'card-placement','width':width,'padded':bool(padded),'trial':trial,'elapsed_ns':elapsed,'rounds':1000,'fields_scanned':scans,'young_fields_found':1000,'false_positive_fields':scans-1000,'modeled_line_bytes':64,'unique_metadata_lines':lines,'metadata_line_transitions':transitions})
                assert len(samples) == 210 and len(warmups) == 14 and layout
                size = run(['size', exe], tag+'-size'); run(['objdump', '-d', exe], tag+'-assembly')
                if compiler == 'gcc' and opt == '-O2':
                    perf = shutil.which('perf')
                    if perf:
                        cmd = [perf, 'stat', '-e', 'cache-references,cache-misses', '--', str(exe)]
                        attempt = subprocess.run(cmd, capture_output=True, text=True, timeout=90)
                        p = logs/'hardware-counters.log'; p.write_text(attempt.stdout+attempt.stderr, encoding='utf8')
                        hardware.append({'command': cmd, 'exit_code': attempt.returncode,
                                         'status': 'observed' if attempt.returncode == 0 else 'unavailable',
                                         'log': p.relative_to(ROOT).as_posix(), 'sha256': hashlib.sha256(p.read_bytes()).hexdigest(),
                                         'scope': 'Whole probe process, not attributed per card/layout or corrected for timer/observer/other workloads.'})
                    else:
                        hardware.append({'status': 'unavailable', 'reason': 'perf executable absent from the current WSL PATH; no host settings or system packages changed.'})
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
        'scope_source_stable': all(digest(p) == v for p, v in pins.items()), 'observations': observations, 'executions': executions, 'hardware_counters': hardware,
        'limits': ['1260 retained samples plus84 discarded warmups;1000 complete equal-contract graph lifecycles or card store/scan/clear rounds per sample.',
                  'Minor comparisons retain initialization, validation, mirror reconstruction, promotion and major cleanup; original full-scan field counts are derived from the inspected loop, adapter counts are returned by its instrumented root-discovery stage. Neither count is total memory traffic.',
                  'Card lines are calculated from actual touched addresses using an explicit64-byte line model. These are not hardware cache misses, coherence transfers or false-sharing measurements. Timings include mutex operations, snapshot copying and checks; no parallel throughput inference.',
                  'No physical target claim. Whole-image size includes linked original runtime and both extensions; .su records are compiler frame observations, not a whole-program stack proof.']}
    if not passed:
        result['error'] = locals().get('failure', 'source changed')
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(result, indent=2)+'\n', encoding='utf8')
    print(json.dumps({'passed': passed, 'report': str(args.report)}))
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
