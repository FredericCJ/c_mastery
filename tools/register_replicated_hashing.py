"""Preserve the required streaming primitive and actual inspected references."""
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ANCHORS=['replicated.'+x for x in ['sha_stream','hash_hmac','hash_lifecycle','hash_security']]
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def dump(p,x):(ROOT/p).write_text(json.dumps(x,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
def register():
    resources=read('research/v5/modern/replicated_resources.json')
    for r in resources['resources']:
        if r['id']=='R-RP-SHA':
            r.update(authors=['Donald Eastlake 3rd','Tony Hansen'],year=2011,edition='RFC6234, May2011; sections4.1,5.1,6.1,6.2 reinspected2026-10-17')
            r['inspection']['evidence_urls']=list(dict.fromkeys(r['inspection']['evidence_urls']+['https://www.rfc-editor.org/rfc/rfc6234.txt']))
    new=[('R-RP-HMAC','HMAC: Keyed-Hashing for Message Authentication',['Hugo Krawczyk','Mihir Bellare','Ran Canetti'],1997,'2104',
          'Section2 HMAC definition; section3 keys; section4 implementation note; section6 final paragraph on implementation and key protection'),
         ('R-RP-HMAC-VECTORS','Identifiers and Test Vectors for HMAC-SHA-224, HMAC-SHA-256, HMAC-SHA-384, and HMAC-SHA-512',['Magnus Nystrom'],2005,'4231',
          'Sections4.2,4.3,4.7 SHA256 full-output test cases1,2,6; section5 limits of the test-vector security claim')]
    for ident,title,authors,year,number,locator in new:
        item=dict(id=ident,title=title,authors=authors,year=year,edition='RFC'+number+'; named passages inspected2026-10-17',kind='manual',
          url='https://www.rfc-editor.org/rfc/rfc'+number+'.txt',access='Public primary passages inspected',domains=['T76'],role='oracle',
          reason='First-principles keyed-hash construction and external expected values; no production certification or authorization inference.',
          prerequisites=['C unsigned arithmetic, byte spans, streaming compression and key ownership'],locators=[locator],assignable=True,
          inspection=dict(date='2026-10-17',method='Opened and read the named primary RFC passages',scope=locator,
          limits='Bounded author inspection; not complete cryptographic proof, independent implementation review or final ISO C source.',
          evidence_urls=['https://www.rfc-editor.org/rfc/rfc'+number+'.txt']))
        resources['resources']=[r for r in resources['resources'] if r['id']!=ident]+[item]
    dump('research/v5/modern/replicated_resources.json',resources)
    gate=read('research/v5/modern/replicated_research_gate.json');p='research/v5/modern/replicated_resources.json';gate['research_input_sha256'][p]=hashlib.sha256((ROOT/p).read_bytes()).hexdigest();dump('research/v5/modern/replicated_research_gate.json',gate)
    family=read('labs/replicated/family.json');family['tests']=list(dict.fromkeys(family['tests']+ANCHORS))
    for row in family['projects']+family['exercises']:
        if row['id'] in ['PR-T76-07','X-T76-007']:row['tests']=list(dict.fromkeys(row['tests']+ANCHORS))
    family['campaign']['hashing_producer']='tools/run_replicated_hashing.py';family['campaign']['hashing_report']='research/v5/modern/replicated_hashing_validation.json';dump('labs/replicated/family.json',family)
    p=ROOT/'labs/replicated/CMakeLists.txt';body=p.read_text(encoding='utf8');line='include("${CMAKE_CURRENT_SOURCE_DIR}/hashing/CMakeLists.txt")'
    if line not in body:p.write_text(body.rstrip()+'\n'+line+'\n',encoding='utf8')
    p=ROOT/'curriculum/v5/exercises/X-T76-007.md';before,rest=p.read_text(encoding='utf8').split('```json\n',1);raw,after=rest.split('\n```',1);meta=json.loads(raw)
    meta['lab']['tests']=list(dict.fromkeys(meta['lab']['tests']+ANCHORS));meta['shared_contracts']=list(dict.fromkeys(meta['shared_contracts']+['labs/replicated/hashing/HASHING.md']));meta['fixtures']=list(dict.fromkeys(meta['fixtures']+['fixtures/replicated/hashing.json']))
    for ident,title,authors,year,number,locator in new:
        meta['source_records']=[r for r in meta['source_records'] if r['id']!=ident]+[dict(id=ident,path='research/v5/modern/replicated_resources.json',passages=[locator])]
    heading='\n## Required streaming hash and authentication primitive\n'
    after=after.split(heading,1)[0].rstrip()+heading+'''

Implement [HASHING.md](../../../labs/replicated/hashing/HASHING.md), including
both message schedules, streaming/final-state ownership, full-width length
admission, HMAC long-key reduction and the original local length-extension
counterexample. Four named anchors and eight controls supplement the original
Merkle tests. Preserve all original inclusion-proof, framed-repair and delayed
obligations; a passing primitive campaign does not complete them.
'''
    p.write_text(before+'```json\n'+json.dumps(meta,indent=2,ensure_ascii=False)+'\n```'+after,encoding='utf8')
    p=ROOT/meta['assessor_analysis'];body=p.read_text(encoding='utf8');h='\n## Streaming primitive analysis\n';p.write_text(body.split(h,1)[0].rstrip()+h+'\nSee [ANALYSIS.md](../hashing/ANALYSIS.md) for schedule, length, HMAC and local forgery arguments, with explicit residual tree/repair obligations.\n',encoding='utf8')
    dump('fixtures/replicated/hashing.json',dict(exercise='X-T76-007',shared_support_for='T76.04 authenticated admission',
         byte_lengths=[0,1,54,55,56,57,63,64,65,66,119,120,127,128,129,1024,8193],key_lengths=[0,1,20,32,63,64,65,131,257],
         published_cases=[dict(rfc=4231,case=1,sha256='b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7'),
                          dict(rfc=4231,case=2,sha256='5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843'),
                          dict(rfc=4231,case=6,sha256='60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54')],
         expected_oracle_cases=1786,expected_digest_comparisons=7144,schedules=[16,64],
         limits=['Fixture counts describe required experiments; execution requires current source-bound evidence.',
                 'Synthetic large counts test encoding and refusal, not processing exabyte inputs.']))
if __name__=='__main__':register()
