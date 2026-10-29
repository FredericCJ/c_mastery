"""Measure the declared runtime storage and pinning model; no target execution."""
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
#include "object_runtime_ext.h"
#include <stdio.h>
#include <time.h>
static volatile unsigned checksum;
static uint64_t ns(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))return 0;return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;}
#define REQUIRE(c) do{if(!(c))return 1;}while(0)
int main(void){unsigned mask,trial,k,i;
 printf("LAYOUT %zu %zu %zu %zu %zu %zu %zu %zu\n",sizeof(struct orx_value),sizeof(struct orx_handle),sizeof(struct orx_cell),sizeof(struct orx_entry),sizeof(struct orx_lease),sizeof(struct orx_heap),sizeof(struct orx_intern),sizeof(struct orx_ticket));
 for(k=1;k<=8;k++){
  struct orx_heap h;struct orx_intern table;struct ow_allocator a={0};struct orx_handle object;orx_heap_init(&h);REQUIRE(orx_intern_init(&table,&h,ow_allocator(&a))==OR_OK);
  for(i=0;i<k;i++){unsigned char value=(unsigned char)i;REQUIRE(orx_intern_get(&table,&value,1,&object)==OR_OK);}
  printf("ALLOC %u %u %zu %zu %zu %zu\n",k,table.capacity,a.attempts,a.live,a.peak,a.releases);REQUIRE(orx_intern_close(&table)==OR_OK&&a.live==0);
 }
 for(mask=0;mask<4;mask++)for(trial=0;trial<8;trial++){
  uint64_t elapsed=0;unsigned pins=mask==0?0:mask==1?2:mask==2?128:170;unsigned total=0;
  for(k=0;k<1000;k++){
   struct orx_heap h;struct orx_handle objects[8];struct orx_ticket tickets[4];const unsigned char *raw;size_t length;unsigned n,ticket_count=0;uint64_t start,end;
   orx_heap_init(&h);for(i=0;i<8;i++){unsigned char value=(unsigned char)i;REQUIRE(orx_new(&h,&value,1,&objects[i])==OR_OK);if(!(170u&(1u<<i)))REQUIRE(orx_release(&h,objects[i])==OR_OK);if(pins&(1u<<i))REQUIRE(orx_foreign_begin(&h,objects[i],1,&tickets[ticket_count++],&raw,&length)==OR_OK);}
   REQUIRE(orx_collect(&h,&n)==OR_OK&&n==4);start=ns();REQUIRE(orx_pack(&h,&n)==OR_OK);end=ns();REQUIRE(end>=start&&start);elapsed+=end-start;total+=n;checksum+=h.cell[h.entry[7].position].bytes[0];
   for(i=0;i<ticket_count;i++)REQUIRE(orx_foreign_finish(tickets[i],1)==OR_OK);
  }
  printf("PACK %u %u %llu %u\n",pins,trial,(unsigned long long)elapsed,total);
 }
 return checksum==0;
}
'''

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--report',type=Path,default=ROOT/'reviews/v5/expansion/modern_completion/object_runtime_storage_probe.json');args=parser.parse_args()
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    build=ROOT/'build/modern_completion'/('object-storage-'+stamp);build.mkdir(parents=True)
    logs=ROOT/'reviews/v5/expansion/modern_completion/runs'/('object-storage-'+stamp);logs.mkdir(parents=True)
    source=build/'probe.c';source.write_text(PROBE,encoding='utf8')
    paths=['tools/run_object_runtime_storage_probe.py','labs/object_runtime/include/object_runtime.h','labs/object_runtime/include/object_runtime_ext.h','labs/object_runtime/EXTENSIONS.md','assessor/object_runtime/object_runtime.c','assessor/object_runtime/object_runtime_ext.c','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','assessor/ownership/ownership.c']
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
                objects=[]
                for index,unit in enumerate([source,ROOT/'assessor/object_runtime/object_runtime.c',ROOT/'assessor/object_runtime/object_runtime_ext.c',ROOT/'assessor/ownership/ownership.c']):
                    obj=build/(tag+'-'+str(index)+'.o');objects.append(obj)
                    run([compiler,'-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fstack-usage','-I'+str(ROOT/'labs/object_runtime/include'),'-I'+str(ROOT/'labs/ownership/include'),'-c',unit,'-o',obj],tag+'-compile-'+str(index))
                run([compiler,*objects,'-o',exe],tag+'-link')
                raw=run([exe],tag+'-execute');layout=None;allocation=[];samples=[];warmup=[]
                for line in raw.splitlines():
                    f=line.split();nums=list(map(int,f[1:]))
                    if f[0]=='LAYOUT':layout=nums
                    elif f[0]=='ALLOC':
                        n,cap,attempts,live,peak,releases=nums;expected_cap=4 if n<=2 else 8 if n<=4 else 16;expected_attempts=1 if n<=2 else 2 if n<=4 else 3
                        assert (cap,attempts,live,peak,releases)==(expected_cap,expected_attempts,1,1 if n<=2 else 2,expected_attempts-1),line
                        allocation.append(nums)
                    elif f[0]=='PACK':
                        pin,trial,elapsed,moves=nums;kept=[1,3,5,7];reserved={x for x in kept if pin>>x&1};available=[x for x in range(8) if x not in reserved];unfixed=[x for x in kept if x not in reserved];expected_moves=sum(a!=b for a,b in zip(unfixed,available))
                        assert moves==1000*expected_moves and elapsed>0,line
                        (warmup if trial==0 else samples).append({'pins':pin,'trial':trial,'elapsed_ns':elapsed,'operations':1000,'moves':moves})
                assert len(samples)==28 and len(allocation)==8 and layout
                size=run(['size',exe],tag+'-size');run(['objdump','-d',exe],tag+'-assembly')
                stack=[]
                for p in sorted(build.glob(tag+'*.su')):
                    q=logs/p.name;q.write_bytes(p.read_bytes());stack.append({'path':q.relative_to(ROOT).as_posix(),'sha256':hashlib.sha256(q.read_bytes()).hexdigest()})
                observations.append({'compiler':compiler,'optimization':opt,'layout':layout,'allocation':allocation,'samples':samples,'discarded_first_samples':warmup,'image_size':size,'stack_artifacts':stack})
        passed=all(digest(p)==v for p,v in pins.items())
    except Exception as error:
        failure=str(error)
    result={'schema_version':1,'status':'passed' if passed else 'failed','passed':passed,'inspected_paths':pins,'scope_source_stable':all(digest(p)==v for p,v in pins.items()),'observations':observations,'executions':executions,'limits':['168 retained pack-time samples and24 discarded first samples; setup and pin completion excluded, clock-read overhead included.','No equivalent-policy speedup, worst-case latency, capability hardware or cache-traffic counter claim.','Physical layout and stack/image size apply only to these recorded host toolchains; dependent callback frames and allocator footprint are separate.']}
    if not passed:result['error']=locals().get('failure','source changed')
    args.report.parent.mkdir(parents=True,exist_ok=True);args.report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print(json.dumps({'passed':passed,'report':str(args.report)}));return 0 if passed else 1

if __name__=='__main__':raise SystemExit(main())
