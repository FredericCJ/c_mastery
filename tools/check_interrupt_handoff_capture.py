"""Validate supplied capture data; never manufacture physical observation."""
from pathlib import Path
import argparse,copy,hashlib,json,tempfile
def check(d,root):
 assert d['schema_version']==1 and d['status']=='captured' and d['origin'] in ['physical','emulator','synthetic-self-test']
 for k in ['board','part_revision','operator','observed_utc','compiler','instrument','clock_source']:assert isinstance(d[k],str) and d[k].strip(),k
 assert isinstance(d['command'],list) and d['command'] and all(isinstance(x,str) and x for x in d['command'])
 assert d['counter_bits']==32 and 0<d['whole_experiment_ticks']<2**32
 assert d['tick_ns_numerator']>0 and d['tick_ns_denominator']>0 and 1<=d['priority_bits']<=8 and 0<=d['grouping']<=7
 assert d['mask_restoration_observed'] is True and d['target_closure_checked'] is True
 assert isinstance(d['unexcluded_actors'],list) and d['unexcluded_actors'] and d['limitations']
 assert {'image','map','disassembly','raw-trace','clock-qualification','source-manifest'}<=set(d['artifacts'])
 for row in d['artifacts'].values():
  p=(root/row['path']).resolve();assert p.is_relative_to(root.resolve()) and p.is_file()
  assert hashlib.sha256(p.read_bytes()).hexdigest()==row['sha256']
 samples=d['samples'];assert len(samples)==80;seen=set();counts=[0,0];prior=[-1,-1]
 for row in samples:
  w,i=row['workload'],row['sequence'];assert w in [0,1] and 0<=i<40 and (w,i) not in seen;seen.add((w,i))
  values=[row[k] for k in ['entry','before','after','exit','consumed']];assert all(type(v) is int and 0<=v<2**32 for v in values)
  entry,before,after,exit,consumed=values;assert entry>prior[w] and exit>=entry;prior[w]=entry
  dropped=w==1 and i%5>=3;assert row['dropped'] is dropped
  if dropped:assert before==after==consumed==0
  else:assert entry<=before<=after<=exit<=consumed;counts[w]+=1
 assert counts==[40,24] and len(d['overhead_ticks'])==32 and all(type(v) is int and v>=0 for v in d['overhead_ticks'])
 return dict(capture_record_validated=True,claimed_origin=d['origin'],observed_by_checker=False,accepted=counts,limits='Artifact identity/record checks do not independently authenticate operator, clock or physical execution.')
def selftest():
 with tempfile.TemporaryDirectory(prefix='xi-capture-') as name:
  root=Path(name);p=root/'synthetic';p.write_text('Synthetic checker fixture, not a board capture.');digest=hashlib.sha256(p.read_bytes()).hexdigest()
  d=dict(schema_version=1,status='captured',origin='synthetic-self-test',command=['synthetic'],counter_bits=32,whole_experiment_ticks=1600000,tick_ns_numerator=40,tick_ns_denominator=1,priority_bits=4,grouping=3,mask_restoration_observed=True,target_closure_checked=True,unexcluded_actors=['NMI'],limitations=['synthetic'],artifacts={k:dict(path='synthetic',sha256=digest) for k in ['image','map','disassembly','raw-trace','clock-qualification','source-manifest']},samples=[],overhead_ticks=[0]*32)
  for k in ['board','part_revision','operator','observed_utc','compiler','instrument','clock_source']:d[k]='synthetic'
  for w in [0,1]:
   for i in range(40):
    entry=1000+i*10000;drop=w==1 and i%5>=3;d['samples'].append(dict(workload=w,sequence=i,dropped=drop,entry=entry,before=0 if drop else entry+1,after=0 if drop else entry+2,exit=entry+3,consumed=0 if drop else entry+4))
  result=check(d,root);assert result['observed_by_checker'] is False
  edits=[lambda x:x.update(status='not-run'),lambda x:x['samples'].pop(),lambda x:x['samples'][0].update(consumed=0),lambda x:x['samples'][43].update(dropped=False),lambda x:x['artifacts']['image'].update(sha256='0'*64),lambda x:x['artifacts']['image'].update(path='../synthetic'),lambda x:x.update(whole_experiment_ticks=2**32),lambda x:x.update(mask_restoration_observed=False),lambda x:x.update(priority_bits=0),lambda x:x.update(overhead_ticks=[])]
  for edit in edits:
   x=copy.deepcopy(d);edit(x)
   try:check(x,root)
   except (AssertionError,KeyError):continue
   raise AssertionError('invalid capture accepted')
  return dict(passed=True,negative_controls=len(edits),physical_observed=False)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('record',nargs='?');p.add_argument('--root',type=Path,default=Path.cwd());p.add_argument('--self-test',action='store_true');a=p.parse_args();print(json.dumps(selftest() if a.self_test else check(json.loads(Path(a.record).read_text()),a.root),indent=2))
