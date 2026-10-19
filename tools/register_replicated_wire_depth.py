"""Preserve required T76.08 nested grammar, ownership and evolution support."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ANCHORS=['replicated.'+a for a in ['wire_nested','wire_limits','wire_stream','wire_evolution']]
LOCATORS={
 'R-RP-WIRE':['Length-Delimited Records and Submessages; Last One Wins; Field Order and Implications'],
 'R-RP-WIRE-EVOLUTION':['Implications on Fingerprinting and Cryptographic Hashing; Deterministic is not Canonical; Inherent Barriers to Stable Serialization','proto2 Language Guide: Unknown Fields and Retaining Unknown Fields'],
 'R-RP-UTF8':['Section3 scalar ranges, octet construction and surrogate exclusion']}
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def dump(p,x):(ROOT/p).write_text(json.dumps(x,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
def register():
    family=read('labs/replicated/family.json');family['tests']=list(dict.fromkeys(family['tests']+ANCHORS))
    for row in family['projects']+family['exercises']:
        if row['id'] in ['PR-T76-08','X-T76-008']:row['tests']=list(dict.fromkeys(row['tests']+ANCHORS))
    family['campaign']['wire_depth_producer']='tools/run_replicated_wire_depth.py';family['campaign']['wire_depth_report']='research/v5/modern/replicated_wire_depth_validation.json';dump('labs/replicated/family.json',family)
    p=ROOT/'labs/replicated/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/wire_depth/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    p=ROOT/'curriculum/v5/exercises/X-T76-008.md';body=p.read_text(encoding='utf8');before,rest=body.split('```json\n',1);raw,after=rest.split('\n```',1);meta=json.loads(raw)
    meta['lab']['tests']=list(dict.fromkeys(meta['lab']['tests']+ANCHORS));meta['shared_contracts']=list(dict.fromkeys(meta['shared_contracts']+['labs/replicated/wire_depth/WIRE.md']));meta['fixtures']=list(dict.fromkeys(meta['fixtures']+['fixtures/replicated/wire_depth.json']))
    for ident,locators in LOCATORS.items():
        row=next((r for r in meta['source_records'] if r['id']==ident),None)
        if row:row['passages']=list(dict.fromkeys(row['passages']+locators))
        else:meta['source_records'].append(dict(id=ident,path='research/v5/modern/replicated_resources.json',passages=locators))
    h='\n## Required nested-wire and relay implementation\n';after=after.split(h,1)[0].rstrip()+h+'''

Implement [WIRE.md](../../../labs/replicated/wire_depth/WIRE.md): the explicitly
selected RPW1 profile, globally bounded nested records, required/advisory feature
admission, actual PR-T24-03 incremental framing, owned partial bytes and retained
unknown fields. Four anchors and ten semantic controls accompany6,102 independent
wire rows and2,112 bit/UTF-8 comparisons. Show why field-wise reconstruction loses
information and why future interpretation of opaque bytes changes the identity
domain. This distinct profile is not an unversioned reinterpretation of the
original flat tag3 schema. Preserve all original R1–R4 obligations and
X-T76-008-DELAYED; author support does not manufacture the delayed attempt.
'''
    p.write_text(before+'```json\n'+json.dumps(meta,indent=2,ensure_ascii=False)+'\n```'+after,encoding='utf8')
    p=ROOT/meta['assessor_analysis'];body=p.read_text(encoding='utf8');h='\n## Nested schema and incremental ownership analysis\n';p.write_text(body.split(h,1)[0].rstrip()+h+'\nSee [ANALYSIS.md](../wire_depth/ANALYSIS.md) for exact budget, publication, identity-domain and actual framing arguments.\n',encoding='utf8')
    dump('fixtures/replicated/wire_depth.json',dict(exercise='X-T76-008',profile='RPW1 selected out of band',max_depth=4,max_raw_bytes=256,max_fields=32,max_record_fields=8,max_arena_records=16,max_opaque_bytes=32,max_framed_bytes=289,
      examples=[dict(input_hex='10010896011a0410000802',canonical_hex='08960110011a0408021000'),dict(input_hex='080110012804',supported=3,status='REQUIRED'),dict(input_hex='080110013004',supported=3,status='OK advisory retained')],
      wire_rows=6102,distinct_canonical_values=1847,normalization_rows=2112,
      limits=['Fixture counts specify experiments; actual execution requires the matching source-bound report.','Schema/version selection, origin authentication and future Unicode normalization are not inferred from feature-bit admission.','No live network, physical target or unaided learner result is inferred.']))
if __name__=='__main__':register()
