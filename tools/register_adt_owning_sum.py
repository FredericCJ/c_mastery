"""Connect required ADT successors without replacing retained mechanisms."""
from pathlib import Path
import json
from common import ROOT,dump,read_json,sha

def register():
    module='owning_sum';anchors=['sum_value','sum_owner','sum_rollback','sum_base'];tests=['adt.'+a for a in anchors]
    f=read_json(ROOT/'labs/adt/family.json')
    for row in f.get('projects',[])+f.get('exercises',[]):
        if row['id'] in ['PR-T67-01','X-T67-001']:row['tests']=list(dict.fromkeys(row['tests']+tests))
    f['tests']=list(dict.fromkeys(f.get('tests',[])+tests));f.setdefault('campaign',{}).update(owning_sum_producer='tools/run_adt_owning_sum.py',owning_sum_report='research/v5/modern/adt_owning_sum_validation.json')
    dump(ROOT/'labs/adt/family.json',f)
    p=ROOT/'labs/adt/CMakeLists.txt';text=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/owning_sum/CMakeLists.txt")'
    if line not in text:p.write_text(text.rstrip()+'\n'+line+'\n',encoding='utf8')
    brief=ROOT/'curriculum/v5/exercises/X-T67-001.md';text=brief.read_text(encoding='utf8');a,rest=text.split('```json\n',1);raw,b=rest.split('\n```',1);m=json.loads(raw)
    m['lab']['tests']=list(dict.fromkeys(m['lab']['tests']+tests));m['shared_contracts']=list(dict.fromkeys(m['shared_contracts']+['labs/adt/owning_sum/OWNING_SUM.md']))
    m['prerequisite_exercises']=list(dict.fromkeys(m['prerequisite_exercises']+['X-T66-001','X-T66-003','X-T66-004']))
    record=dict(id='R-AD-OWN-GCC-SWITCH',path='research/v5/modern/adt_owning_sum_resources.json',passages=['Warning Options: -Wswitch and -Wswitch-enum'])
    m['source_records']=[r for r in m['source_records'] if r['id']!=record['id']]+[record]
    b=b.replace('Complete the earlier domain projects and retain the named prerequisite artifacts with their contracts, ownership ledger and tests. Required learner artifacts: . Do not import an assessor implementation.','For the owning successor complete X-T66-001, X-T66-003 and X-T66-004 and retain your constructor, rollback and move reasoning. Implement the original ADValue provider before its owning adapter. No prebuilt owner library or assessor implementation is a learner dependency.')
    heading='\n## Required owning successor\n';b=b.split(heading,1)[0].rstrip()+heading+'\n[OWNING_SUM.md](../../../labs/adt/owning_sum/OWNING_SUM.md) specifies the owning text alternative, fallible clone/drop protocol, complete old-eliminator migration, exact diagnostic controls and source-bound author support. The original transfer IDs and independent learner requirement remain active.\n'
    brief.write_text(a+'```json\n'+json.dumps(m,indent=2,ensure_ascii=False)+'\n```'+b,encoding='utf8')
    resource=dict(id=record['id'],title='GCC Warning Options: enum switch diagnostics',authors=['GNU Compiler Collection contributors'],edition='Online manual passages inspected 2026-10-18',year=2026,kind='manual',url='https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html',access='public primary text inspected',domains=['T67'],role='oracle',reason='Distinguish default-suppressed switch diagnostics from switch-enum policy.',prerequisites=['C17 switch and enumerations'],locators=record['passages'],assignable=True,inspection=dict(date='2026-10-18',method='Opened primary manual and read the named paragraphs; compared against actual GCC/Clang probes separately.',scope='-Wswitch suppresses a missing-enumerator warning when default exists; -Wswitch-enum still diagnoses omitted enumerators.',limits='Only these diagnostic paragraphs; no whole-manual inspection or ISO exhaustiveness claim.'))
    source='research/v5/modern/adt_owning_sum_resources.json';dump(ROOT/source,dict(schema_version=1,resources=[resource],reused_sources=['R-ADMP-ENUM','R-ADMP-MATCH','R-ADMP-C11'],inspection_note='Rust enum introduction/constructor/value and match scrutinee/binding/guard passages reread2026-10-18; existing exact source identities retained. C11 draft authority remains bounded; no published C17/C23 inspection.'))
    contract='labs/adt/owning_sum/OWNING_SUM.md'
    dump(ROOT/'research/v5/modern/adt_owning_sum_research_gate.json',dict(status='passage-inspected',domains=['T67'],reviewer='author inspection only; no independent review',research_input_sha256={p:sha(ROOT/p) for p in [source,contract,'research/v5/modern/adt_resources.json']}))
    dump(ROOT/'labs/adt/owning_sum/campaign.json',dict(module=module,target='adt-owning_sum',learner_option='CMASTERY_ADO_LEARNER',anchors=anchors,control_count=8,observation_markers=['ADO_VALUES variants4','ADO_OWNER independent-copy','ADO_ROLLBACK before-allocation','ADO_BASE actual-ad-measure'],oracle='tools/verify_adt_owning_sum.py',metrics=['metrics'],object_sources=['adt.c','owning_sum.c'],producer='tools/run_adt_owning_sum.py',report='research/v5/modern/adt_owning_sum_validation.json',imports=['labs/adt/include/adt.h','labs/adt/src/adt.c','assessor/adt/adt.c','labs/adt/CONTRACTS.md',source,'research/v5/modern/adt_owning_sum_research_gate.json','tools/register_adt_owning_sum.py']))
if __name__=='__main__':register()
