"""Attach original mapping criteria to bounded, actually executed successor evidence."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4]
BASE=Path(__file__).resolve().parent
CONTRACT='labs/posix_lifecycle_depth/mapping/CONTRACTS.md'
PRIVATE='assessor/posix_lifecycle_depth/mapping/map.c'
TEST='labs/posix_lifecycle_depth/mapping/tests.c'
REPORT='reviews/v5/expansion/legacy_posix_mapping/mapping_validation.json'
SUPPORT='research/v5/expansion/legacy_posix_mapping/support.json'
SCOPES={
 'T23.08.a':'residency: actual3/16page anonymous mappings, one touched page, mincore samples and getrusage deltas; file-private sharing transfer uses the separate sharing experiment and bounded analysis.',
 'T23.08.b':'sharing and faults: fork-private versus shared byte update, private file write versus second view/backing file, and isolated PROT_NONE child SIGSEGV.',
 'T23.08.c':'residency: separate page size, virtual extent, touched-page count, resident snapshots and fault deltas, with no exact physical commitment inference; huge-page variation is analysis only.',
 'T23.09.a':'views: logical extent, zero/overflow/unaligned rejection, fd-close lifetime, same-inode map replacement, invalidated generation, wrap refusal and repeated cleanup.',
 'T23.09.c':'faults: child map readiness, parent truncation, controlled access release, real SIGBUS versus nontruncated control; publication supplies separately synchronized in-bounds writes.',
 'T23.10.a':'publication: version/state/byte-order records, reader exclusion through every initialization stage and exactly one winner among two gated original initializers.',
 'T23.10.b':'codec: all0..64payload lengths in both byte orders, independent expected session bytes, distinct arrays, invalid extents/offsets before pointer formation; fresh-exec pathname reader has no inherited mapping.',
 'T23.10.c':'publication: actual owned initializer SIGKILL at all3confirmed states, process-lock release, commit-state recovery and strictly increasing expected-session replacement; PID is never used as identity.'}
def write(path,value):
 if path.name=='support.json':
  import runpy
  value=runpy.run_path(str(ROOT/'research/v5/expansion/legacy_posix_followthrough/variant_support.py'))['expand'](value,ROOT)
 path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!='PR-T23-03':return record
 ids=[]
 for criterion in record['criteria']:
  if criterion['objective'] in SCOPES:criterion['successor_support']={'path':SUPPORT,'id':criterion['id']};ids.append(criterion['id'])
 record['successor_support']={'path':SUPPORT,'project':record['project'],'criterion_ids':ids,
   'scope':'Explicit mapping construction/platform observations for eight criteria; .09.b workload comparison remains open.'}
 record['execution_support'].update({'observation_status':'historical-initial-authoring','observed_before_revision':'fd85773420a2fbb11003e83c8289417d62282ec4','active_successor':SUPPORT})
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for name in ('map.h','map_stub.c','tests.c','fault_helper.c'):
  path='labs/posix_lifecycle_depth/mapping/'+name
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def integrate_family():
 family=ROOT/'labs/posix_lifecycle_depth/family.json';data=json.loads(family.read_text(encoding='utf-8'));campaigns=data.setdefault('supplemental_campaigns',[])
 tests=['posix_lifecycle_depth.mapping_'+case for case in ('views','codec','publication')]
 data['projects']=[p for p in data['projects'] if p['id']!='PR-T23-03']+[{'id':'PR-T23-03','tests':tests,
  'scope':'Checked mapping and invalidated borrows, versioned byte records, process-lock publication and owner-death recovery. Supplied fault/residency investigations have separate supplemental receipts.'}]
 for test in tests:
  if test not in data['tests']:data['tests'].append(test)
 contract='curriculum/v5/expansion/legacy_assessments/PR-T23-03.json'
 if contract not in data['assessment_contracts']:data['assessment_contracts'].append(contract)
 campaigns[:]=[r for r in campaigns if r.get('id')!='mapping-followthrough'];campaigns.append({'id':'mapping-followthrough','producer':'tools/run_posix_mapping.py','report':REPORT,'contract':CONTRACT,
  'scope':'Mapping ownership, byte records, actual process death/publication, fresh exec and original isolated faults; platform investigations distinguished from learner implementation tests.'});write(family,data)
def main():
 rows=[('MMAP','mmap(2)','https://man7.org/linux/man-pages/man2/mmap.2.html',
   ['DESCRIPTION extent, alignment and fd-close lifetime, lines31-53','MAP_SHARED and MAP_PRIVATE visibility, lines79-104','SIGSEGV and SIGBUS, lines383-392'],
   'Mapping ownership/visibility and isolated protection/truncation observations; no physical backing identity or durability proof.'),
  ('LOCK','fcntl_locking(2)','https://man7.org/linux/man-pages/man2/fcntl_locking.2.html',
   ['Advisory process record locks and F_SETLK/F_SETLKW, lines32-107','Process death, fork/exec and close-any-fd/thread limitations, lines146-168'],
   'Cooperating single-threaded process publication and owner-death exclusion. Not an interprocess ISO C atomic claim or NFS lost-lock proof.'),
  ('RESIDENCY','mincore(2)','https://man7.org/linux/man-pages/man2/mincore.2.html',
   ['DESCRIPTION page rounding, vector bits and snapshot limitation, lines35-55'],
   'Bounded current residency samples only; no future residency, hard latency or physical commitment promise.'),
  ('REMAP','mremap(2)','https://man7.org/linux/man-pages/man2/mremap.2.html',
   ['MREMAP_MAYMOVE invalidates absolute pointers, lines71-79','MREMAP_FIXED replacement and multi-mapping caveats, lines81-104'],
   'Contrast Linux remap with the course allocate-before-unmap replacement; the implementation does not call mremap or claim it always moves.')]
 resources=[]
 for ident,title,url,locators,scope in rows:
  resources.append({'id':'R-LMAP-'+ident,'title':'Linux '+title+', man-pages6.19','authors':['Linux man-pages project'],'url':url,'locators':locators,
   'domains':['T23'],'kind':'manual','edition':'Linux man-pages6.19','access':'Public bounded passages inspected','role':'oracle','assignable':True,
   'reason':scope,'prerequisites':['C17 object lifetimes and hosted descriptor ownership'],
   'inspection':{'date':'2026-10-17','method':'Read actual opened primary manual passages, not a bibliography or search result.','scope':scope,
                 'limits':'Selected Linux manual text only; no full POSIX publication, physical system or adversarial filesystem proof.','evidence_urls':[url]}})
 source=BASE/'resources.json';write(source,{'schema_version':1,'resources':resources})
 write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T23'],'inspection_kind':'Author inspection, not external review',
  'research_input_sha256':{source.relative_to(ROOT).as_posix():hashlib.sha256(source.read_bytes()).hexdigest()}})
 report=json.loads((ROOT/REPORT).read_text(encoding='utf-8'))
 if not report.get('passed') or len(report.get('profiles',[]))!=6:raise ValueError('Mapping campaign must pass all six profiles')
 if not all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in report['inspected_paths'].items()):raise ValueError('Stale mapping source')
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T23-03.json';record=attach(json.loads(path.read_text(encoding='utf-8')));write(path,record)
 updates=[]
 for criterion in record['criteria']:
  if criterion['objective'] not in SCOPES:continue
  scope=SCOPES[criterion['objective']]
  updates.append({'id':criterion['id'],'original_source':path.relative_to(ROOT).as_posix(),'original_objective':criterion['objective'],'project':record['project'],
   'dimensions':{'teaching':{'state':'complete','evidence':[path.relative_to(ROOT).as_posix(),CONTRACT]},
    'implementation':{'state':'complete-for-declared-reference-contract','evidence':[PRIVATE,TEST,'labs/posix_lifecycle_depth/mapping/fault_helper.c']},
    'author_validation':{'state':'passed-with-stated-scope','evidence':[REPORT]},'external_review':{'state':'not-run','evidence':[]},
    'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-applicable-to-hosted-mapping-contract','evidence':[]}},
   'mechanism_scope':scope,'closure_rationale':scope,'variant_ids':[v['id'] for v in criterion['variants']],
   'variant_readiness':'Declared changed-condition reference implementation or platform/analysis support supplied; no learner delay is fabricated.',
   'required_environment':'Unprivileged Linux local filesystem, owned helpers, process record locks, discovered page size; no host policy changes.',
   'external_blockers':['Independent learner attempt and actual non-author review.'],
   'evidence_limits':['Bounded kernel-process and byte-codec cases only; no physical page identity, power-loss durability, hostile lock-ignoring peer or absolute mastery proof.']})
 original=next(g for g in json.loads((ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json').read_text(encoding='utf-8'))['gaps'] if g['id']=='LRUN-06')
 updates.append({'id':'LRUN-06','original_source':'research/v5/expansion/legacy_runtime/gap_ledger.json','original_record':original,
  'dimensions':{'teaching':{'state':'complete','evidence':[CONTRACT]},'implementation':{'state':'complete-for-enumerated-author-experiments','evidence':[PRIVATE,TEST]},
   'author_validation':{'state':'passed-with-stated-scope','evidence':[REPORT]},'external_review':{'state':'not-run','evidence':[]},
   'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-applicable-to-hosted-mapping-contract','evidence':[]}},
  'closure_rationale':'Inspected the actual mapping/locking/fault source passages and executed isolated mapping faults, fresh-exec attachment, owner-death cuts and process-shared publication. Original fork/native-header test remains a narrower historical observation.',
  'required_environment':'Owned Linux process/address spaces and local mapped temporary file.','external_blockers':['Actual learner investigations and external review.'],
  'evidence_limits':['Fresh-exec child opens by pathname with no inherited mapping/fd; no reliance on coincidentally distinct numeric virtual addresses between processes.','Process termination preserves kernel page cache; no physical durability claim.']})
 pins={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in [CONTRACT,PRIVATE,TEST,REPORT,path.relative_to(ROOT).as_posix(),source.relative_to(ROOT).as_posix()]}
 write(BASE/'support.json',{'schema_version':1,'required':True,'scope':'Eight original criteria and LRUN-06 executable successors; preserve original gap snapshots.',
  'source_sha256':pins,'updates':updates,'remaining_author_work':['AX-PR-T23-03-T23.09.b explicit-I/O versus mapped workload with equal validation/failure contract and changed sparse access remains required.']})
 integrate_family()
 print(json.dumps({'resources':len(resources),'criteria':len(SCOPES),'original_gap':'LRUN-06','passed_campaign':True}))
if __name__=='__main__':main()
