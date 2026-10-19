"""Attach exact T32 architecture obligations without changing historical bodies."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T32-01';SUPPORT='research/v5/expansion/legacy_testing_architecture/support.json';CONTRACT='labs/testing_depth/architecture/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_testing_architecture/campaign.json'
SCOPES={
'T32.01.a':'Actual caller-owned queue vs scripted/host socket harness, object/link attribution, stack-backed versus malloc/free harness storage with unchanged production implementation.',
'T32.01.b':'Independent Python list predicts original7/8/FULL/EMPTY and all65536 selected histories/profile across four implementations, including caller-owned linked nodes.',
'T32.01.c':'Actual native plain, ASan+UBSan, gcov and trap-only object/link maps with host allocator attribution; plain/trap no-library link and empty undefined symbols, no universal target-helper inference.',
'T32.02.a':'458752 intermediate observations/profile check FIFO/conservation and separate layout invariants; nonconsuming peek included in every selected history.',
'T32.02.b':'Capacity1..8, empty/singleton/full/repeated wrap, invalid extents and canaries/refusal snapshots; partial batch commits exact accepted prefix instead of blanket unchanged behavior.',
'T32.02.c':'Identical public suites/oracle on spare ring, compact array, linked nodes and all-slots-usable counted ring at equal visible capacity.',
'T32.03.a':'Actual decoder→queue→owned staging→writer with exact prefix, copied input, cancellation/error/uncertainty/terminal behavior; zero-progress callback budget bounds termination.',
'T32.03.b':'Actual AF_UNIX acquisition/send/receive/close, controlled adapter cap and separate kernel short-return/EAGAIN observation; early consumer close produces contained EPIPE with MSG_NOSIGNAL.',
'T32.03.c':'Authored and checked property-authority matrix explicitly separates finite logical evidence, actual selected OS results, unsupported emulator peripheral and unobserved UART baud/ADC analog accuracy.'}
def write(p,v):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Nine original architecture/unit/integration criteria and nine exact changed conditions; author finite/model/native OS evidence, no physical or learner completion.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 paths=['fixtures/embedded/PR-T32-01.json',*sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/testing_depth/architecture').rglob('*') if p.is_file() and '__pycache__' not in p.parts)]
 for p in paths:
  if p not in record['supplied_artifacts']:record['supplied_artifacts'].append(p)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'));from expansion_evidence import require_batch_research;from check_testing_architecture import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(path.read_text(encoding='utf-8')));write(path,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':path.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
   'dimensions':{'teaching':{'state':'complete-for-nine-original-criteria-and-changed-conditions','evidence':[CONTRACT,'labs/testing_depth/architecture/evidence_matrix.json']},'implementation':{'state':'implemented-exact-finite-queue-flow-adapter-and-analysis-contracts','evidence':['assessor/testing_depth/architecture.c','assessor/testing_depth/architecture.md','labs/testing_depth/architecture/tests.c','labs/testing_depth/architecture/socket.c','labs/testing_depth/architecture/oracle.py']},'author_validation':{'state':'passed-six-host-profiles-independent-oracle-controls-and-four-runtime-audits','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-physical-UART-baud-and-ADC-accuracy-unobserved','evidence':[]}},
   'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 native Linux GCC/Clang debug/release/ASan+UBSan, actual local AF_UNIX sockets; GNU nm/linker and coverage/trap options. Strict component no allocator/libc helpers on recorded x86-64 build; host verifier dependencies explicit.',
   'external_blockers':['Independent human review and original minimum-seven-day unaided learner assessment remain unattempted.','Physical UART/ADC observations and unavailable emulator peripheral evidence remain explicitly unobserved.'],
   'evidence_limits':['Finite histories and tests are not proof of every execution or learner mastery.','Plain/trap empty undefined sets are exact native compiler/flags observations, not all ISA/toolchain dependency guarantees.','Caller storage/lifetime/disjointness/serialization are explicit C programmer obligations.','MSG_NOSIGNAL is an OS library contract, not ISO C; successful send does not prove remote delivery or durability.','Controlled one-byte adapter chunking and actual kernel short return are separate observations.','Callback attempts are bounded logically; no wall-clock scheduling or arbitrary blocked-call cancellation claim.','No physical peripheral evidence can be inferred from identical host/emulator bytes.','Exact published C17 remains unavailable.']}
  updates.append(row)
  for variant in c['variants']:
   child=deepcopy(row);child.update(id=variant['id'],parent_id=c['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],variant_readiness='Exact original changed condition implemented or analyzed with evidence authority preserved; independent delayed learner and physical observations remain separate.');updates.append(child)
 resource=BASE/'resources.json';resources=json.loads(resource.read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,path.relative_to(ROOT).as_posix(),'tools/check_testing_architecture.py','assessor/testing_depth/architecture.md',*[p.relative_to(ROOT).as_posix() for p in (BASE/'author_support.py',BASE/'register_sources.py',resource,BASE/'source_access.json',BASE/'research_gate.json')]]
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource.relative_to(ROOT).as_posix(),'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_testing_architecture.py','report':CAMPAIGN}],'prior_attempts':['reviews/v5/expansion/legacy_testing_architecture/campaign_attempt1.json'],'remaining_author_work':[],'remaining_external_work':['Physical peripheral observations where claimed.','Independent review and delayed learner assessment.'],'scope':'All nine exact original criteria/changed conditions; finite component tests, actual selected OS adapter and bounded runtime dependency analysis.'})
 print(json.dumps({'updates':len(updates),'criteria':9,'variants':9}))
if __name__=='__main__':main()
