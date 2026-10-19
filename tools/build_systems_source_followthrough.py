"""Package scoped source-parent successors without touching shared registers."""
import json,pathlib
from record_systems_source_followthrough import ROOT,BASE,sha,read,put
from audit_inventory_aliases import original
REPORT='reviews/v5/expansion/systems_source_followthrough/campaign_02.json'
PRIOR='research/v5/expansion/source_reconciliation/completion_dispositions.json'
PARENTS={
 'LSYS-ABI-GABI':dict(children=['LSYS-ABI-GABI-AMD64','LSYS-ABI-GABI-ELF-CORE'],resources=['legacy_systems_abi','legacy_systems_elf'],text='The formerly failed historical AMD64 call/entry source is retained as inspected draft0.99.6; existing ELF core passages plus fresh dynamic-linking chapter text cover the concrete assigned structures and responsibilities. Exact editions and uninspected processor extensions remain explicit; no complete historical/current ABI equivalence is claimed.'),
 'LSYS-RUNTIME':dict(children=['LSYS-RUNTIME-DWARF','LSYS-RUNTIME-TLS-INIT'],resources=['legacy_systems_abi','legacy_systems_elf','legacy_systems_startup','legacy_systems_freestanding'],text='The actual assignments select bounded loader/runtime ownership, a concrete compiler-rt/libgcc helper, one embedded output-library service boundary and code-location-specific unwind rules. Retained selected compiler-rt/Newlib/DWARF passages and fresh musl TLS/loader and glibc mapping/relocation/lookup bodies support those mechanisms. Uninspected lower lookup/machine relocation callees are named; whole-libc and every-helper audits are not implied or used to excuse an assigned missing mechanism.'),
 'LSYS-FORMATS':dict(children=['LSYS-FORMATS-DEBUGLINK-HEX'],resources=['legacy_systems_elf'],text='The full historical Intel HEX specification is freshly inspected in its pinned11-page1998 reprint; six record mechanisms, distinct wrap arithmetic, inert entry metadata and stricter device-window policy now have an original required C17 companion and independent arithmetic checks. The retained GDB debuglink/build-ID passages and mismatch experiments cover the actual assigned companion format. No universal programmer compatibility or cryptographic authenticity is inferred.')}

def snapshot():
 inventory=read(ROOT/'curriculum/v5/expansion/work_inventory.json');rows=[]
 for r in inventory['obligations']:
  if r['id'] in PARENTS:rows.append(dict(id=r['id'],origins=[dict(**o,record=original(o)) for o in r['origins']],requirements=r['requirements'],links=r['links']))
 assert {r['id'] for r in rows}==set(PARENTS)
 put(BASE/'parent_input.json',dict(schema_version=1,parents=rows))

def make():
 original_rows=read(BASE/'parent_input.json')['parents'];prior=read(ROOT/PRIOR);updates=[];bindings=[];files={REPORT,PRIOR,'tools/build_systems_source_followthrough.py','tools/check_systems_source_followthrough.py','tools/record_systems_source_followthrough.py','tools/run_systems_source_followthrough.py'}
 public='labs/systems_source_followthrough/CONTRACTS.md';analysis='assessor/systems_source_followthrough/ANALYSIS.md'
 def dimensions(source,code,execution,source_only=False):return dict(teaching=dict(state='complete-for-exact-assigned-source-scope',evidence=source),implementation=dict(state='not-applicable-source-parent' if source_only else 'complete-for-declared-original-parser-and-window-contract',evidence=code),author_validation=dict(state='passed-source-identity-and-bounded-author-execution',evidence=execution),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-applicable-to-source-and-inert-host-decoder',evidence=[]))
 for entry in original_rows:
  p=PARENTS[entry['id']];selected=[];source=[public,analysis,BASE.relative_to(ROOT).as_posix()+'/comparison.md',BASE.relative_to(ROOT).as_posix()+'/resources.json',BASE.relative_to(ROOT).as_posix()+'/assignment_scope.json']
  for o in entry['origins']:assert original(o)==o['record'];files.add(o['path'])
  for cid in p['children']:
   r=next(r for r in prior['updates'] if r['id']==cid);selected.append(r)
  for name in p['resources']:
   for item in ['resources.json','research_gate.json']:
    path='research/v5/expansion/'+name+'/'+item;source.append(path);files.add(path)
  updates.append(dict(id=entry['id'],original_origins=entry['origins'],dimensions=dimensions(source,[public],[REPORT],True),closure_rationale=p['text'],source_children=p['children'],artifacts=source+[REPORT],evidence_limits=['Only the exact named passages are inspected; every source has version and scope.','No whole-library, all-ABI, physical-target, independent learner or reviewer claim.','Exact published ISO C17/C23 remains a separate unresolved source obligation.']))
  bindings.append(dict(**entry,selected_source_children=selected,clause_analysis=p['text']))
 for ident in ['SE-SYS-HEX-01','SE-SYS-HEX-01-DELAYED']:
  updates.append(dict(id=ident,parent_project='PR-T09-05',minimum_days=7 if ident.endswith('DELAYED') else None,dimensions=dimensions([public],[public,'assessor/systems_source_followthrough/hex.c','labs/systems_source_followthrough/tests.c'],[REPORT]),closure_rationale='Original bounded byte parser covers all six historical record mechanisms under explicit profiles; distinct segment/linear wrap and a nonwrapping data-window changed condition execute with independent arithmetic oracle and compiling semantic controls.',artifacts=[public,analysis,'labs/systems_source_followthrough/hex.h','labs/systems_source_followthrough/hex.c','labs/systems_source_followthrough/tests.c','labs/systems_source_followthrough/driver.c'],evidence_limits=read(ROOT/REPORT)['limits']+['The delayed task has an author reference; independent learner completion after seven days is not inferred.']))
 for tree in [BASE,ROOT/'labs/systems_source_followthrough',ROOT/'assessor/systems_source_followthrough']:
  files.update(p.relative_to(ROOT).as_posix() for p in tree.iterdir() if p.is_file() and p.name not in ['support.json','evidence_map.json'])
 return updates,dict(schema_version=1,scope='Original three source parents and exact prior bounded source-child records',bindings=bindings),files

def main():
 if not (BASE/'parent_input.json').exists():snapshot()
 updates,mapping,files=make();put(BASE/'evidence_map.json',mapping);files.add((BASE/'evidence_map.json').relative_to(ROOT).as_posix())
 put(BASE/'support.json',dict(schema_version=1,scope='Three scoped source-parent resolutions and one required original companion with changed-condition implementation',updates=updates,source_sha256={p:sha(ROOT/p) for p in sorted(files)},current_reports=[REPORT],remaining_external_work=['Independent source/content review and ordinary/delayed learner evidence are not supplied.','Exact published ISO editions and other named physical/source obligations retain their separate records.']))
 print(json.dumps(dict(updates=len(updates),source_parents=len(PARENTS))))
if __name__=='__main__':main()
