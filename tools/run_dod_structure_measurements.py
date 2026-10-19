"""Bounded entity-layout workloads and equal-RC-contract reclamation measurements."""
from datetime import datetime, timezone
from pathlib import Path
import argparse, hashlib, json, statistics, subprocess
ROOT=Path(__file__).resolve().parents[1]
SOURCES=['labs/dod/tests/structure_measure.c','assessor/dod/dod.c','assessor/dod/dod_ext.c','assessor/dod/dod_entities.c','assessor/dod/dod_persistent.c']
PATHS=SOURCES+['labs/dod/include/'+s for s in ['dod.h','dod_ext.h','dod_entities.h','dod_persistent.h']]+['tools/run_dod_structure_measurements.py']
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def logical(edits,ratio):
 masks=[1,2];total=0
 for i in range(edits):
  masks[i%2]=(i//2)%3+1;total+=ratio*sum(2*id+3 for id,m in enumerate(masks) if m==3)
 return total
def copy_model(sparse):
 masks=[1,2];tables=[[0],[1],[]];components=[[0],[1]];writes=0
 for step in range(12):
  id=step%2;mask=(step//2)%3+1;old=masks[id]
  if not sparse:
   writes+=2 if mask==old else 6
   if mask!=old:tables[old-1].remove(id);tables[mask-1].append(id)
  else:
   for bit,items in enumerate(components):
    if mask&(1<<bit):
     if id not in items:items.append(id);writes+=1
     writes+=1
    elif id in items:
     at=items.index(id)
     if at!=len(items)-1:items[at]=items[-1];writes+=2
     items.pop()
  masks[id]=mask
 return writes
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--report',required=True);args=parser.parse_args();report=(ROOT/args.report).resolve();assert report.is_relative_to(ROOT/'reviews/v5/expansion')
 stamp='dod-structures-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs=report.parent/'runs'/stamp;logs.mkdir(parents=True);build=ROOT/'build/modern_completion'/stamp;build.mkdir(parents=True)
 pins={p:sha(ROOT/p) for p in PATHS};executions=[];measurements=[];observations=[];stack=[]
 def execute(command,label):
  r=subprocess.run(list(map(str,command)),cwd=ROOT,text=True,capture_output=True,timeout=180);p=logs/(label+'.log');p.write_text(r.stdout+'\n'+r.stderr,encoding='utf8');executions.append({'command':list(map(str,command)),'exit_code':r.returncode,'log':p.relative_to(ROOT).as_posix(),'log_sha256':sha(p)});assert r.returncode==0,(label,r.stderr);return r.stdout
 body=(ROOT/'assessor/dod/dod.c').read_text();statements=['to->ids[row]=id;','to->position[row]=position;','to->velocity[row]=velocity;','from->ids[r]=moved;','from->position[r]=from->position[last];','from->velocity[r]=from->velocity[last];']
 for statement in statements:
  assert statement in body;left=statement.split('=')[0];body=body.replace(statement,'{'+statement+'d_structure_copy(sizeof('+left+'));}')
 instrumented=build/'observed-dod.c';instrumented.write_text('#include <stddef.h>\nextern void d_structure_copy(size_t);\n'+body)
 for compiler in ['gcc','clang']:
  execute([compiler,'--version'],compiler+'-version');folder=build/compiler;folder.mkdir();objects=[]
  flags=['-std=c17','-O2','-g','-Wall','-Wextra','-Wpedantic','-Werror','-fno-lto','-fstack-usage','-Ilabs/dod/include']
  for source in SOURCES:
   obj=folder/(Path(source).stem+'.o');execute([compiler,*flags,'-c',source,'-o',obj],compiler+'-'+Path(source).stem);objects.append(obj)
  raw=folder/'raw';observed=folder/'observed';execute([compiler,*objects,'-o',raw],compiler+'-link');execute([compiler,*flags,'-DD_STRUCTURE_OBSERVE',*[instrumented if s=='assessor/dod/dod.c' else s for s in SOURCES],'-o',observed],compiler+'-observed-build')
  rows=[s.split() for s in execute([observed,'observe'],compiler+'-observations').splitlines()];assert len(rows)==5
  for row in rows[:4]:
   mode,ratio,edits,sample,elapsed,checksum,copies,misses=map(int,row[1:]);assert checksum==logical(edits,ratio);assert copies==copy_model(mode)*4;assert misses==(0 if mode else edits)
  assert list(map(int,rows[-1][1:4]))==[32,32,15]
  observations.append({'compiler':compiler,'rows':rows,'limits':'Payload+ID field stores at listed source statements only; full-world staging and metadata copies excluded. sizeof(unsigned)==sizeof(int)==4 is required by this observed counter model.'})
  grouped={}
  for row in [s.split() for s in execute([raw,'time'],compiler+'-timings').splitlines()]:
   if row[0]=='E':
    mode,ratio,edits,sample,elapsed,checksum,copies,misses=map(int,row[1:]);assert checksum==logical(edits,ratio) and copies==0;key=('entity',mode,ratio);assert misses==(0 if mode else edits)
   else:
    budget,sample,elapsed,calls,reclaimed,drops,pending=map(int,row[1:]);assert reclaimed==15000 and drops==8000;assert calls==1000*(1 if budget==0 else (15+budget-1)//budget);assert pending<=4;key=('reclaim',budget)
   grouped.setdefault(key,[]).append(elapsed)
  assert len(grouped)==10
  for key,values in grouped.items():
   assert len(values)==7;measurements.append({'compiler':compiler,'workload':key,'samples_ns':values,'median_ns':statistics.median(values),'min_ns':min(values),'max_ns':max(values)})
  execute(['size',raw],compiler+'-size');execute(['objdump','-d',raw],compiler+'-assembly')
  for p in sorted(folder.glob('*.su')):
   target=logs/(compiler+'-'+p.name);target.write_bytes(p.read_bytes());stack.append({'path':target.relative_to(ROOT).as_posix(),'sha256':sha(target)})
 stable=pins=={p:sha(ROOT/p) for p in PATHS};assert stable
 result={'schema_version':1,'passed':True,'status':'passed','inspected_paths':pins,'scope_source_stable':True,'executions':executions,'instrumentation':{'path':instrumented.relative_to(ROOT).as_posix(),'sha256':sha(instrumented),'statements':statements},'observations':observations,'measurements':measurements,'stack_usage':stack,'limits':['Entity comparison is a two-live-entity success workload, not equivalent unrestricted APIs. Both validate actual DSparse handles; the archetype adapter also stages a whole world and uses its actual versioned cache.','Sparse component sets are original benchmark code with dense values/inverse maps. The scalar dictionary oracle checks each whole workload checksum independently.','Exactly6000 mutations and1/16/256 queries per mutation; timings include state publication and query-cache maintenance. No deletion/reuse in this timing workload; those have separate semantic tests.','RC alternatives reclaim the same15-node/eight-owned-leaf tree;1000 reconstructions occur outside each timer, but clock overhead remains in the accumulated duration.','Region integer tree versus RC owned-text tree census is structural accounting, not a fair comparative cleanup latency benchmark.','Seven host trials, no affinity/frequency control, no universal performance or hard real-time guarantee. Static stack reports exclude runtime/caller stack and require interpretation.']}
 report.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':True,'timing_samples':140,'cells':20}))
if __name__=='__main__':main()
