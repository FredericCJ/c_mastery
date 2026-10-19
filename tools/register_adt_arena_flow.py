"""Canonical T67.07 whole-arena and finite fixed-point successor."""
import json
from common import ROOT,dump,read_json,sha

def register():
    anchors=['arena_certificate','arena_worklist','arena_outcomes','arena_base'];tests=['adt.'+a for a in anchors]
    f=read_json(ROOT/'labs/adt/family.json')
    for row in f.get('projects',[])+f.get('exercises',[]):
        if row['id'] in ['PR-T67-07','X-T67-007']:row['tests']=list(dict.fromkeys(row['tests']+tests))
    f['tests']=list(dict.fromkeys(f.get('tests',[])+tests));f.setdefault('campaign',{}).update(arena_flow_producer='tools/run_adt_arena_flow.py',arena_flow_report='research/v5/modern/adt_arena_flow_validation.json');dump(ROOT/'labs/adt/family.json',f)
    p=ROOT/'labs/adt/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/arena_flow/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    source='research/v5/modern/adt_arena_flow_resources.json';contract='labs/adt/arena_flow/FLOW.md'
    r=dict(id='R-AD-FLOW-CLANG',title='Clang Data flow analysis: an informal introduction',authors=['Clang contributors'],edition='Rolling primary documentation inspected2026-10-18',year=2026,kind='manual',url='https://clang.llvm.org/docs/DataFlowAnalysisIntro.html',access='public primary passages inspected',domains=['T67'],role='theory',reason='Separate finite-height monotone convergence from incomplete computation or arbitrary nonmonotone equations.',prerequisites=['Sets, partial orders and graph edges'],locators=['Formalization: transfer/join equations, finite-height monotone convergence and worklist'],assignable=True,inspection=dict(date='2026-10-18',method='Opened primary documentation and read actual lines167-329.',scope='Finite set approximation, top/bottom, transfer/join equations, finite-height monotone termination and worklist scheduling.',limits='Named introductory passages only; no certification of the original transfer functions, solver or arbitrary-language soundness.'))
    dump(ROOT/source,dict(schema_version=1,resources=[r],original_construction='Owned whole-arena certificate, eight-fact copied session, least-fixed-point proof and separate exhaustive Boolean search are original bounded teaching mechanisms.'))
    gate='research/v5/modern/adt_arena_flow_research_gate.json';dump(ROOT/gate,dict(status='passage-inspected',domains=['T67'],reviewer='author only; no independent review',research_input_sha256={p:sha(ROOT/p) for p in [source,contract,'research/v5/modern/adt_resources.json']}))
    p=ROOT/'curriculum/v5/exercises/X-T67-007.md';body=p.read_text(encoding='utf8');a,rest=body.split('```json\n',1);raw,b=rest.split('\n```',1);m=json.loads(raw);m['lab']['tests']=list(dict.fromkeys(m['lab']['tests']+tests));m['shared_contracts']=list(dict.fromkeys(m['shared_contracts']+[contract]))
    if not any(x['id']==r['id'] for x in m['source_records']):m['source_records'].append(dict(id=r['id'],path=source,passages=r['locators']))
    heading='\n## Required owned-arena and cyclic-analysis successor\n';b=b.split(heading,1)[0].rstrip()+heading+'\n[FLOW.md](../../../labs/adt/arena_flow/FLOW.md) specifies the actual retained validator import, owned certificate, bounded cyclic worklist, resumable budget and independent exhaustive Boolean solver. The latter distinguishes proved nonexistence from an unfinished monotone computation. All four original rubric criteria and the delayed variant remain required.\n';p.write_text(a+'```json\n'+json.dumps(m,indent=2,ensure_ascii=False)+'\n```'+b,encoding='utf8')
    imports=['labs/adt/include/adt.h','labs/adt/src/adt.c','assessor/adt/adt.c','labs/adt/CONTRACTS.md',source,gate,'tools/register_adt_arena_flow.py']
    dump(ROOT/'labs/adt/arena_flow/campaign.json',dict(module='arena_flow',target='adt-arena_flow',learner_option='CMASTERY_ADF_LEARNER',anchors=anchors,control_count=8,observation_markers=['ADF_ARENA whole-owned-certificate','ADF_FLOW cyclic-union-reschedule owned-reverse-ring','ADF_OUTCOME budget-not-no-fixed-point oscillation-with-solutions','ADF_BASE actual-whole-arena'],oracle='tools/verify_adt_arena_flow.py',metrics=['metrics'],object_sources=['adt.c','arena_flow.c'],producer='tools/run_adt_arena_flow.py',report='research/v5/modern/adt_arena_flow_validation.json',imports=imports))
if __name__=='__main__':register()
