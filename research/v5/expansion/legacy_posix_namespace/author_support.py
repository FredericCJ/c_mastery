"""Attach bounded namespace successor evidence without replacing original criteria."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4]
BASE=Path(__file__).resolve().parent
CONTRACT='labs/posix_lifecycle_depth/namespace/CONTRACTS.md'
PRIVATE='assessor/posix_lifecycle_depth/namespace/path.c'
TEST='labs/posix_lifecycle_depth/namespace/tests.c'
REPORT='reviews/v5/expansion/legacy_posix_namespace/namespace_validation.json'
SUPPORT='research/v5/expansion/legacy_posix_namespace/support.json'
SCOPES={
 'T23.05.a':'identity: original live handle across rename/recreate/unlink, hard-link equality and zero remaining links; independent inode/byte observations, not permanent identity after lifetime.',
 'T23.05.b':'anchored: relative open/create across cwd change and directory rename; absolute owned path ignores invalid dirfd; anchor is not confinement.',
 'T23.05.c':'confined: acknowledged child symlink replacement causes the original flawed reader to read OTHER; final-component repair fails on intermediate link; full openat2 policy rejects both and upward/absolute escape. Hard links remain accepted, proving no authenticity guarantee.',
 'T23.06.a':'metadata and metadata_tmpfs: raw named type/size/block/mode/owner/time fields, stat versus link metadata, sparse extension and requested timestamp fractions on two observed distinct filesystem types; no fixed allocation/resolution claim.',
 'T23.06.b':'policy: actual child027/0666/no-default-ACL0640; explicit three-entry default ACL model0755 with0666 yields0644 despite077 mask; injected EACCES is distinguished from host denial.',
 'T23.06.c':'policy: search-only actor denied read/write, parent write permits replacement independently of file write; inverse permissions and missing-search controls. Restricted serialized adapter, not a complete credential/ACL engine.'}

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
 record['successor_support']={'path':SUPPORT,'project':record['project'],'criterion_ids':ids,'scope':'Six namespace/metadata criteria; file-transfer and persistence criteria remain separately required.'}
 record['execution_support'].update({'observation_status':'historical-initial-authoring','observed_before_revision':'fd85773420a2fbb11003e83c8289417d62282ec4','active_successor':SUPPORT})
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for name in ('path.h','path_stub.c','tests.c'):
  path='labs/posix_lifecycle_depth/namespace/'+name
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record

def integrate_family():
 path=ROOT/'labs/posix_lifecycle_depth/family.json';data=json.loads(path.read_text(encoding='utf-8'))
 tests=['posix_lifecycle_depth.namespace_'+case for case in ('confined','metadata','policy')]
 row=next((p for p in data['projects'] if p['id']=='PR-T23-02'),None)
 if row is None:
  row={'id':'PR-T23-02','tests':[],'scope':'Full-resolution Linux regular-file open, named metadata and restricted creation/operation-authority models. Actual identity/anchor and changed-filesystem investigations use supplemental receipt.'};data['projects'].append(row)
 for test in tests:
  if test not in row['tests']:row['tests'].append(test)
 for test in tests:
  if test not in data['tests']:data['tests'].append(test)
 contract='curriculum/v5/expansion/legacy_assessments/PR-T23-02.json'
 if contract not in data['assessment_contracts']:data['assessment_contracts'].append(contract)
 campaigns=data.setdefault('supplemental_campaigns',[])
 campaign={'id':'namespace-followthrough','producer':'tools/run_posix_namespace.py','report':REPORT,'contract':CONTRACT,'scope':'Six original criteria, controlled synthetic symlink replacement, identity/rename/hard links, two filesystem observations, actual child umask and limited ACL/authority models.'}
 existing=next((i for i,r in enumerate(campaigns) if r.get('id')=='namespace-followthrough'),None)
 if existing is None:campaigns.append(campaign)
 else:campaigns[existing]=campaign
 write(path,data)

def main():
 rows=[('OPENAT2','openat2(2)','https://man7.org/linux/man-pages/man2/openat2.2.html',
   ['DESCRIPTION zero-filled open_how, lines46-61','RESOLVE_BENEATH and explicit magic-link policy, lines97-108','RESOLVE_NO_SYMLINKS versus final O_NOFOLLOW, lines170-199','EAGAIN, ELOOP and EXDEV resolution failures, lines247-274'],
   'Full-resolution no-symlink/beneath policy; propagate unsupported/unstable resolution rather than weakening fallback.'),
  ('RESOLUTION','path_resolution(7)','https://man7.org/linux/man-pages/man7/path_resolution.7.html',
   ['Relative/absolute starting directories, lines20-44','Intermediate components and link resolution, lines46-90','Search, file permissions and privilege qualifications, lines152-195'],
   'Separate directory anchor, component traversal and operation authority; no host credential changes.'),
  ('STAT','stat(2)','https://man7.org/linux/man-pages/man2/stat.2.html',
   ['stat/lstat/fstat identity and non-atomic field snapshot, lines50-77','fstatat directory-relative/absolute rules and AT_SYMLINK_NOFOLLOW, lines79-117'],
   'Named metadata copying with explicit final-link behavior and unchanged outputs on failure.'),
  ('FIELDS','stat(3type)','https://man7.org/linux/man-pages/man3/stat.3type.html',
   ['Named fields and timestamp structures, lines22-43','Inode, link count, logical size, block units and times, lines59-113','Timestamp precision history and feature-test macros, lines131-148'],
   'Record exact units and measured precision without inferring uniform physical allocation or immutable snapshots.'),
  ('UMASK','umask(2)','https://man7.org/linux/man-pages/man2/umask.2.html',
   ['Creation mask and default ACL alternative, lines27-46','Child inheritance and multithreaded fetch/restore race, lines79-89'],
   'Change mask only in owned child; distinguish ordinary mask arithmetic from default ACL creation.'),
  ('ACL','acl(5)','https://man7.org/linux/man-pages/man5/acl.5.html',
   ['ACL types, entries and mask scope, lines19-67','Object creation with and without default ACLs, lines107-136','Abandoned POSIX.1e draft status, lines280-287'],
   'Restricted three-entry default model only; no claim to evaluate named entries, ACL mask, or full Linux authorization.')]
 resources=[]
 for ident,title,url,locators,scope in rows:
  resources.append({'id':'R-LNS-'+ident,'title':'Linux '+title,'authors':['Linux man-pages / Linux ACL documentation projects'],'url':url,'locators':locators,
   'domains':['T23'],'kind':'manual','edition':'Opened current Linux manual; man-pages6.19 where identified; ACL project manual separately identified','access':'Actual bounded public passages inspected','role':'oracle','assignable':True,
   'reason':scope,'prerequisites':['C17 object/descriptor ownership and Linux hosted process model'],
   'inspection':{'date':'2026-10-17','method':'Read opened primary manual passages before implementing the successor.','scope':scope,
    'limits':'Selected passages only; no unseen standard, host security audit, full ACL implementation or kernel proof.','evidence_urls':[url]}})
 source=BASE/'resources.json';write(source,{'schema_version':1,'resources':resources})
 write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T23'],'inspection_kind':'Author inspection, not external review','research_input_sha256':{source.relative_to(ROOT).as_posix():hashlib.sha256(source.read_bytes()).hexdigest()}})
 report=json.loads((ROOT/REPORT).read_text(encoding='utf-8'))
 if not report.get('passed') or len(report.get('profiles',[]))!=6:raise ValueError('Namespace campaign must pass all six profiles')
 if not all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in report['inspected_paths'].items()):raise ValueError('Stale namespace source')
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T23-02.json';record=attach(json.loads(path.read_text(encoding='utf-8')));write(path,record)
 updates=[]
 for criterion in record['criteria']:
  if criterion['objective'] not in SCOPES:continue
  scope=SCOPES[criterion['objective']]
  updates.append({'id':criterion['id'],'original_source':path.relative_to(ROOT).as_posix(),'original_objective':criterion['objective'],'project':record['project'],
   'dimensions':{'teaching':{'state':'complete','evidence':[path.relative_to(ROOT).as_posix(),CONTRACT]},'implementation':{'state':'complete-for-declared-reference-contract','evidence':[PRIVATE,TEST]},
    'author_validation':{'state':'passed-with-stated-scope','evidence':[REPORT]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-applicable-to-declared-hosted-contract','evidence':[]}},
   'mechanism_scope':scope,'closure_rationale':scope,'variant_ids':[v['id'] for v in criterion['variants']],
   'variant_readiness':'Named changed-condition reference/platform/model support supplied; no learner delay/independence fabricated.',
   'required_environment':'Owned Linux temporary directories, fork/pipe helpers, openat2, procfs fd observations and two observed filesystem types.',
   'external_blockers':['Actual independent learner attempt and non-author review.'],
   'evidence_limits':['Bounded original programs only; no content authenticity, arbitrary filesystem confinement or complete credential/ACL model.']})
 original=next(g for g in json.loads((ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json').read_text(encoding='utf-8'))['gaps'] if g['id']=='LRUN-05')
 updates.append({'id':'LRUN-05','original_source':'research/v5/expansion/legacy_runtime/gap_ledger.json','original_record':original,
  'dimensions':{'teaching':{'state':'complete','evidence':[CONTRACT]},'implementation':{'state':'complete-for-enumerated-author-experiments','evidence':[PRIVATE,TEST]},'author_validation':{'state':'passed-with-stated-scope','evidence':[REPORT]},
   'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-applicable-to-declared-hosted-contract','evidence':[]}},
  'closure_rationale':'Controlled final/intermediate symlink counterexamples and actual full-resolution repair now supplied with ownership/identity, two-filesystem metadata and mask/ACL/authority investigations.',
  'required_environment':'Unprivileged local Linux owned teaching tree; no external targets, mounts or host policy changes.',
  'external_blockers':['Actual learner investigations and external review.'],'evidence_limits':['Default ACL and operation authorization are explicitly limited models; the actual mask observation requires absence of default ACL.','Hard links and mutable contents remain outside the resolution guarantee.']})
 pins={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in [CONTRACT,PRIVATE,TEST,REPORT,path.relative_to(ROOT).as_posix(),source.relative_to(ROOT).as_posix()]}
 write(BASE/'support.json',{'schema_version':1,'required':True,'scope':'Six original namespace/metadata criteria and LRUN-05 executable successors, retaining original records.','source_sha256':pins,'updates':updates,
  'remaining_author_work':['PR-T23-02 transfer/large-offset and persistence criteria remain separately required; this successor does not close the whole project.']})
 integrate_family();print(json.dumps({'resources':len(resources),'criteria':len(SCOPES),'original_gap':'LRUN-05','passed_campaign':True}))
if __name__=='__main__':main()
