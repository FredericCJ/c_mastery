"""Canonical additive attachment of exact original X41/X42 variants."""
import json
from common import ROOT,dump,sha
BASE='research/v5/expansion/network_http_variants'
IDS=['X-T41-001','X-T42-001','X-T42-002','X-T42-003','X-T42-004']
PUBLIC='labs/network_http_variants'
LOCATORS=['Independent checksum segments — X-T41-001 R1/V1/V2','Declared wire maxima — X-T42-002 R1–R3/V1/V2','Calendar cycles and source authority — X-T42-003 R1–R3/V1/V2','Concurrent descriptor ownership — X-T42-004 R1–R3/V1/V2','Retained framing and execution evidence','Deliverables, rubric, hints and changed conditions']
def main():
    contract=PUBLIC+'/CONTRACTS.md'
    def record(id,title,url,edition,authors,locators,limits):
        return dict(id=id,title=title,url=url,authors=authors,edition=edition,year=2026 if url.startswith('labs/') else None,kind='manual' if url.startswith('labs/') else 'documentation',access='Named actual passages inspected',domains=['T41','T42'],role='primary',assignable=True,reason='Exact checksum algebra, byte/date grammar and Linux ownership mechanism support.',prerequisites=['T03','T04','T23','T24','T41','T42'],locators=locators,inspection=dict(date='2026-10-18',method='Actual named primary web passages and local source/contract reading',scope='; '.join(locators),limits=limits))
    records=[record('R-NV-CONTRACT','Original network and HTTP changed mechanisms',contract,'Original companion 2026-10-18',['Curriculum authors'],LOCATORS,'Author support only; no physical or independent learner observation.'),
      record('R-NV-RFC1071','Computing the Internet Checksum','https://www.rfc-editor.org/rfc/rfc1071','RFC1071 September1988',['R. Braden','D. Borman','C. Partridge'],['1: byte pairing, final odd zero and end-around carry','2(A)–(B): parity-respecting splitting and odd-boundary byte swap','3: odd-boundary numerical example'],'Informational techniques, not complete protocol conformance or endorsement of historical pointer casts.'),
      record('R-NV-RFC9110-DATE','HTTP Semantics date grammar','https://www.rfc-editor.org/rfc/rfc9110.txt','RFC9110 June2022',['R. Fielding','M. Nottingham','J. Reschke'],['5.6.7: IMF-fixdate grammar and 00:00:00 through 23:59:60'],'Selected date grammar only; no current clock or historical bulletin attestation.'),
      record('R-NV-RFC3339-LEAP','Date and Time on the Internet: Timestamps','https://www.rfc-editor.org/rfc/rfc3339','RFC3339 July2002',['G. Klyne','C. Newman'],['5.7: leap insertion/deletion limits and announcement authority','Appendix D opening: UTC day-end positive leap representation and TAI distinction'],'Reading its source-time premise does not reinterpret HTTP wire syntax as RFC3339. No current bulletin inspected.'),
      record('R-NV-LINUX-CLOSE','Linux close(2)','https://man7.org/linux/man-pages/man2/close.2.html','Linux man-pages6.19 February2026',['Linux man-pages project'],['CAVEATS: multithreaded open-file-description lifetime','CAVEATS: errors, early descriptor release and nonretry on Linux','CAVEATS final paragraph: POSIX.1-2024 EINTR policy differs from Linux'],'Linux-specific descriptor ownership; no generic POSIX close-EINTR equivalence. Earlier standards/current Issue8 remain separate.')]
    records[0]['inspection']['local_sha256']=sha(ROOT/contract)
    for r in records:
        if r['year'] is None:r['year']={'R-NV-RFC1071':1988,'R-NV-RFC9110-DATE':2022,'R-NV-RFC3339-LEAP':2002,'R-NV-LINUX-CLOSE':2026}[r['id']]
    dump(ROOT/BASE/'resources.json',dict(resources=records))
    passages={contract:LOCATORS,'assessor/network_http_variants/ANALYSIS.md':['All exact original roots/rubric/V1/V2 derivations'],'assessor/network_http_variants/variant.c':['Complete checksum,declared-limit,cycle and source-authority adapters'],'assessor/network_http_variants/owned.c':['Complete Linux owned descriptor adapter'],'assessor/b045_systems/systems.c':['s_checksum_add and s_checksum_finish only'],'assessor/http_depth/http_depth.c':['Complete fixed-domain numeral/date and original serial pool implementation'],'assessor/hosted_followthrough/http/flow.c':['Complete struct hf_pool/response/flow implementation'],'labs/hosted_followthrough/http/CONTRACTS.md':['Complete exact successor contract'],'labs/hosted_followthrough/http/parser_cases.py':['Actual fixed expectations and all-split corpus'],'labs/hosted_followthrough/http/avr_length.c':['Actual fixed-wire-domain16bit extent assertions and target cases']}
    for id in IDS:passages[f'curriculum/v5/exercises/{id}.md']=['Original complete metadata,body,rubric and variants']
    assets=[p.relative_to(ROOT).as_posix() for p in (ROOT/PUBLIC).iterdir() if p.is_file()]
    assets+=['labs/b045_systems/include/systems.h','labs/b045_systems/src/systems.c','labs/http_depth/include/http_depth.h','labs/http_depth/src/http_depth.c','labs/hosted_followthrough/http/flow.h','labs/hosted_followthrough/http/flow.c','labs/communications/include/com_http.h','labs/hosted_followthrough/http/avr_start.S','labs/hosted_followthrough/http/run_avr.py']
    for id in IDS:
        p=ROOT/f'curriculum/v5/exercises/{id}.md';text=p.read_text(encoding='utf8');head,rest=text.split('```json\n',1);encoded,tail=rest.split('\n```',1);d=json.loads(encoded)
        d['fixtures']=list(dict.fromkeys(d.get('fixtures',[])+assets));d['shared_contracts']=list(dict.fromkeys(d.get('shared_contracts',[])+[contract]))
        relevant={'R-NV-CONTRACT'}|({'R-NV-RFC1071'} if id=='X-T41-001' else {'R-NV-RFC9110-DATE','R-NV-RFC3339-LEAP'} if id=='X-T42-003' else {'R-NV-LINUX-CLOSE'} if id=='X-T42-004' else set())
        d['source_records']=[r for r in d['source_records'] if not r['id'].startswith('R-NV-')]+[dict(id=r['id'],path=BASE+'/resources.json',passages=r['locators']) for r in records if r['id'] in relevant]
        d['status']['required_followthrough']=dict(contract=contract,readiness=BASE+'/readiness.json',report=BASE+'/validation.json',export_report=BASE+'/export_validation.json',scope='Exact original root,rubric,V1,V2 author support; external and independent learner evidence remain separate.')
        marker='\n## Required original network and HTTP variants\n'
        if marker in tail:tail=tail.split(marker,1)[0]
        tail+=marker+'\nComplete the applicable [required companion](../../../labs/network_http_variants/CONTRACTS.md), preserving this original gate and V1/V2. It supplies parity-aware independent checksum composition, arbitrary declared numeric limits, Gregorian cycles/leap-source authority and concurrent descriptor ownership. The earlier framing and AVR implementations are explicit dependencies; author execution does not award learner mastery.\n'
        p.write_text(head+'```json\n'+json.dumps(d,ensure_ascii=False,indent=2)+'\n```'+tail,encoding='utf8')
    dump(ROOT/BASE/'research_gate.json',dict(status='passage-inspected',research_input_sha256={BASE+'/resources.json':sha(ROOT/BASE/'resources.json')},resources=BASE+'/resources.json',input_sha256={p:sha(ROOT/p) for p in passages},passages=passages,limits=['Exact published C17/C23 access remains open; C17 implementation baseline is retained.','Primary URL readings are selected passages, not whole-document inspection.','Original baseline identities and acceptance receipts remain unchanged.']))
    print(dict(attached=IDS,assets=len(assets)))
if __name__=='__main__':main()
