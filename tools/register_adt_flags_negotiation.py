"""Canonical required/advisory flags successor and exact passage registration."""
import json
from common import ROOT,dump,read_json,sha
def register():
    module='flags_negotiation';anchors=['flags_negotiation','flags_sets','flags_base'];tests=['adt.'+a for a in anchors]
    f=read_json(ROOT/'labs/adt/family.json')
    for row in f.get('projects',[])+f.get('exercises',[]):
        if row['id'] in ['PR-T67-03','X-T67-003']:row['tests']=list(dict.fromkeys(row['tests']+tests))
    f['tests']=list(dict.fromkeys(f.get('tests',[])+tests));f.setdefault('campaign',{}).update(flags_negotiation_producer='tools/run_adt_flags_negotiation.py',flags_negotiation_report='research/v5/modern/adt_flags_negotiation_validation.json');dump(ROOT/'labs/adt/family.json',f)
    p=ROOT/'labs/adt/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/flags_negotiation/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    brief=ROOT/'curriculum/v5/exercises/X-T67-003.md';body=brief.read_text(encoding='utf8');a,rest=body.split('```json\n',1);raw,b=rest.split('\n```',1);m=json.loads(raw)
    contract='labs/adt/flags_negotiation/NEGOTIATION.md';source='research/v5/modern/adt_flags_negotiation_resources.json'
    m['lab']['tests']=list(dict.fromkeys(m['lab']['tests']+tests));m['shared_contracts']=list(dict.fromkeys(m['shared_contracts']+[contract]))
    record=dict(id='R-AD-FLAGS-N1570',path=source,passages=['6.5.3.3 paragraph4: complement and integer promotions','6.5.10 paragraphs2-4 and6.5.12 paragraphs2-4: integer bitwise AND and inclusive OR'])
    m['source_records']=[r for r in m['source_records'] if r['id']!=record['id']]+[record]
    heading='\n## Required negotiated-flags successor\n';b=b.split(heading,1)[0].rstrip()+heading+'\n[NEGOTIATION.md](../../../labs/adt/flags_negotiation/NEGOTIATION.md) requires actual full32-bit constructor enumeration, validated set algebra, two-version required/advisory negotiation and forwarding/activation transfer. It preserves the original rubric and delayed IDs; author campaigns do not constitute independent learner assessment.\n'
    brief.write_text(a+'```json\n'+json.dumps(m,indent=2,ensure_ascii=False)+'\n```'+b,encoding='utf8')
    resource=dict(id=record['id'],title='N1570 integer promotions and bitwise set operations',authors=['ISO/IEC JTC1/SC22/WG14'],edition='N1570 C11 committee draft, April2011; not published C17/C23',year=2011,kind='standard-draft',url='https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf',access='public primary draft passages inspected',domains=['T67'],role='oracle',reason='Derive exact bit-set membership and bound claims about complement promotions.',prerequisites=['Integer promotions and unsigned types'],locators=record['passages'],assignable=True,inspection=dict(date='2026-10-18',method='Opened primary PDF and read named clauses at PDFpages106,114,115.',scope='Complement after promotions and bitwise AND/OR after usual arithmetic conversions; per-bit semantics.',limits='Only named C11 draft passages; exact published C17/C23 remain inaccessible. Negotiation policy is original teaching design, not specified by this standard.'))
    dump(ROOT/source,dict(schema_version=1,resources=[resource]))
    dump(ROOT/'research/v5/modern/adt_flags_negotiation_research_gate.json',dict(status='passage-inspected',domains=['T67'],reviewer='author only; no independent review',research_input_sha256={p:sha(ROOT/p) for p in [source,contract,'research/v5/modern/adt_resources.json']}))
    dump(ROOT/'labs/adt/flags_negotiation/campaign.json',dict(module=module,target='adt-flags_negotiation',learner_option='CMASTERY_ADF_LEARNER',anchors=anchors,control_count=6,observation_markers=['ADF_NEGOTIATION intersection-authority','ADF_SETS complete-three-bit-lattice','ADF_BASE actual-ad-flags'],oracle='tools/verify_adt_flags_negotiation.py',metrics=['metrics'],object_sources=['adt.c','flags_negotiation.c'],producer='tools/run_adt_flags_negotiation.py',report='research/v5/modern/adt_flags_negotiation_validation.json',imports=['labs/adt/include/adt.h','labs/adt/src/adt.c','assessor/adt/adt.c','labs/adt/CONTRACTS.md',source,'research/v5/modern/adt_flags_negotiation_research_gate.json','tools/register_adt_flags_negotiation.py']))
if __name__=='__main__':register()
