"""Canonical registration of the original signed rational-unit successor."""
import json
from common import ROOT,dump,read_json,sha

def register():
    module='rational_units';anchors=['rational_fraction','rational_boundary','rational_base'];tests=['adt.'+a for a in anchors]
    f=read_json(ROOT/'labs/adt/family.json')
    for row in f.get('projects',[])+f.get('exercises',[]):
        if row['id'] in ['PR-T67-02','X-T67-002']:row['tests']=list(dict.fromkeys(row['tests']+tests))
    f['tests']=list(dict.fromkeys(f.get('tests',[])+tests));f.setdefault('campaign',{}).update(rational_units_producer='tools/run_adt_rational_units.py',rational_units_report='research/v5/modern/adt_rational_units_validation.json');dump(ROOT/'labs/adt/family.json',f)
    p=ROOT/'labs/adt/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/rational_units/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    brief=ROOT/'curriculum/v5/exercises/X-T67-002.md';body=brief.read_text(encoding='utf8');a,rest=body.split('```json\n',1);raw,b=rest.split('\n```',1);m=json.loads(raw)
    m['lab']['tests']=list(dict.fromkeys(m['lab']['tests']+tests));contract='labs/adt/rational_units/RATIONAL.md';m['shared_contracts']=list(dict.fromkeys(m['shared_contracts']+[contract]))
    source='research/v5/modern/adt_rational_units_resources.json';record=dict(id='R-AD-RATIONAL-N1570',path=source,passages=['6.3.1.3 paragraphs1-3: signed and unsigned integer conversions'])
    m['source_records']=[r for r in m['source_records'] if r['id']!=record['id']]+[record]
    heading='\n## Required rational-unit successor\n';b=b.split(heading,1)[0].rstrip()+heading+'\n[RATIONAL.md](../../../labs/adt/rational_units/RATIONAL.md) defines the signed rational conversion, exact precision/range contract, complete no-wider-integer argument, diagnostic controls and actual raw/wrapper comparison. Original delayed IDs and independent assessment remain active.\n'
    brief.write_text(a+'```json\n'+json.dumps(m,indent=2,ensure_ascii=False)+'\n```'+b,encoding='utf8')
    resource=dict(id=record['id'],title='N1570 integer conversions for signed rational scaling',authors=['ISO/IEC JTC1/SC22/WG14'],edition='N1570 C11 committee draft, April2011; not published C17/C23',year=2011,kind='standard-draft',url='https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf',access='public primary draft passage inspected',domains=['T67'],role='oracle',reason='Derive magnitude via unsigned conversion without unrepresentable signed conversion.',prerequisites=['Integer ranges and representations'],locators=record['passages'],assignable=True,inspection=dict(date='2026-10-18',method='Opened primary PDF and read6.3.1.3 paragraphs1-3, PDFpage68.',scope='Representable conversion preserves value; conversion to unsigned is modulo maximum-plus-one; unrepresentable signed conversion has implementation-defined behavior/signal.',limits='Only named C11 draft clauses; published C17/C23 texts were not inspected.'))
    dump(ROOT/source,dict(schema_version=1,resources=[resource],reused_sources=['R-ADMP-DURATION'],inspection_note='Reread rolling C++ draft time.duration.cast paragraphs1-3 on2026-10-18; comparison explicitly bounded to those formulas and actual small C++17 executions.'))
    dump(ROOT/'research/v5/modern/adt_rational_units_research_gate.json',dict(status='passage-inspected',domains=['T67'],reviewer='author only; no independent review',research_input_sha256={p:sha(ROOT/p) for p in [source,contract,'research/v5/modern/adt_resources.json']}))
    dump(ROOT/'labs/adt/rational_units/campaign.json',dict(module=module,target='adt-rational_units',learner_option='CMASTERY_ADQ_LEARNER',anchors=anchors,control_count=6,observation_markers=['ADQ_FRACTION no-wide-integer','ADQ_BOUNDARY signed-minimum','ADQ_BASE actual-seconds-provider'],oracle='tools/verify_adt_rational_units.py',metrics=['metrics'],object_sources=['adt.c','rational_units.c'],producer='tools/run_adt_rational_units.py',report='research/v5/modern/adt_rational_units_validation.json',imports=['labs/adt/include/adt.h','labs/adt/src/adt.c','assessor/adt/adt.c','labs/adt/CONTRACTS.md',source,'research/v5/modern/adt_rational_units_research_gate.json','tools/register_adt_rational_units.py']))
if __name__=='__main__':register()
