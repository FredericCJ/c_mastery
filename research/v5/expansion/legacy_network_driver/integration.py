"""Read verified actual-driver child closure; never infer it from an authored label."""
from pathlib import Path
import hashlib,json,functools
ROOT=Path(__file__).resolve().parents[4]
SUPPORT='research/v5/expansion/legacy_network_driver/integration_completion_dispositions.json'
@functools.lru_cache(None)
def digest(p,size,mtime):return hashlib.sha256(p.read_bytes()).hexdigest()
def checked(path,h):
 p=ROOT/path;t=p.stat()
 if digest(p,t.st_size,t.st_mtime_ns)!=h:raise ValueError('stale actual driver evidence '+path)
def read():
 d=json.loads((ROOT/SUPPORT).read_text(encoding='utf8'))
 for p,h in d['source_sha256'].items():
  checked(p,h)
 for path in ['research/v5/expansion/legacy_network_driver/completion_dispositions.json','reviews/v5/expansion/legacy_network_driver/current_validation.json']:
  nested=json.loads((ROOT/path).read_text(encoding='utf8'))
  for key in ['source_sha256','input_sha256','outputs_sha256']:
   for p,h in nested.get(key,{}).items():checked(p,h)
 expected={'NW-ARP-DRIVER','NW-IP-DRIVER','NW-TCP-DRIVER'}
 if {r['id'] for r in d['updates']}!=expected:raise ValueError('driver child identities')
 for r in d['updates']:
  if r['dimensions']['implementation']['state']!='complete' or r['dimensions']['author_validation']['state']!='passed' or r['remaining_author_work']:raise ValueError('driver child incomplete')
  for k,state in [('physical_observation','not-run'),('external_review','not-run'),('independent_learner','not-assessed')]:
   if r['dimensions'][k]['state']!=state or r['dimensions'][k]['evidence']:raise ValueError('driver external evidence invented')
 return d
def apply(rows,files,child):
 d=read();r=next(r for r in d['updates'] if r['id']==child)
 files.update([ROOT/SUPPORT,Path(__file__),*[ROOT/p for p in d['source_sha256']]])
 for row in rows:
  row['driver_integration']=dict(id=child,support=SUPPORT,evidence=r['evidence'],artifacts=r['artifacts'])
  row['remaining_author_work']=[v for v in row['remaining_author_work'] if v!=child]
  if not row['remaining_author_work']:row['dimensions']['implementation']['state']='complete'
  row['dimensions']['implementation']['evidence']=list(dict.fromkeys(row['dimensions']['implementation']['evidence']+r['artifacts']))
  row['dimensions']['author_validation']['evidence']=list(dict.fromkeys(row['dimensions']['author_validation']['evidence']+r['evidence']))
  row['closure_rationale']+=' Actual driver successor closes '+child+'; original core campaign scope/limitations remain historical to that run.'

def validate(rows,child):
 """Require the actual verified closure, including its precise evidence links."""
 errors=[]
 try:d=read();record=next(r for r in d['updates'] if r['id']==child)
 except (OSError,ValueError,KeyError,StopIteration) as e:return [str(e)]
 expected=dict(id=child,support=SUPPORT,evidence=record['evidence'],artifacts=record['artifacts'])
 for row in rows:
  if row.get('driver_integration')!=expected:errors.append('missing actual driver integration '+row.get('id',''))
  if child in row.get('remaining_author_work',[]):errors.append('stale driver gap '+row.get('id',''))
  for key,items in [('implementation',record['artifacts']),('author_validation',record['evidence'])]:
   if not all(p in row.get('dimensions',{}).get(key,{}).get('evidence',[]) for p in items):errors.append('driver evidence omitted '+row.get('id',''))
 return errors

CONGESTION='research/v5/expansion/legacy_network_congestion/completion_dispositions.json'
CONGESTION_GUARD='reviews/v5/expansion/legacy_network_congestion/package_validation.json'
def congestion():
 d=json.loads((ROOT/CONGESTION).read_text(encoding='utf8'))
 for p,h in d['source_sha256'].items():checked(p,h)
 for p in [*d['current_reports'],CONGESTION_GUARD]:
  report=json.loads((ROOT/p).read_text(encoding='utf8'))
  if report.get('passed') is not True:raise ValueError('unpassed congestion evidence '+p)
  for key in ['input_sha256','outputs_sha256']:
   for path,h in report.get(key,{}).items():checked(path,h)
 r=next(r for r in d['updates'] if r['id']=='NW-CORPUS-CONGESTION')
 if r['remaining_author_work'] or r['dimensions']['implementation']['state']!='complete' or r['dimensions']['author_validation']['state']!='passed':raise ValueError('congestion incomplete')
 return d,r

def apply_congestion(rows,files):
 d,r=congestion()
 files.update([ROOT/CONGESTION,ROOT/CONGESTION_GUARD,Path(__file__),*[ROOT/p for p in d['source_sha256']]])
 for row in rows:
  if row.get('objective')!='T41.07.c':continue
  row['congestion_refinement']=dict(id=r['id'],support=CONGESTION,evidence=d['current_reports'])
  row['remaining_author_work']=[v for v in row['remaining_author_work'] if v!=r['id']]
  if not row['remaining_author_work']:row['dimensions']['implementation']['state']='complete'
  for key in ['teaching','implementation','author_validation']:
   row['dimensions'][key]['evidence']=list(dict.fromkeys(row['dimensions'][key]['evidence']+r['dimensions'][key]['evidence']))
  row['closure_rationale']+=' Required X-T41-003 congestion refinement is implemented and executed under its own bounded contract; it does not turn the one-flight predecessor into general TCP.'

def validate_congestion(rows):
 try:d,r=congestion()
 except (OSError,ValueError,KeyError,StopIteration) as e:return [str(e)]
 expected=dict(id=r['id'],support=CONGESTION,evidence=d['current_reports']);errors=[]
 for row in rows:
  if row.get('objective')!='T41.07.c':continue
  if row.get('congestion_refinement')!=expected:errors.append('missing congestion refinement '+row.get('id',''))
  if r['id'] in row.get('remaining_author_work',[]):errors.append('stale congestion gap')
  for key in ['teaching','implementation','author_validation']:
   if not all(p in row['dimensions'][key]['evidence'] for p in r['dimensions'][key]['evidence']):errors.append('congestion evidence omitted')
 return errors
