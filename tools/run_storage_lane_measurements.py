"""Separate copy observations and equal-value capture/lane host timing campaigns."""
from pathlib import Path
from datetime import datetime,timezone
import argparse,hashlib,json,statistics,subprocess
ROOT=Path(__file__).resolve().parents[1]
PATHS=['labs/dispatch/tests/storage_measure.c','assessor/dispatch/capture.c','assessor/dispatch/dispatch.c','assessor/dod/dod.c','assessor/dod/dod_ext.c','assessor/dod/dod_buffers.c','assessor/ownership/ownership.c','labs/dispatch/include/capture.h','labs/dispatch/include/dispatch.h','labs/dod/include/dod.h','labs/dod/include/dod_ext.h','labs/dod/include/dod_buffers.h','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','tools/run_storage_lane_measurements.py']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);args=parser.parse_args();report=(ROOT/args.report).resolve();assert report.is_relative_to(ROOT/'reviews/v5/expansion')
 stamp='storage-lanes-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs=report.parent/'runs'/stamp;logs.mkdir(parents=True);build=ROOT/'build/modern_completion'/stamp;build.mkdir(parents=True)
 pins={p:sha(ROOT/p) for p in PATHS};records=[];observations=[];measurements=[]
 def execute(command,label):
  p=subprocess.run(list(map(str,command)),cwd=ROOT,text=True,capture_output=True,timeout=180);log=logs/(label+'.log');log.write_text(p.stdout+'\n'+p.stderr,encoding='utf8');records.append({'command':list(map(str,command)),'exit_code':p.returncode,'log':log.relative_to(ROOT).as_posix(),'log_sha256':sha(log)});assert p.returncode==0,(label,p.stderr);return p.stdout
 copies={}
 small=(ROOT/'assessor/dod/dod.c').read_text();capture=(ROOT/'assessor/dispatch/capture.c').read_text()
 transforms={
 'dod.c':[(x,'{'+x+'d_copy_observe(1);}') for x in ['dst[i]=d_small_data(s)[i];','dst[s->length+i]=p[i];','s->local[i]=s->heap[i];','d->local[i]=s->local[i];']],
 'capture.c':[(x,'{'+x+'d_copy_observe('+n+');}') for x,n in [('bytes[4*i+b]=(unsigned char)(word>>(8*b));','1'),('c->data.local[i]=p[i];','sizeof(int32_t)'),('copy[i]=p[i];','sizeof(int32_t)')]]}
 for name,text in [('dod.c',small),('capture.c',capture)]:
  for old,new in transforms[name]:assert old in text,old;text=text.replace(old,new)
  text='#include <stddef.h>\nextern void d_copy_observe(size_t);\n'+text;path=build/('observed-'+name);path.write_text(text);copies[name]={'path':path.relative_to(ROOT).as_posix(),'sha256':sha(path),'statement_instrumentation':transforms[name]}
 expected=[]
 for compact in [0,1]:
  for cycles in [1,8,64]:expected.append(['O',compact,cycles,16+33*cycles if compact else 32+cycles,cycles if compact else 1,cycles if compact else 1,0 if compact else 1])
 for n in range(1,9):expected.append(['C',n,8*n,4*n,2*int(n>4),2*int(n>4)])
 pairs=[];random=7
 for _ in range(65536):
  random=(random*1664525+1013904223)&0xffffffff;left=random
  random=(random*1664525+1013904223)&0xffffffff;pairs.append((left,random))
 lane_expected={}
 for mode in range(3):
  bits=8 if mode==0 else 16;mask=(1<<bits)-1;total=0
  for index,(left,right) in enumerate(pairs,1):
   result=0
   for shift in range(0,32,bits):
    a=(left>>shift)&mask;b=(right>>shift)&mask
    if mode==2:
     a=a if a<32768 else a-65536;b=b if b<32768 else b-65536
     value=max(-32768,min(32767,a+b))&mask
    else:value=min(mask,a+b)
    result|=value<<shift
   total+=result
   if index in (1024,65536):lane_expected[mode,index]=total
 for compiler,optimization in [(c,o) for c in ['gcc','clang'] for o in ['O0','O2','O3']]:
  profile=compiler+'-'+optimization
  execute([compiler,'--version'],profile+'-version');flags=['-std=c17','-'+optimization,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-lto','-Ilabs/dispatch/include','-Ilabs/dod/include','-Ilabs/ownership/include'];sources=['labs/dispatch/tests/storage_measure.c','assessor/dispatch/dispatch.c','assessor/dod/dod_ext.c','assessor/dod/dod_buffers.c','assessor/ownership/ownership.c']
  raw=build/(profile+'-raw');observed=build/(profile+'-observed')
  execute([compiler,*flags,*sources,'assessor/dispatch/capture.c','assessor/dod/dod.c','-ldl','-o',raw],profile+'-raw-build')
  execute([compiler,*flags,'-DCOPY_OBSERVED=1',*sources,build/'observed-capture.c',build/'observed-dod.c','-ldl','-o',observed],profile+'-observed-build')
  text=execute([observed,'observe'],profile+'-observations');rows=[[s.split()[0],*map(int,s.split()[1:])] for s in text.splitlines()];assert rows[:-1]==expected,(compiler,rows,expected);assert rows[-1][0]=='L';observations.append({'compiler':compiler,'optimization':optimization,'payload_write_and_allocation_rows':rows,'model_matches':True})
  raw_rows=[[s.split()[0],*map(int,s.split()[1:])] for s in execute([raw,'observe'],profile+'-raw-observations').splitlines()]
  for actual,observed_row in zip(raw_rows[:-1],expected):
   want=observed_row[:]
   if want[0]=='O':want[3]=0
   else:want[2]=want[3]=0
   assert actual==want
  execute(['size',raw,observed],profile+'-size');execute(['objdump','-d',raw],profile+'-assembly')
  data=[s.split() for s in execute([raw,'time'],profile+'-timings').splitlines()];assert len(data)==252
  grouped={}
  for row in data:
   if row[0]=='T':
    n=int(row[1]);key=('capture',n,int(row[3]));value=int(row[4]);q,r=divmod(100000,17)
    assert int(row[5])==100000*sum(i-2 for i in range(n))+q*136+r*(r-1)//2
   else:
    key=('lanes',int(row[1]),int(row[2]),int(row[4]));value=int(row[5])
    assert int(row[6])==lane_expected[int(row[1]),int(row[2])]
   grouped.setdefault(key,[]).append(value)
  for key,values in grouped.items():assert len(values)==7;measurements.append({'compiler':compiler,'optimization':optimization,'workload':key,'samples_ns':values,'median_ns':statistics.median(values),'min_ns':min(values),'max_ns':max(values)})
 stable=pins=={p:sha(ROOT/p) for p in PATHS};assert stable
 result={'schema_version':1,'passed':True,'status':'passed','inspected_paths':pins,'scope_source_stable':stable,'instrumentation':copies,'observations':observations,'measurements':measurements,'executions':records,'limits':['Copy observations count explicit payload-byte writes at identified source statements, not all compiler-generated struct/metadata copies or hardware memory traffic.','Timing binaries omit the copy observer completely and reuse actual struct d_small, signed/unsigned lane kernels, typed capture and T66 counting allocator.','Timing captures only repeated invocation after construction; capture encoding/allocation costs are reported by operation observations, not folded into call latency.','Seven host samples per cell, no affinity/frequency control, no universal speed threshold or second ABI claim.','Scalar comparison uses the same saturation contract; its compiler may itself vectorize or synthesize packed operations.']};report.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':True,'timing_samples':sum(len(m['samples_ns']) for m in measurements),'cells':len(measurements),'observation_rows':len(expected)*6}))
if __name__=='__main__':main()
