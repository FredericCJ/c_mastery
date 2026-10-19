"""Preserve original terminal criteria while binding exact executable successors."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4]
BASE=Path(__file__).resolve().parent
CONTRACT='labs/posix_lifecycle_depth/terminal/CONTRACTS.md'
REPORT='reviews/v5/expansion/legacy_posix_terminal/terminal_validation.json'
SUPPORT='research/v5/expansion/legacy_posix_terminal/support.json'
SCOPES={
 'T23.11.a':'Actual concurrent producer/copier/sink transfers four discovered pipe capacities plus17 bytes; six acquisition failures, early-third-stage closure, timeout cleanup and independent exact-child reap checks.',
 'T23.11.b':'Owned UNIX98 PTY canonical/raw A/newline/NUL delivery, actual attribute save/readback/restoration and injected partial-success/error paths; all four VMIN/VTIME cases with actual owned-child blocked-read enrollment.',
 'T23.11.c':'Owned child default SIGPIPE versus ignored-signal EPIPE, actual PTY disconnect EOF/EIO classification and failed-restoration retention, plus local socket half-close and remaining response.',
 'T24.10.a':'Owned PTY symbolic B9600/CS8/no parity/one stop-bit configuration and full named-field readback/restoration; no-first-byte versus interbyte timer observation. Physical baud/electrical evidence remains unobserved.',
 'T24.10.b':'One unchanged freestanding parser object reused by byte-array, actual PTY and local socket adapters across59 chunk bounds and malformed/valid corpus; mid-frame reconnect and queued-data discard policy, bounded queue backpressure and epoch exhaustion.'}
def write(path,value):
 if path.name=='support.json':
  import runpy
  value=runpy.run_path(str(ROOT/'research/v5/expansion/legacy_posix_followthrough/variant_support.py'))['expand'](value,ROOT)
 # Additive successors may use different JSON formatting. Preserve identical
 # canonical content bytes instead of invalidating unrelated current bindings.
 if path.parent.name=='legacy_assessments' and path.is_file() and json.loads(path.read_text(encoding='utf-8'))==value:return
 path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project'] not in ('PR-T23-04','PR-T24-06'):return record
 ids=[]
 for c in record['criteria']:
  if c['objective'] in SCOPES:c['successor_support']={'path':SUPPORT,'id':c['id']};ids.append(c['id'])
 additions=record.setdefault('additional_successor_support',[]);additions[:]=[r for r in additions if r.get('path')!=SUPPORT]
 additions.append({'path':SUPPORT,'project':record['project'],'criterion_ids':ids,'scope':'Exact hosted terminal/pipeline and explicit same-object parser criteria; no SocketCAN or physical serial/bus closure.'})
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for name in ('terminal.h','terminal_stub.c','tests.c','stream.h','stream_stub.c','stream_tests.c','run_stream.py'):
  path='labs/posix_lifecycle_depth/terminal/'+name
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record

def can_successor():
 path=ROOT/'research/v5/expansion/legacy_can/support.json'
 if not path.is_file():return None
 value=json.loads(path.read_text(encoding='utf-8'));pins=value.get('source_sha256',{})
 if not pins or not all((ROOT/p).is_file() and hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items()):return None
 expected={'AX-PR-T24-06-T24.10.c','AX-PR-T24-06-T24.10.c-DELAYED'}
 if {r['id'] for r in value.get('updates',[])}!=expected:return None
 return {'original_work':'T24.10.c SocketCAN/frame adapter and physical two-node transfer remain separately required.',
  'author_successor':path.relative_to(ROOT).as_posix(),
  'scope':'Frame adapter and complete bench preparation packet authored/verified; actual physical/device selection remains required.',
  'remaining_external':'research/v5/expansion/legacy_can/source_access.json#G-CAN-TWO-NODE-DEVICE'}
def integrate_family():
 path=ROOT/'labs/posix_lifecycle_depth/family.json';data=json.loads(path.read_text(encoding='utf-8'))
 data['title']='Hosted process, byte I/O, namespace, mapping and terminal lifecycle'
 for project,tests,scope in [('PR-T23-04',['terminal_guard','terminal_pipeline'],'Actual owned PTY guard and concurrent pipeline; supplied timing/disconnect investigations have a separate receipt.'),('PR-T24-06',['terminal_guard'],'Owned PTY configuration and restoration; explicit completed parser import uses terminal/run_stream.py, not an automatic assessor dependency.')]:
  names=['posix_lifecycle_depth.'+t for t in tests];row=next((r for r in data['projects'] if r['id']==project),None)
  if row is None:row={'id':project,'tests':[]};data['projects'].append(row)
  for name in names:
   if name not in row['tests']:row['tests'].append(name)
   if name not in data['tests']:data['tests'].append(name)
  row['scope']=scope;assessment='curriculum/v5/expansion/legacy_assessments/'+project+'.json'
  if assessment not in data['assessment_contracts']:data['assessment_contracts'].append(assessment)
 campaigns=data.setdefault('supplemental_campaigns',[])
 new={'id':'terminal-followthrough','producer':'tools/run_posix_terminal.py','report':REPORT,'contract':CONTRACT,'scope':'Five original criteria; six hosted profiles, actual owned PTYs/pipelines, explicit same parser object over three transports. No physical serial/bus or learner claim.'}
 for i,row in enumerate(campaigns):
  if row.get('id')==new['id']:campaigns[i]=new;break
 else:campaigns.append(new)
 write(path,data)
def main():
 rows=[('TERMIOS','Linux termios(3), man-pages6.19','https://man7.org/linux/man-pages/man3/termios.3.html',
  ['Retrieving/changing attributes and action timing, lines391-416','Canonical and four noncanonical VMIN/VTIME cases, lines418-500','Raw flags and symbolic line-speed state, lines502-514 and560-577','Partial successful tcsetattr and readback, lines650-664'],
  'Actual bounded manual passages establish selected line-discipline and readback contracts; not physical serial execution.'),
  ('PTY','Linux pty(7), man-pages6.19','https://man7.org/linux/man-pages/man7/pty.7.html',
   ['Virtual master/slave channel and asynchronous data flow, lines17-40','UNIX98 allocation/grant/unlock/slave open, lines57-64'],
   'Owned software terminal construction and asynchronous delivery; no electrical or operational terminal target.'),
  ('PIPE','Linux pipe(7), man-pages6.19','https://man7.org/linux/man-pages/man7/pipe.7.html',
   ['Blocking, stream boundaries, writer-reference EOF and SIGPIPE/EPIPE, lines43-67','Capacity variation and concurrent consumption, lines71-88'],
   'Full-pipeline ownership and capacity-sensitive deadlock controls; no fixed capacity assumption.'),
  ('SHUTDOWN','Linux shutdown(2), man-pages6.19','https://man7.org/linux/man-pages/man2/shutdown.2.html',
   ['Partial full-duplex shutdown, return values and errors, lines25-47'],
   'Local socket half-close separately from both-direction closure.'),
  ('PARSER','Original checkpoint parser implementation and public header','labs/checkpoint_depth/include/checkpoint_depth.h',
   ['Public CPParser/CPQueue ownership and API, header lines1-23','cp_feed/inspect/cp_deliver/cp_take in assessor/checkpoint_depth/checkpoint_depth.c'],
   'Actual course source inspected for unchanged-object reuse, suffix consumption and queue refusal; original contract rather than third-party authority.')]
 resources=[]
 for ident,title,url,locators,scope in rows:
  resources.append({'id':'R-LTERM-'+ident,'title':title,'authors':['Linux man-pages project' if ident!='PARSER' else 'C Mastery authors'],'url':url,'locators':locators,'domains':['T23','T24'],'kind':'manual' if ident!='PARSER' else 'course','edition':'Linux man-pages6.19' if ident!='PARSER' else 'C17 course source','access':'Actual bounded passages inspected','role':'oracle','assignable':ident!='PARSER','reason':scope,'prerequisites':['Descriptor ownership, parser queues and C17 object bounds'],
   'inspection':{'date':'2026-10-17','method':'Read actual primary manual passages or explicitly named course source before authoring successor.','scope':scope,'limits':'No whole-manual/standard inspection, external review, learner or physical result.','evidence_urls':[url]}})
 source=BASE/'resources.json';write(source,{'schema_version':1,'resources':resources})
 write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T23','T24'],'inspection_kind':'Author passage/source inspection, not independent external review','research_input_sha256':{source.relative_to(ROOT).as_posix():hashlib.sha256(source.read_bytes()).hexdigest()}})
 report=json.loads((ROOT/REPORT).read_text(encoding='utf-8'))
 if not report.get('passed') or len(report['profiles'])!=6 or sum(len(p['outcomes']) for p in report['profiles'])!=120:raise ValueError('Terminal campaign incomplete')
 if not all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in report['inspected_paths'].items()):raise ValueError('Stale terminal source')
 updates=[];canonical=[]
 for project in ('PR-T23-04','PR-T24-06'):
  path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');record=attach(json.loads(path.read_text(encoding='utf-8')));write(path,record);canonical.append(path.relative_to(ROOT).as_posix())
  for c in record['criteria']:
   if c['objective'] not in SCOPES:continue
   stream=c['objective']=='T24.10.b';scope=SCOPES[c['objective']];implementation='assessor/posix_lifecycle_depth/terminal/'+('stream.c' if stream else 'terminal.c');tests='labs/posix_lifecycle_depth/terminal/'+('stream_tests.c' if stream else 'tests.c')
   updates.append({'id':c['id'],'original_source':canonical[-1],'original_objective':c['objective'],'project':project,
    'dimensions':{'teaching':{'state':'complete-for-declared-hosted-contract','evidence':[canonical[-1],CONTRACT]},'implementation':{'state':'complete-for-declared-reference-and-variant-contract','evidence':[implementation,tests]},'author_validation':{'state':'passed-with-stated-scope','evidence':[REPORT]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-required-for-physical-extension' if c['objective']=='T24.10.a' else 'not-applicable-to-declared-hosted-contract','evidence':[]}},
    'mechanism_scope':scope,'closure_rationale':scope,'variant_ids':[v['id'] for v in c['variants']],'variant_readiness':'Exact named delayed variant reference support executed; actual delayed unaided learner attempt remains unassessed.',
    'required_environment':'Linux owned PTYs/pipes/socketpairs and child-only proc syscall access; explicit completed parser source/header import for .10.b.','external_blockers':['External review and delayed learner assessment.']+(['Dedicated physical serial evidence if claiming baud/electrical behavior.'] if c['objective']=='T24.10.a' else []),
    'evidence_limits':['Finite author experiments; no physical baud/electrical/CAN guarantee or all-schedule proof.','Parser closure audit uses an uninstrumented object; hosted sanitizer results do not instrument its internals.']})
 original=next(r for r in json.loads((ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json').read_text(encoding='utf-8'))['gaps'] if r['id']=='LRUN-07')
 updates.append({'id':'LRUN-07','original_source':'research/v5/expansion/legacy_runtime/gap_ledger.json','original_record':original,
  'dimensions':{'teaching':{'state':'complete-for-hosted-author-experiments','evidence':[CONTRACT]},'implementation':{'state':'complete-for-hosted-author-experiments','evidence':['assessor/posix_lifecycle_depth/terminal/terminal.c','assessor/posix_lifecycle_depth/terminal/stream.c']},'author_validation':{'state':'passed-with-stated-scope','evidence':[REPORT]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-required-for-physical-extension','evidence':[]}},
  'closure_rationale':'Full concurrent pipeline, actual four-case PTY timing/restoration/deadlines and named hosted transfer variants have executable reference support; retain separate physical and learner obligations.',
  'required_environment':'Owned local Linux processes/PTYS/socketpairs; no host security changes.','external_blockers':['Learner and external review.','Dedicated hardware needed for real serial electrical/timing/turnaround claims.'],'evidence_limits':['No SocketCAN/frame-adapter or physical two-node evidence is inferred.']})
 pins={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in [CONTRACT,REPORT,source.relative_to(ROOT).as_posix(),*canonical]}
 write(BASE/'support.json',{'schema_version':1,'required':True,'scope':'Five exact terminal/pipeline/parser criteria and LRUN-07; original content retained.','source_sha256':pins,'updates':updates,'remaining_author_work':[] if can_successor() else ['T24.10.c SocketCAN/frame adapter and physical two-node transfer remain separately required.'],'superseded_remaining_author_work':[can_successor()] if can_successor() else [],'remaining_external_work':['Independent delayed learner attempts and external review.','Physical baud/electrical/RS485 evidence requires dedicated equipment; no current host-setting changes or hardware interaction authorized.']})
 integrate_family();print(json.dumps({'criteria':len(SCOPES),'gap':'LRUN-07','campaign_outcomes':120}))
if __name__=='__main__':main()
