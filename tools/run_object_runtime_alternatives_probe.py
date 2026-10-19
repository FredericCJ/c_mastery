"""Measure equal-contract prototype lookup paths and build costs; no target execution."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import time

ROOT=Path(__file__).resolve().parents[1]
PROBE=r'''
#define _POSIX_C_SOURCE 200809L
#include "object_runtime_alternatives.h"
#include "probe_config.h"
#include <stdio.h>
#include <time.h>
static volatile uint64_t checksum;
static uint64_t ns(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))return 0;return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;}
#define REQUIRE(c) do{if(!(c))return 1;}while(0)
int main(void){unsigned width,mode,trial,i,k;
 printf("LAYOUT %zu %zu %zu %zu\n",sizeof(ORDictionary),sizeof(ORShapes),sizeof(ORObject),sizeof(ORCache));
 for(width=1;width<=3;width++)for(mode=0;mode<2;mode++)for(trial=0;trial<16;trial++){
  ORShapes shapes;ORObject objects[3]={{0}};ORCache cache={0};uint64_t start,end,sum=0;unsigned misses=0;
  or_shapes_init(&shapes);
  for(i=0;i<3;i++){if(i)REQUIRE(or_property_set(&shapes,&objects[i],(uint8_t)(i+1),-1)==OR_OK);REQUIRE(or_property_set(&shapes,&objects[i],1,(int32_t)(i*10+1))==OR_OK);if(!i)REQUIRE(or_property_set(&shapes,&objects[i],2,-1)==OR_OK);}
  start=ns();
  for(k=0;k<PROBE_ROUNDS;k++){
   ORObject *object=&objects[k%width];uint8_t key=(uint8_t)(k%17?1:4);int32_t out=777;ORStatus status;
   if(k%13==0)REQUIRE(or_property_set(&shapes,object,1,(int32_t)(100+k))==OR_OK);
   status=mode?or_property_get(&shapes,object,key,&cache,&out):or_property_generic(&shapes,object,key,&out);
   REQUIRE(status==(key==4?OR_MISS:OR_OK));sum+=(uint32_t)out;misses+=status==OR_MISS;
  }
  end=ns();REQUIRE(start&&end>=start);checksum+=sum;
  printf("LOOKUP %u %u %u %llu %llu %u %u %u %u\n",width,mode,trial,(unsigned long long)(end-start),(unsigned long long)sum,misses,cache.hits,cache.misses,cache.mega);
 }
 return checksum==0;
}
'''


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--report',type=Path,default=ROOT/'reviews/v5/expansion/modern_completion/object_runtime_alternatives_probe.json');args=parser.parse_args()
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    build=ROOT/'build/modern_completion'/('object-alternatives-'+stamp);build.mkdir(parents=True)
    logs=ROOT/'reviews/v5/expansion/modern_completion/runs'/('object-alternatives-'+stamp);logs.mkdir(parents=True)
    source=build/'probe.c';source.write_text(PROBE,encoding='utf8')
    config=build/'probe_config.h';config.write_text('#define PROBE_ROUNDS 10000\n',encoding='utf8')
    paths=['tools/run_object_runtime_alternatives_probe.py','labs/object_runtime/include/object_runtime.h','labs/object_runtime/include/object_runtime_alternatives.h','labs/object_runtime/ALTERNATIVES.md','assessor/object_runtime/object_runtime.c','assessor/object_runtime/object_runtime_alternatives.c']
    digest=lambda p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
    pins={p:digest(p) for p in paths};executions=[];observations=[];passed=False
    def run(cmd,label):
        start=time.monotonic();r=subprocess.run(list(map(str,cmd)),cwd=build,capture_output=True,text=True,timeout=90);text=r.stdout+r.stderr
        p=logs/(f'{len(executions):03}-{label}.log');p.write_text(text,encoding='utf8')
        executions.append({'command':list(map(str,cmd)),'cwd':str(build),'exit_code':r.returncode,'seconds':time.monotonic()-start,'stdout':text,'log':p.relative_to(ROOT).as_posix(),'log_sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
        if r.returncode:raise RuntimeError(text[-1500:])
        return text
    try:
        for compiler in ['gcc','clang']:
            run([compiler,'--version'],compiler+'-version')
            for opt in ['-O0','-O2','-O3']:
                tag=compiler+opt;exe=build/tag
                project=build/tag;project.mkdir();cmake=project/'CMakeLists.txt'
                units=[source,ROOT/'assessor/object_runtime/object_runtime.c',ROOT/'assessor/object_runtime/object_runtime_alternatives.c']
                cmake.write_text('cmake_minimum_required(VERSION 3.20)\nproject(shape_probe C)\nset(CMAKE_C_STANDARD 17)\nset(CMAKE_C_EXTENSIONS OFF)\nadd_executable(probe '+ ' '.join('"'+str(x)+'"' for x in units)+')\ntarget_include_directories(probe PRIVATE "'+str(ROOT/'labs/object_runtime/include')+'" "'+str(ROOT/'labs/ownership/include')+'")\ntarget_compile_options(probe PRIVATE '+opt+' -Wall -Wextra -Wpedantic -Werror -fstack-usage)\n',encoding='utf8')
                output=project/'out';run(['cmake','-S',project,'-B',output,'-G','Ninja','-DCMAKE_C_COMPILER='+compiler],tag+'-configure')
                clean_index=len(executions);run(['cmake','--build',output,'--parallel','4'],tag+'-clean-build')
                nochange_index=len(executions);nochange=run(['cmake','--build',output,'--parallel','4'],tag+'-unchanged-build');assert 'no work to do' in nochange,nochange
                config.write_text('#define PROBE_ROUNDS 10000\n/* rebuild '+tag+' */\n',encoding='utf8')
                changed_index=len(executions);changed=run(['cmake','--build',output,'--parallel','4'],tag+'-one-header-build');assert 'probe.c.o' in changed,changed
                exe=output/'probe';raw=run([exe],tag+'-execute');layout=None;samples=[];warmup=[]
                for line in raw.splitlines():
                    f=line.split();nums=list(map(int,f[1:]))
                    if f[0]=='LAYOUT':layout=nums
                    elif f[0]=='LOOKUP':
                        width,mode,trial,elapsed,total,missing,hits,misses,mega=nums
                        values=[1,11,21];wanted=0;miss_count=0;cache=[];chits=cmisses=cmega=0
                        for k in range(10000):
                            object=k%width;key=1 if k%17 else 4
                            if k%13==0:values[object]=100+k
                            wanted+=values[object] if key==1 else 777;miss_count+=key==4
                            if mode:
                                if not cmega and (object,key) in cache:chits+=1
                                else:
                                    cmisses+=1
                                    if key==1 and not cmega:
                                        if len(cache)==2:cmega=1
                                        else:cache.append((object,key))
                        assert (total,missing,hits,misses,mega)==(wanted,miss_count,chits,cmisses,cmega) and elapsed>0,line
                        (warmup if trial==0 else samples).append({'receiver_layouts':width,'cached':bool(mode),'trial':trial,'elapsed_ns':elapsed,'operations':10000,'checksum':total,'missing':missing,'hits':hits,'misses':misses,'sticky_generic':bool(mega)})

                assert len(samples)==90 and len(warmup)==6 and layout
                size=run(['size',exe],tag+'-size');run(['objdump','-d',exe],tag+'-assembly')
                stack=[]
                for index,p in enumerate(sorted(output.rglob('*.su'))):
                    q=logs/(tag+'-'+str(index)+'.su');q.write_bytes(p.read_bytes());stack.append({'path':q.relative_to(ROOT).as_posix(),'sha256':hashlib.sha256(q.read_bytes()).hexdigest()})
                observations.append({'compiler':compiler,'optimization':opt,'layout':layout,'samples':samples,'discarded_first_samples':warmup,'image_size':size,'stack_artifacts':stack,'build_execution_indices':{'clean':clean_index,'unchanged':nochange_index,'one_header_changed':changed_index}})
        passed=all(digest(p)==v for p,v in pins.items())
    except Exception as error:
        failure=str(error)
    result={'schema_version':1,'status':'passed' if passed else 'failed','passed':passed,'inspected_paths':pins,'scope_source_stable':all(digest(p)==v for p,v in pins.items()),'observations':observations,'executions':executions,'limits':['540 retained equal-contract samples plus36 discarded warmups;10000 direct/cached lookups each, one/two/three receiver layouts, missing every17th and value update every13th call.','Setup excluded; loop/timer/check/diagnostic overhead included. Direct path owns no cache. Same original object, current-layout and output contracts for both paths.','No arbitrary callback, JIT, hardware-counter or physical claim; actual clean/no-change/header builds and local stack/image records apply to the recorded host.']}
    if not passed:result['error']=locals().get('failure','source changed')
    args.report.parent.mkdir(parents=True,exist_ok=True);args.report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print(json.dumps({'passed':passed,'report':str(args.report)}));return 0 if passed else 1

if __name__=='__main__':raise SystemExit(main())
