"""Integrate required inline movement and opaque-token author support."""
import json
from common import ROOT,dump,read_json,sha
ANCHORS=['move_base','move_inline','move_admission','move_registry','move_theft','move_retirement']
MARKERS=['OMM_BASE actual-move','OMM_INLINE self-address','OMM_ADMISSION actual-allocation-failure','OMM_REGISTRY opaque-representation','OMM_THEFT identical-live-copy','OMM_RETIREMENT final-generation']

def register():
    public='labs/ownership/move';contract=public+'/MOVE.md';source='research/v5/modern/ownership_move_resources.json';gate='research/v5/modern/ownership_move_research_gate.json';tests=['ownership.'+a for a in ANCHORS]
    f=read_json(ROOT/'labs/ownership/family.json')
    for r in f['exercises']:
        if r['id']=='X-T66-003':r['tests']=list(dict.fromkeys(r['tests']+tests))
    f['tests']=list(dict.fromkeys(f['tests']+tests));f['campaign'].update(move_producer='tools/run_ownership_move.py',move_report='research/v5/modern/ownership_move_validation.json');dump(ROOT/'labs/ownership/family.json',f)
    p=ROOT/'labs/ownership/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/move/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    p=ROOT/'curriculum/v5/exercises/X-T66-003.md';a,rest=p.read_text(encoding='utf8').split('```json\n',1);raw,b=rest.split('\n```',1);m=json.loads(raw);m['lab']['tests']=list(dict.fromkeys(m['lab']['tests']+tests));m['shared_contracts']=list(dict.fromkeys(m['shared_contracts']+[contract]));heading='\n## Required inline-move and opaque-token investigation\n';b=b.split(heading,1)[0].rstrip()+heading+'\n[MOVE.md](../../../labs/ownership/move/MOVE.md) specifies actual ow_move/ow_consume, inline pointer repair, typed allocation failures and opaque generation authority. Execute paired C17 compiler-extension diagnostics and C23 nodiscard comparisons separately. Demonstrate that a copied live token can act first: generation checking prevents repeated consumption, not holder authentication. Original R1-R4 and X-T66-003-DELAYED remain separately assessed.\n' ;p.write_text(a+'```json\n'+json.dumps(m,indent=2,ensure_ascii=False)+'\n```'+b,encoding='utf8')
    inspected=[('assessor/ownership/ownership.c',['Actual ow_move/ow_consume and OW_MUTANT3/13']),('labs/ownership/CONTRACTS.md',['Owner transfer and compiler diagnostic limits']),('assessor/ownership/analyses/X-T66-003.md',['Inline/self-referential and opaque changed assumptions']),('curriculum/v5/exercises/X-T66-003.md',['Original four criteria and delayed opaque representation'])]
    records=[]
    specs=[
      ('R-OMM-GCC-UNUSED','GCC Common Function Attributes: warn_unused_result','GNU Compiler Collection maintainers','GCC15.2 documentation',2025,'https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/Common-Function-Attributes.html',['warn_unused_result: warning when caller does not use result'],'Actual primary warn_unused_result passage lines744-756 read','Compiler extension comparison, not an ISO ownership guarantee.'),
      ('R-OMM-CLANG-UNUSED','Clang Attribute Reference: nodiscard and warn_unused_result','LLVM Project','Rolling primary documentation inspected2026-10-18',2026,'https://clang.llvm.org/docs/AttributeReference.html',['nodiscard, warn_unused_result: supported syntaxes and discarded-result diagnostics'],'Actual supported syntax and semantics lines3700-3710 read','Observed compiler versions and diagnostics recorded separately; no linear-use enforcement.'),
      ('R-OMM-N3096-NODISCARD','WG14 N3096 nodiscard attribute','ISO/IEC JTC1/SC22/WG14','N3096 C23 draft; not published C23',2023,'https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3096.pdf',['6.7.12.2p1-5: nodiscard constraints and recommended diagnostic practice'],'Actual cached draft text lines7387-7425 read','Recommended warning is not a required ownership type constraint; published C17/C23 unavailable.')]
    for identity,title,author,edition,year,url,locators,method,limits in specs:
        inspection=dict(date='2026-10-18',method=method,scope='The exact registered diagnostic passages only',limits=limits)
        if identity=='R-OMM-N3096-NODISCARD':inspection['local_cache_sha256']=sha(ROOT/'build/allocation-depth/n3096.txt')
        records.append(dict(id=identity,title=title,authors=[author],edition=edition,year=year,kind='standards-draft' if identity=='R-OMM-N3096-NODISCARD' else 'official-documentation',url=url,access='Actual primary passages inspected',domains=['T66'],role='diagnostic-enforcement-comparison',reason='Distinguish ISO structure copying, recommended diagnostics, compiler extensions and explicit runtime ownership protocols.',prerequisites=['Owner transfer and C translation diagnostics'],locators=locators,assignable=True,inspection=inspection))
    m['source_records']=[r for r in m['source_records'] if not r['id'].startswith('R-OMM-')]+[dict(id=r['id'],path=source,passages=r['locators']) for r in records]
    (ROOT/'curriculum/v5/exercises/X-T66-003.md').write_text(a+'```json\n'+json.dumps(m,indent=2,ensure_ascii=False)+'\n```'+b,encoding='utf8')
    dump(ROOT/source,dict(schema_version=1,resources=records,inspected_artifacts=[dict(path=p,sha256=sha(ROOT/p),passages=s) for p,s in inspected],reference_gaps=['Published C17 and C23 remain inaccessible.','No independent human/learner or physical execution evidence.']))
    dump(ROOT/gate,dict(status='passage-inspected',domains=['T66'],reviewer='author only',research_input_sha256={p:sha(ROOT/p) for p in [source,contract]}))
    imports=['labs/ownership/CONTRACTS.md','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','labs/ownership/src/ownership.c','assessor/ownership/ownership.c',source,gate,'tools/register_ownership_move.py']
    dump(ROOT/public/'campaign.json',dict(module='move',target='ownership-move',learner_option='CMASTERY_OMM_LEARNER',anchors=ANCHORS,control_count=6,observation_markers=MARKERS,oracle='tools/verify_ownership_move.py',metrics=['metrics'],object_sources=['move.c','registry.c','ownership.c'],producer='tools/run_ownership_move.py',report='research/v5/modern/ownership_move_validation.json',imports=imports))
if __name__=='__main__':register()
