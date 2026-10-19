"""Bind exact original I/O objectives to additive hosted/ABI successor evidence."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4]
BASE=Path(__file__).resolve().parent
CONTRACT='labs/posix_lifecycle_depth/io/CONTRACTS.md'
PRIVATE='assessor/posix_lifecycle_depth/io/io.c'
TEST='labs/posix_lifecycle_depth/io/tests.c'
REPORT='reviews/v5/expansion/legacy_posix_io/io_validation.json'
SUPPORT='research/v5/expansion/legacy_posix_io/support.json'
SCOPES={
 'T23.04.a':'stream: exact seven-byte script, budgeted EINTR, EAGAIN retained suffix, actual nonblocking empty/full/drained pipes, and hard sink failure after five committed bytes with stable pending ownership.',
 'T23.04.b':'stream plus separately executed launcher exact-child wait; close_policy executes released-number reuse and retained-on-EINTR transition models; inspected connect/SO_ERROR discussion distinguishes readiness from completion. No alternative-OS kernel execution or direct inaccessible standard inspection.',
 'T23.07.a':'extent: guard start/exclusive-end representability and selected file extent before casts, including zero and extrema; actual native off_t64, i386 off_t32 and i386 large-file off_t64 builds regenerate boundaries.',
 'T23.07.b':'sparse: actual0/1MiB data, zero hole, shared dup offset versus pread, explicit-zero rewrite with allocation observations, and5GiB logical extent under native and i386 large-file ABIs.',
 'T23.07.c':'position: two actual workers write64 disjoint records, strict O_APPEND exclusion, short positioned write plus EAGAIN progress/EINTR, deterministic shared lseek interference, and mixed overlapping record repaired by cooperative process record-lock publication.'}
def write(path,value):
 if path.name=='support.json':
  import runpy
  value=runpy.run_path(str(ROOT/'research/v5/expansion/legacy_posix_followthrough/variant_support.py'))['expand'](value,ROOT)
 path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!='PR-T23-02':return record
 ids=[]
 for criterion in record['criteria']:
  if criterion['objective'] in SCOPES:criterion['successor_support']={'path':SUPPORT,'id':criterion['id']};ids.append(criterion['id'])
 additions=record.setdefault('additional_successor_support',[])
 updated={'path':SUPPORT,'project':record['project'],'criterion_ids':ids,'scope':'Five I/O/positioned-record criteria; existing namespace and separate request/persistence successors retain their own scopes.'}
 for i,row in enumerate(additions):
  if row.get('path')==SUPPORT:additions[i]=updated;break
 else:additions.append(updated)
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for name in ('io.h','io_stub.c','tests.c','TOOLCHAIN.md'):
  path='labs/posix_lifecycle_depth/io/'+name
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def integrate_family():
 path=ROOT/'labs/posix_lifecycle_depth/family.json';data=json.loads(path.read_text(encoding='utf-8'))
 tests=['posix_lifecycle_depth.io_'+case for case in ('stream','extent','position','close_policy')]
 row=next(r for r in data['projects'] if r['id']=='PR-T23-02')
 for test in tests:
  if test not in row['tests']:row['tests'].append(test)
  if test not in data['tests']:data['tests'].append(test)
 row['scope']='Namespace/metadata/limited authority models plus resumable bytes, checked offsets, positioned records and explicit close ownership models; supplemental receipts cover platform/changed-ABI investigations.'
 campaigns=data.setdefault('supplemental_campaigns',[])
 updated={'id':'io-followthrough','producer':'tools/run_posix_io.py','report':REPORT,'contract':CONTRACT,'scope':'Five original criteria: native hosted mechanisms and actual i386 off_t32/large-file ABI; private sysroot explicit, no host install.'}
 for i,row in enumerate(campaigns):
  if row.get('id')==updated['id']:campaigns[i]=updated;break
 else:campaigns.append(updated)
 write(path,data)
def main():
 rows=[('PREAD','pread(2)','https://man7.org/linux/man-pages/man2/pread.2.html',
   ['DESCRIPTION unchanged shared offset, lines38-46','RETURN VALUE short success and errors, lines49-61','O_APPEND Linux/POSIX difference, lines90-96'],
   'Positioned record progress and rejection of Linux append descriptors; no multi-call record atomicity.'),
  ('CLOSE','close(2)','https://man7.org/linux/man-pages/man2/close.2.html',
   ['Release, record locks and last open reference, lines27-40','Early release and unsafe close retry, lines112-135','EINTR and POSIX2008/2024 versus Linux policy, lines140-162'],
   'Separate returned error from descriptor ownership; actual number reuse and declared alternative transition model.'),
  ('CONNECT','connect(2)','https://man7.org/linux/man-pages/man2/connect.2.html',
   ['Nonblocking progress, readiness and SO_ERROR, lines93-123','Failed socket state and portable recreation guidance, lines158-162'],
   'Reasoning comparison only: do not import copy/close retry rules into a connection state machine.')]
 resources=[]
 for ident,title,url,locators,scope in rows:
  resources.append({'id':'R-LIO-'+ident,'title':'Linux '+title+', man-pages6.19','authors':['Linux man-pages project'],'url':url,'locators':locators,
   'domains':['T23'],'kind':'manual','edition':'Linux man-pages6.19','access':'Actual bounded public passages inspected','role':'oracle','assignable':True,
   'reason':scope,'prerequisites':['C17 signed representability, cursor invariants and descriptor ownership'],
   'inspection':{'date':'2026-10-17','method':'Read actual opened primary Linux manual passages before successor implementation.','scope':scope,'limits':'Not full standards inspection, other-OS execution, socket implementation or physical durability.','evidence_urls':[url]}})
 shared=[{'id':'R-LPOSIX-'+k,'path':'research/v5/expansion/legacy_posix_followthrough/resources.json'} for k in ('READ','WRITE','WAIT')]
 shared += [{'id':'R-LMAP-LOCK','path':'research/v5/expansion/legacy_posix_mapping/resources.json'}]
 source=BASE/'resources.json';write(source,{'schema_version':1,'resources':resources,'canonical_external_sources':shared,
  'reference_gaps':[{'id':'LIO-POSIX2024-CLOSE','url':'https://pubs.opengroup.org/onlinepubs/9799919799/functions/close.html','status':'unavailable','observed':'HTTP403 from actual open attempt; not inspected. Edition comparison is attributed only to Linux close(2) lines140-162.'}]})
 pins={source.relative_to(ROOT).as_posix():hashlib.sha256(source.read_bytes()).hexdigest()}
 pins.update({r['path']:hashlib.sha256((ROOT/r['path']).read_bytes()).hexdigest() for r in shared})
 write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T23'],'inspection_kind':'Author inspection with an explicit direct-standard gap; not external review','research_input_sha256':pins})
 report=json.loads((ROOT/REPORT).read_text(encoding='utf-8'))
 if not report.get('passed') or len(report.get('profiles',[]))!=8:raise ValueError('I/O campaign must pass six native and two actual i386 profiles')
 if not all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in report['inspected_paths'].items()):raise ValueError('Stale I/O source')
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T23-02.json';record=attach(json.loads(path.read_text(encoding='utf-8')));write(path,record)
 updates=[]
 for criterion in record['criteria']:
  if criterion['objective'] not in SCOPES:continue
  scope=SCOPES[criterion['objective']];evidence=[REPORT]
  if criterion['objective']=='T23.04.b':evidence += ['reviews/v5/expansion/legacy_posix_followthrough/launch_validation.json','research/v5/expansion/legacy_posix_followthrough/support.json']
  updates.append({'id':criterion['id'],'original_source':path.relative_to(ROOT).as_posix(),'original_objective':criterion['objective'],'project':record['project'],
   'dimensions':{'teaching':{'state':'complete-with-direct-standard-reference-gap','evidence':[path.relative_to(ROOT).as_posix(),CONTRACT]},'implementation':{'state':'complete-for-declared-reference-contract','evidence':[PRIVATE,TEST]},
    'author_validation':{'state':'passed-with-stated-scope','evidence':evidence},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-applicable-to-declared-hosted-contract','evidence':[]}},
   'mechanism_scope':scope,'closure_rationale':scope,'variant_ids':[v['id'] for v in criterion['variants']],
   'variant_readiness':'Named changed-condition reference/platform/model support supplied; no learner delay or independent review fabricated.',
   'required_environment':'Owned Linux files/pipes/children, GCC/Clang native plus explicit private i386 sysroot/loader.','external_blockers':['Actual learner attempt and external review.'],
   'evidence_limits':['Bounded hosted scripts/processes and two ABI contracts; no arbitrary device-error recovery, durable commit or alternative-OS kernel execution.']})
 original=next(g for g in json.loads((ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json').read_text(encoding='utf-8'))['gaps'] if g['id']=='LRUN-04')
 updates.append({'id':'LRUN-04','original_source':'research/v5/expansion/legacy_runtime/gap_ledger.json','original_record':original,
  'dimensions':{'teaching':{'state':'complete-with-stated-reference-gap','evidence':[CONTRACT]},'implementation':{'state':'complete-for-enumerated-author-experiments','evidence':[PRIVATE,TEST]},'author_validation':{'state':'passed-with-stated-scope','evidence':[REPORT]},
   'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-applicable-to-hosted-contract','evidence':[]}},
  'closure_rationale':'Scripted short transfer/EINTR/EAGAIN, actual nonblocking pipe resumption, checked off_t extrema, actual off_t32/64 builds and bounded sparse large-file experiments supplied without replacing learner stubs.',
  'required_environment':'Local Linux and explicit hosted native/i386 toolchain.','external_blockers':['Learner-owned implementation and independent delayed attempt/review.'],
  'evidence_limits':['Reference execution only; original gap request to execute learner code remains learner-unattempted.','Direct POSIX2024 close page unavailable; Linux manual edition comparison is explicitly attributed.']})
 pins={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in [CONTRACT,PRIVATE,TEST,REPORT,path.relative_to(ROOT).as_posix(),source.relative_to(ROOT).as_posix()]}
 write(BASE/'support.json',{'schema_version':1,'required':True,'scope':'Five original I/O criteria and LRUN-04 successors; preserve namespace support and original criterion bodies.','source_sha256':pins,'updates':updates,
  'remaining_author_work':['Direct POSIX2024 close passage inspection remains unavailable.','T23.04.c request/effect deduplication requires its separately bound service successor; no inference from byte I/O.','T23.09.b equivalent-contract mapping versus positioned-I/O workload remains required.']})
 integrate_family();print(json.dumps({'resources':len(resources),'criteria':len(SCOPES),'original_gap':'LRUN-04','passed_campaign':True}))
if __name__=='__main__':main()
