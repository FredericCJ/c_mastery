"""Preserve original assessment observations and attach bounded executable successors."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4]
BASE=Path(__file__).resolve().parent
CONTRACT='labs/posix_lifecycle_depth/followthrough/CONTRACTS.md'
PRIVATE='assessor/posix_lifecycle_depth/followthrough/launch.c'
TEST='labs/posix_lifecycle_depth/followthrough/tests.c'
REPORT='reviews/v5/expansion/legacy_posix_followthrough/launch_validation.json'
SUPPORT='research/v5/expansion/legacy_posix_followthrough/support.json'
ROWS=[
 ('DUP','dup(2)','https://man7.org/linux/man-pages/man2/dup.2.html',['DESCRIPTION dup2 atomic replacement and equal descriptors, lines51-84','NOTES separately retained close-error handle, lines127-155'],'Atomic descriptor replacement; snapshots for a two-destination cycle are original course construction.'),
 ('PIPE','pipe(7)','https://man7.org/linux/man-pages/man7/pipe.7.html',['I/O on pipes: byte stream, EAGAIN and every-writer EOF, lines43-67'],'Reference ownership and explicit enrollment; no assumed message boundary.'),
 ('WAIT','wait(2)','https://man7.org/linux/man-pages/man2/waitpid.2.html',['RETURN VALUE and ERRORS, lines229-262','NOTES zombie and subreaper qualification, lines287-303'],'Exact child reaping, EINTR and no inference that every orphan always has PID1 as parent.'),
 ('SIGNAL','sigaction(2)','https://man7.org/linux/man-pages/man2/sigaction.2.html',['SA_RESTART, lines132-137','NOTES inherited and exec-reset dispositions, lines658-661'],'Process-local non-restarting test handler; no host policy changes.'),
 ('SAFE','signal-safety(7)','https://man7.org/linux/man-pages/man7/signal-safety.7.html',['DESCRIPTION state/reentrancy and required-function table, lines17-62','Table entries close,dup2,execve,_exit,fcntl,getpid,kill,read, lines81-155'],'Restricted child call audit, not a proof of every multithreaded launcher or libc implementation.'),
 ('READ','read(2)','https://man7.org/linux/man-pages/man2/read.2.html',['RETURN VALUE short success and EOF, lines43-56','ERRORS EINTR before any data, lines77-78'],'Separate accepted progress, EOF and interruption.'),
 ('WRITE','write(2)','https://man7.org/linux/man-pages/man2/write.2.html',['RETURN VALUE partial progress, lines52-66','ERRORS EINTR, lines105-106','NOTES before/after-progress interruption, lines157-160'],'Retry only the remaining suffix and do not equate transfer with durability.')]

def write(path,value):
 if path.name=='support.json':
  import runpy
  value=runpy.run_path(str(ROOT/'research/v5/expansion/legacy_posix_followthrough/variant_support.py'))['expand'](value,ROOT)
 path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 """Pure, idempotent canonical-renderer hook; no execution or report generation."""
 if record['project']!='PR-T23-01':return record
 ids=[]
 for criterion in record['criteria']:
  if criterion['objective'] in ('T23.02.a','T23.02.b','T23.03.b','T23.03.c'):
   criterion['successor_support']={'path':SUPPORT,'id':criterion['id']};ids.append(criterion['id'])
 record['successor_support']={'path':SUPPORT,'project':record['project'],'criterion_ids':ids,
   'scope':'Current explicit successor for these four criteria only; preserve original observations and all other criterion obligations.'}
 record['execution_support'].update({'observation_status':'historical-initial-authoring','observed_before_revision':'fd85773420a2fbb11003e83c8289417d62282ec4','active_successor':SUPPORT})
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for artifact in ('launch.h','launch_stub.c','tests.c','original_tests.c','io_inject.c','original_inject.c'):
  value='labs/posix_lifecycle_depth/followthrough/'+artifact
  if value not in record['supplied_artifacts']:record['supplied_artifacts'].append(value)
 return record
def main():
 resources=[]
 for ident,title,url,locators,scope in ROWS:
  resources.append({'id':'R-LPOSIX-'+ident,'title':'Linux '+title+', man-pages6.19','authors':['Linux man-pages project'],
   'url':url,'locators':locators,'domains':['T23'],'kind':'manual','edition':'Linux man-pages6.19','access':'Public bounded passages inspected',
   'role':'oracle','assignable':True,'reason':scope,'prerequisites':['C17 lifetimes and descriptor ownership'],
   'inspection':{'date':'2026-10-17','method':'Read actual opened primary manual text, including the listed function behavior and error cases.',
                 'scope':scope,'limits':'Bounded Linux manual passages; not direct inspection of the complete POSIX standard or every libc/kernel version.', 'evidence_urls':[url]}})
 source=BASE/'resources.json';write(source,{'schema_version':1,'resources':resources})
 write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T23'],'inspection_kind':'Author inspection, not external review',
       'research_input_sha256':{source.relative_to(ROOT).as_posix():hashlib.sha256(source.read_bytes()).hexdigest()}})
 report=json.loads((ROOT/REPORT).read_text(encoding='utf-8'))
 if not report.get('passed'):raise ValueError('Current launch campaign must pass')
 if not all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in report['inspected_paths'].items()):raise ValueError('Stale launch campaign')
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T23-01.json';record=json.loads(path.read_text(encoding='utf-8'))
 mapping={
  'T23.02.a':('original, channel, interrupts: exact-child wait with observed EINTR, legitimate127 versus ENOENT and self-SIGTERM.','partial-required-criterion','Arbitrary hard wait failure/launch cancellation recovery is outside the exclusive-owner precondition and still requires a distinct recovery protocol.'),
  'T23.02.b':('original native-int partial record plus stage-tagged eight-byte successor, unknown stages, one-byte transfers and premature death.','complete-for-declared-reference-contract','General cancellation/deadline protocols are not implied.'),
  'T23.03.b':('redirect: equal and crossed descriptors, both CLOEXEC policies, four acquisition/replacement failure positions and exact temporary cleanup.','complete-for-declared-reference-contract','Two destination replacements are not one atomic transaction; mixed setup failure must terminate the child.'),
  'T23.03.c':('inheritance: controlled original grandchild writer, ready-byte plus EOF enrollment, EAGAIN/EOF distinction, release and exact descendant reaping.','complete-for-declared-reference-contract','No unrelated-process descriptor enumeration or PID-order inference.')}
 updates=[]
 for criterion in record['criteria']:
  if criterion['objective'] not in mapping:continue
  scope,state,limit=mapping[criterion['objective']];ident=criterion['id']
  criterion['successor_support']={'path':SUPPORT,'id':ident}
  updates.append({'id':ident,'original_source':path.relative_to(ROOT).as_posix(),'original_objective':criterion['objective'],'project':record['project'],
   'dimensions':{'teaching':{'state':'complete','evidence':[path.relative_to(ROOT).as_posix(),CONTRACT]},
     'implementation':{'state':state,'evidence':[PRIVATE,TEST,'assessor/posix_lifecycle_depth/posix_lifecycle.c']},
     'author_validation':{'state':'passed-with-stated-scope','evidence':[REPORT]},'external_review':{'state':'not-run','evidence':[]},
     'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-applicable-to-hosted-process-contract','evidence':[]}},
   'mechanism_scope':scope,'closure_rationale':scope,'variant_ids':[v['id'] for v in criterion['variants']],
   'variant_readiness':'Original listed changed-condition reference support supplied; no elapsed learner attempt is manufactured.',
   'required_environment':'Unprivileged Linux original processes and synthetic owned pipes/files; GCC/Clang and libc.',
   'external_blockers':['Independent learner attempt and actual non-author review.'],'evidence_limits':[limit,'Finite tests are not absolute mastery or exhaustive scheduler coverage.']})
 attach(record)
 write(path,record)
 source_pins={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in [CONTRACT,PRIVATE,TEST,REPORT,'research/v5/expansion/legacy_posix_followthrough/resources.json',path.relative_to(ROOT).as_posix()]}
 gap=json.loads((ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json').read_text(encoding='utf-8'))
 original=next(g for g in gap['gaps'] if g['id']=='LRUN-03')
 gapupdate={'id':'LRUN-03','original_source':'research/v5/expansion/legacy_runtime/gap_ledger.json','original_record':original,
   'dimensions':{'teaching':{'state':'complete','evidence':[CONTRACT]},'implementation':{'state':'complete-for-enumerated-author-experiments','evidence':[PRIVATE,TEST]},
    'author_validation':{'state':'passed-with-stated-scope','evidence':[REPORT]},'external_review':{'state':'not-run','evidence':[]},
    'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-applicable-to-hosted-process-contract','evidence':[]}},
   'closure_rationale':'All five enumerated missing experiment mechanisms now have original executable reference support. The original requirement explicitly includes learner experiments, which remain unattempted.',
   'required_environment':'Unprivileged Linux owned teaching helpers only.','external_blockers':['Actual independent learner work and non-author review.'],
   'evidence_limits':['Does not close every T23 criterion or general launch cancellation/deadline recovery.']}
 write(BASE/'support.json',{'schema_version':1,'required':True,'scope':'Four criterion successors and one original named gap; no baseline or original gap rewrite.',
       'source_sha256':source_pins,'updates':updates+[gapupdate],'remaining_author_work':['T23.02.a arbitrary hard wait failure/recovery and T23.02.c posix_spawn comparison are not implemented by this bounded package.']})
 family=ROOT/'labs/posix_lifecycle_depth/family.json';data=json.loads(family.read_text(encoding='utf-8'))
 campaigns=data.setdefault('supplemental_campaigns',[]);campaigns[:]=[r for r in campaigns if r.get('id')!='launch-followthrough']
 campaigns.append({'id':'launch-followthrough','producer':'tools/run_posix_followthrough.py','report':REPORT,'contract':CONTRACT,
                   'scope':'Original and tagged failure channels, real/injected EINTR, descriptor cycles and grandchild writer ledgers.'});write(family,data)
 print(json.dumps({'resources':len(resources),'criteria':len(updates),'original_gap':'LRUN-03','passed_campaign':True}))
if __name__=='__main__':main()
