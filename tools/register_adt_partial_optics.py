"""Canonical T67.08 nested partial and version-preserving optic successor."""
import json
from common import ROOT,dump,read_json,sha

def register():
    anchors=['optics_nested','optics_prism','optics_effects','optics_base'];tests=['adt.'+a for a in anchors]
    f=read_json(ROOT/'labs/adt/family.json')
    for row in f.get('projects',[])+f.get('exercises',[]):
        if row['id'] in ['PR-T67-08','X-T67-008']:row['tests']=list(dict.fromkeys(row['tests']+tests))
    f['tests']=list(dict.fromkeys(f.get('tests',[])+tests));f.setdefault('campaign',{}).update(partial_optics_producer='tools/run_adt_partial_optics.py',partial_optics_report='research/v5/modern/adt_partial_optics_validation.json');dump(ROOT/'labs/adt/family.json',f)
    p=ROOT/'labs/adt/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/partial_optics/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    source='research/v5/modern/adt_partial_optics_resources.json';contract='labs/adt/partial_optics/OPTICS.md'
    dump(ROOT/source,dict(schema_version=1,resources=[],reused_sources=[dict(id='R-ADMP-LENS',path='research/v5/modern/adt_resources.json',passages=['Lens laws near lines85-106; Prism review/preview laws near lines345-365'],reread='2026-10-18',method='Opened actual maintainer Control.Lens.Type source; read lens equations/type at85-142 and prism equations/domain explanation345-375.',scope='Three lens equations, polymorphic type encoding, prism review/preview round trips and distinction from the type-changing matching residual.',limits='Bounded semantic/type comparison; no Haskell runtime execution or proof that arbitrary implementations satisfy the laws.')],original_construction='Two-level optional configuration, exact versioned error domains and at-most-two transactional-output callback sequence are original first-principles teaching contracts.'))
    gate='research/v5/modern/adt_partial_optics_research_gate.json';dump(ROOT/gate,dict(status='passage-inspected',domains=['T67'],reviewer='author only; no independent review',research_input_sha256={p:sha(ROOT/p) for p in [source,contract,'research/v5/modern/adt_resources.json']}))
    p=ROOT/'curriculum/v5/exercises/X-T67-008.md';body=p.read_text(encoding='utf8');a,rest=body.split('```json\n',1);raw,b=rest.split('\n```',1);m=json.loads(raw);m['lab']['tests']=list(dict.fromkeys(m['lab']['tests']+tests));m['shared_contracts']=list(dict.fromkeys(m['shared_contracts']+[contract]))
    heading='\n## Required nested optional and versioned-prism successor\n';b=b.split(heading,1)[0].rstrip()+heading+'\n[OPTICS.md](../../../labs/adt/partial_optics/OPTICS.md) supplies exact partial-domain laws, actual pair-lens composition, version/context-preserving IO focus and staged callback updates. Independent models check collateral fields and observable callback traces as well as ordinary values. Original criteria and delayed IDs remain active.\n';p.write_text(a+'```json\n'+json.dumps(m,indent=2,ensure_ascii=False)+'\n```'+b,encoding='utf8')
    imports=['labs/adt/include/adt.h','labs/adt/src/adt.c','assessor/adt/adt.c','labs/adt/CONTRACTS.md',source,gate,'tools/register_adt_partial_optics.py']
    dump(ROOT/'labs/adt/partial_optics/campaign.json',dict(module='partial_optics',target='adt-partial_optics',learner_option='CMASTERY_ADL_LEARNER',anchors=anchors,control_count=8,observation_markers=['ADL_NESTED partial-composition three-laws collateral-preserved','ADL_PRISM version-and-payload roundtrip partial-domain','ADL_EFFECT once-in-order external-effects-not-rollback','ADL_BASE actual-lens-prism laws'],oracle='tools/verify_adt_partial_optics.py',metrics=['metrics'],object_sources=['adt.c','partial_optics.c'],producer='tools/run_adt_partial_optics.py',report='research/v5/modern/adt_partial_optics_validation.json',imports=imports))
if __name__=='__main__':register()
