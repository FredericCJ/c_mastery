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
#include "object_runtime_shapes.h"
#include "probe_config.h"
#include <stdio.h>
#include <time.h>
static volatile uint64_t checksum;
static uint64_t ns(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))return 0;return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;}
#define REQUIRE(c) do{if(!(c))return 1;}while(0)
int main(void){unsigned depth_index,mode,trial,i,k;unsigned depths[]={1,4,8};
 printf("LAYOUT %zu %zu %zu %zu %zu %zu\n",sizeof(ORXWorld),sizeof(ORXDescriptor),sizeof(ORXObject),sizeof(ORXField),sizeof(ORXPropertyCache),_Alignof(ORXPropertyCache));
 for(depth_index=0;depth_index<3;depth_index++)for(mode=0;mode<2;mode++)for(trial=0;trial<16;trial++){
  ORXWorld w;ORXHandle object[8];ORXPropertyCache cache={0};unsigned depth=depths[depth_index],dead;int32_t value;uint64_t start,end,sum=0,hits=0,misses=0;
  orx_world_init(&w);for(i=0;i<depth;i++)REQUIRE(orx_world_new(&w,&object[i])==OR_OK);
  REQUIRE(orx_define(&w,object[depth-1],7,0,10,NULL,NULL)==OR_OK);
  for(i=0;i+1<depth;i++)REQUIRE(orx_prototype(&w,object[i],object[i+1])==OR_OK);
  start=ns();
  for(k=0;k<PROBE_ROUNDS;k++){
   if(k%16==0)REQUIRE(orx_define(&w,object[depth-1],7,0,(int32_t)(10+k/16),NULL,NULL)==OR_OK);
   if(mode==0)cache=(ORXPropertyCache){0};
   REQUIRE(orx_get(&w,object[0],7,&cache,&value)==OR_OK);sum+=(uint32_t)value;
   if(mode==0){hits+=cache.hits;misses+=cache.misses;}
  }
  end=ns();REQUIRE(start&&end>=start);
  if(mode==1){hits=cache.hits;misses=cache.misses;}
  checksum+=sum;
  printf("LOOKUP %u %u %u %llu %llu %llu %llu\n",depth,mode,trial,(unsigned long long)(end-start),(unsigned long long)sum,(unsigned long long)hits,(unsigned long long)misses);
  for(i=0;i<depth;i++)REQUIRE(orx_release(&w.heap,object[i])==OR_OK);
  REQUIRE(orx_world_collect(&w,&dead)==OR_OK&&dead==depth);
 }
 return checksum==0;
}
'''


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--report',type=Path,default=ROOT/'reviews/v5/expansion/modern_completion/object_runtime_shapes_probe.json');args=parser.parse_args()
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    build=ROOT/'build/modern_completion'/('object-shapes-'+stamp);build.mkdir(parents=True)
    logs=ROOT/'reviews/v5/expansion/modern_completion/runs'/('object-shapes-'+stamp);logs.mkdir(parents=True)
    source=build/'probe.c';source.write_text(PROBE,encoding='utf8')
    config=build/'probe_config.h';config.write_text('#define PROBE_ROUNDS 10000\n',encoding='utf8')
    paths=['tools/run_object_runtime_shapes_probe.py','labs/object_runtime/include/object_runtime.h','labs/object_runtime/include/object_runtime_ext.h','labs/object_runtime/include/object_runtime_shapes.h','labs/object_runtime/SHAPES_EXTENSIONS.md','assessor/object_runtime/object_runtime.c','assessor/object_runtime/object_runtime_ext.c','assessor/object_runtime/object_runtime_shapes.c','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','assessor/ownership/ownership.c']
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
                units=[source,ROOT/'assessor/object_runtime/object_runtime.c',ROOT/'assessor/object_runtime/object_runtime_ext.c',ROOT/'assessor/object_runtime/object_runtime_shapes.c',ROOT/'assessor/ownership/ownership.c']
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
                        depth,mode,trial,elapsed,total,hits,misses=nums
                        assert total==sum(10+k//16 for k in range(10000)) and elapsed>0,line
                        assert (hits,misses)==((9999,1) if mode else (0,10000)),line
                        (warmup if trial==0 else samples).append({'chain_objects':depth,'retain_cache':bool(mode),'trial':trial,'elapsed_ns':elapsed,'operations':10000,'checksum':total,'hits':hits,'misses':misses})
                assert len(samples)==90 and len(warmup)==6 and layout
                size=run(['size',exe],tag+'-size');run(['objdump','-d',exe],tag+'-assembly')
                stack=[]
                for index,p in enumerate(sorted(output.rglob('*.su'))):
                    q=logs/(tag+'-'+str(index)+'.su');q.write_bytes(p.read_bytes());stack.append({'path':q.relative_to(ROOT).as_posix(),'sha256':hashlib.sha256(q.read_bytes()).hexdigest()})
                observations.append({'compiler':compiler,'optimization':opt,'layout':layout,'samples':samples,'discarded_first_samples':warmup,'image_size':size,'stack_artifacts':stack,'build_execution_indices':{'clean':clean_index,'unchanged':nochange_index,'one_header_changed':changed_index}})
        passed=all(digest(p)==v for p,v in pins.items())
    except Exception as error:
        failure=str(error)
    result={'schema_version':1,'status':'passed' if passed else 'failed','passed':passed,'inspected_paths':pins,'scope_source_stable':all(digest(p)==v for p,v in pins.items()),'observations':observations,'executions':executions,'limits':['540 retained lookup samples and36 discarded first samples: each10000 calls plus625 same-layout field updates, with depths1/4/8 and reset or retained cache. Setup and root cleanup excluded; clock/loop/counter/reset overhead included.','The same typed lookup function is used with either a reset cache or retained cache; this is not a pure dictionary, JIT or JavaScript engine performance comparison.','No hardware cache-counter, worst-case latency, arbitrary callback cost or physical target claim. Clean, unchanged and single-header incremental commands are actually timed, with this host filesystem/toolchain only.']}
    if not passed:result['error']=locals().get('failure','source changed')
    args.report.parent.mkdir(parents=True,exist_ok=True);args.report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print(json.dumps({'passed':passed,'report':str(args.report)}));return 0 if passed else 1

if __name__=='__main__':raise SystemExit(main())
