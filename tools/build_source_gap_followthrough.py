"""Exact named-source dispositions, retaining historical and unresolved scopes."""
import copy,hashlib,json,re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];BASE='research/v5/expansion/source_gap_followthrough/';PRIVATE='research/private/source_gap_followthrough/';REVIEW='reviews/v5/expansion/source_gap_followthrough/'
REPORTS=[REVIEW+'genmc_validation_02.json',REVIEW+'genmc_attempts_01.json',BASE+'source_validation.json']
SUCCESSORS={'research/v5/expansion/effective_c_followthrough/resources.json':['R-EC2-LEAK-SUCCESSOR'],'research/v5/expansion/source_reconciliation/resources.json':['R-SREC-CLOSE24','R-SREC-CLOSE17','R-SREC-RDLOCK24','R-SREC-RDLOCK17','R-SREC-LINUX-CLOSE']}
PROVIDERS={'research/v5/expansion/effective_c_followthrough/completion_dispositions.json':['G-FAILURE-EFFECTIVEC-LEAKS'],'research/v5/expansion/source_reconciliation/completion_dispositions.json':['LIO-POSIX2024-CLOSE','G-SOURCE-POSIX2024-RDLOCK']}
REMAINDER=['Exact published ISO C17/C23 wording remains outside supplied access.','A POSIX edition classifier and inspected clauses do not establish actual POSIX2024 runtime behavior or a second OS execution.']
def read(p):return json.loads((ROOT/p).read_text(encoding='utf-8-sig'))
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def objsha(v):return hashlib.sha256(json.dumps(v,ensure_ascii=False,sort_keys=True).encode()).hexdigest()
def write(p,v):
 q=ROOT/p;q.parent.mkdir(parents=True,exist_ok=True);q.write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def front(n):return json.loads(re.match(r'```json\s*(.*?)\s*```',(ROOT/f'curriculum/modules/T{n}.md').read_text(),re.S)[1])
def safe(r):
 out=copy.deepcopy(r);ins=out['inspection']
 for k in ['local_path','private_payload']:ins.pop(k,None)
 if isinstance(ins.get('local_document_id'),str) and ('/' in ins['local_document_id'] or '\\' in ins['local_document_id']):ins['local_document_id']=None
 return out
def selected():
 old=read(PRIVATE+'assigned_inspection_input.json');paths=sorted({p for pairs in old['resources'].values() for p,r in pairs});allrows=[]
 for p in paths:
  v=read(p);allrows.extend((p,r) for r in (v if isinstance(v,list) else v['resources']))
 rows=[]
 for n in [18,19,20,21,22,43,44,45,46,47,48,49,50]:
  for assignment in front(n)['readings']:
   matches=[(p,r) for p,r in allrows if r['id']==assignment['resource_id'] and assignment['locator'] in r.get('locators',[]) and r.get('assignable') is not False and r.get('inspection',{}).get('status') not in {'unavailable','not-read','not-inspected','not-run','planned','discovered-only','reference-gap'} and all(r.get('inspection',{}).get(k) for k in ['date','scope','method','limits'])];assert matches,assignment
   p,r=sorted(matches,key=lambda v:(v[1]['inspection']['date'],v[0]))[-1];rows.append(dict(topic='T'+str(n),assignment=assignment,registry=p,registry_sha256=sha(p),entry_sha256=objsha(r),entry=safe(r),basis='retained substantive passage inspection; no new reading inferred from locator match'))
 return rows,paths

def successors():
 rows=[]
 for path,ids in SUCCESSORS.items():
  for ident in ids:
   entry=next(r for r in read(path)['resources'] if r['id']==ident)
   rows.append(dict(registry=path,registry_sha256=sha(path),entry_sha256=objsha(entry),entry=safe(entry),basis='Integrator independently recorded this named source inspection; retained date and scope, not a new reading or runtime execution by this audit.'))
 return rows
def main():
 rows,registries=selected();latest=read(PRIVATE+'LATEST.json');capture=read(latest['receipt']);assert sha(latest['receipt'])==latest['sha256'];receipt_path=latest['receipt']
 write(BASE+'assigned_inspection.json',dict(assignments=rows,topics=13,resources=len({r['assignment']['resource_id'] for r in rows}),limits=['Exact assigned locators only, not all listed publication contents.','Retained inspection dates remain original; new reinspection listed separately.']))
 fresh=[dict(resource='R-modern-c-3',locators=['10.1 Interface documentation'],pages='PDF216-218',summary='Public caller obligations and semantically connected interfaces; C23 examples do not alter C17 baseline.'),dict(resource='R-hanson-cii',locators=['3.1 Interface','3.2 Implementation','6.1 Interface'],pages='PDF48-56 and105-107',summary='Interned immutable owned copy; bucket representation and length lookup; arena group lifetime and distinct allocation exceptions.'),dict(resource='R-kernighan-ritchie-2',locators=['5.11 Pointers to Functions'],pages='PDF106-108',summary='Historical sorting dispatch; incompatible callback casts require exactly typed modern adapters.'),dict(resource='R-parnas-modules-1972',locators=['Comparison of the Two Modularizations','The Criteria'],url='https://www.cs.lafayette.edu/~gexia/cs301/resources/parnas.html',summary='Representation/time-of-work decisions stay private; identical assembled algorithms can have different maintainable interfaces.'),dict(resource='R-dijkstra-ewd249',locators=['On program families.'],url='https://www.cs.utexas.edu/~EWD/transcriptions/EWD02xx/EWD249/EWD249.html',summary='Shared refinements isolate anticipated variation and share reasoning; avoid unjustified generality.'),dict(resource='R-doxygen-contract-docs',locators=['Documenting the code','\\pre { description of the precondition }'],url='https://www.doxygen.nl/manual/commands.html#cmdpre',summary='Structured comments format consumer contracts but do not enforce preconditions.'),dict(resource='R-KER-MULTIBOOT2',locators=['3.3 I386 machine state','3.6.1 Boot information format','3.6.2 Basic tags structure','3.6.6 Modules','3.6.8 Memory map'],url='https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html',summary='Own stack/GDT/IDT; aligned size-bounded tag traversal; reserve live kernel/MBI/module ranges even within available-RAM entries; inconsistent stray size sentence retained.')]
 public=dict(passed=True,date='2026-10-18',assigned_readings=len(rows),assigned_resources=len({r['assignment']['resource_id'] for r in rows}),fresh_passages=fresh,held_identities=[{k:v for k,v in row.items() if k!='path'} for row in capture['held']],private_receipt=receipt_path,private_receipt_sha256=sha(receipt_path),renewed_public_sources=[{k:v for k,v in row.items() if k!='path'} for row in capture['web']],successor_inspections=successors(),successor_providers=[dict(path=p,sha256=sha(p),child_ids=ids) for p,ids in PROVIDERS.items()],unresolved=REMAINDER,no_whole_book_claim=True)
 write(BASE+'source_validation.json',public)
 origins=[('G-LDEPTH-API-SOURCES','research/v5/expansion/legacy_api/gap_ledger.json','gaps'),('G-LDEPTH-DIAGNOSTICS-SOURCES','research/v5/expansion/legacy_diagnostics/gap_ledger.json','gaps'),('XI-GENMC-OPT-IR','research/v5/expansion/legacy_advanced_handoff/source_access.json','gaps')];updates=[]
 evidence=['labs/source_gap_followthrough/README.md','assessor/source_gap_followthrough/ANALYSIS.md',BASE+'assigned_inspection.json',BASE+'source_validation.json']
 successor_evidence=list(SUCCESSORS)+list(PROVIDERS)+['research/v5/expansion/effective_c_followthrough/inspection.json','labs/source_readings/realloc_ownership.md','assessor/source_readings/realloc_ownership.md','labs/posix_edition/CONTRACTS.md','assessor/posix_edition/ANALYSIS.md']
 for ident,path,key in origins:
  original=next(r for r in read(path)[key] if r['id']==ident);tool=ident.startswith('XI-');dimension=dict(teaching=dict(state='complete-for-exact-bounded-source-and-tool-comparison',evidence=[evidence[0],evidence[2],evidence[3]]),implementation=dict(state='partial-selected-optimized-profile-supported-default-O1-still-unsupported' if tool else 'not-applicable-source-access-disposition-ISO-gap-retained',evidence=[evidence[1]]+(['tools/run_genmc_ir_validation.py'] if tool else [evidence[2]])),author_validation=dict(state='passed-selected-source-and-execution-scope-with-explicit-unresolved-remainder',evidence=REPORTS+([] if tool else successor_evidence)),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-claimed',evidence=[]))
  updates.append(dict(id=ident,original_source=path,original_record=original,closure_rationale='Exact original queue/harness at O1-fno-builtin verified under unchanged RC11 mode with all three semantic race controls; default O1 promotion failure remains unsupported.' if tool else 'Every current assigned locator has an exact bounded inspected registry passage; selected accessible omissions renewed from actual sources, original inspection dates and exact ISO gaps retained. No unspecified other-book blocker inferred.',dimensions=dimension,evidence_limits=public['unresolved']+['Author source/tool work is not independent review, learner retrieval, physical execution or a whole-program/whole-book claim.'],remaining_scope=['Default O1 memory-intrinsic promotion and whole service/frame translation unit remain unsupported here.'] if tool else public['unresolved']))
 tool_row=next(r for r in updates if r['id']=='XI-GENMC-OPT-IR')
 tool_row.update(required_environment='GenMC v0.19.0 commit 9f6c4c0 with LLVM 21.1.8; retained original queue/harness and RC11 verify mode, with race detection enabled.',
  validation_method='Reproduce the exact default -O1 command and inspect emitted LLVM IR and termination status. A future supported tool must also pass the same three semantic race controls; the selected -O1 -fno-builtin result is a separate compiler profile.',
  external_blockers=[dict(missing_prerequisite='A GenMC/LLVM combination that correctly handles the retained default -O1 aggregate memory intrinsic without the observed signal 4 during promotion.',
   affected_ids=['XI-GENMC-OPT-IR'],attempts=REPORTS[:2],
   completed_author_support=['tools/run_genmc_ir_followthrough.py','tools/run_genmc_ir_validation.py','assessor/interrupt_depth/handoff/queue.c','assessor/interrupt_depth/handoff/genmc.c',evidence[0],evidence[1]],
   procedure_to_finish='Use a separately installed, version-pinned tool; retain the original failed default-O1 command and raw log from genmc_attempts_01.json. Rerun that command, IR verification and all positive/race-control checks against unchanged source. Qualify any whole-service extension separately; do not infer it from the selected queue model.')])
 pins=set(registries+evidence+REPORTS+[p for i,p,k in origins]+[BASE+'support.json'])- {BASE+'support.json'}
 pins.update(successor_evidence)
 for r in public['successor_inspections']:
  cap=r['entry']['inspection'].get('capture')
  if cap:pins.add(cap['path'])
 history=BASE+'history/source_packet_01/manifest.json';pins.add(history)
 for r in read(history).values():pins.add(r['archive'])
 pins.update([BASE+'assigned_inspection.json',PRIVATE+'LATEST.json',PRIVATE+'capture.py',PRIVATE+'capture_01.py',PRIVATE+'assigned_inspection_input.json',receipt_path,'tools/build_source_gap_followthrough.py','tools/check_source_gap_followthrough.py','tools/run_genmc_ir_followthrough.py','tools/run_genmc_ir_validation.py',BASE+'history/run_genmc_ir_validation_01.py',REVIEW+'genmc_validation_01.json','research/v5/expansion/legacy_advanced_handoff/optimized_model_attempt.json'])
 pins.update('curriculum/modules/T'+str(n)+'.md' for n in [18,19,20,21,22,43,44,45,46,47,48,49,50])
 for report in REPORTS[:2]:pins.update(read(report)['input_sha256'])
 for r in capture['extracts']:pins.add(r['extract'])
 for r in capture['web']:
  if r.get('path'):pins.add(r['path'])
 write(BASE+'support.json',dict(schema_version=1,required=True,updates=updates,current_reports=[REPORTS[0],REPORTS[2]],historical_attempts=[REPORTS[1],REVIEW+'genmc_validation_01.json'],source_sha256={p:sha(p) for p in sorted(pins)},remaining_author_work=[dict(id='XI-GENMC-OPT-IR',scope='Unsupported unrestricted default O1 route retained; selected compiler profile now executed.')],remaining_external_work=public['unresolved']))
 print(json.dumps(dict(rows=3,assignments=len(rows),resources=public['assigned_resources'],source_files=len(pins))))
if __name__=='__main__':main()
