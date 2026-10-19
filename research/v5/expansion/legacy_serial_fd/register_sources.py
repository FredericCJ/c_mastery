"""Register selected actually read current/historical primary passages."""
import hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 specs=[('R-FD-ISO2024','Road vehicles — Controller area network — Part1','ISO11898-1:2024 third edition',2024,'ISO','https://www.iso.org/standard/86384.html','build/can-source-access/iso11898-1-2024.pdf',[
 'Printed26–27 §6.6.4.4: CRC polynomials, initialization and recurrence',
 'Printed33–36 §6.6.11: base/extended FD fields, current ESI request relation, data and protected count/CRC; figures18–20 and22–23 visually inspected',
 'Printed42 §6.6.13: dynamic/fixed stuffing and single fixed bit at data/CRC boundary',
 'Printed56–58 §7.3.4: measured delay, secondary sample, following ordinary reaction and end-of-data-phase limits']),
 ('R-FD-BOSCH2012','CAN with Flexible Data-Rate Specification','Version1.0 released17April2012',2012,'Robert Bosch GmbH','https://web.archive.org/web/20151211125301id_/http://www.bosch-semiconductors.de/media/ubk_semiconductors/pdf_1/canliteratur/can_fd_spec.pdf','build/can-fd-source-access/bosch-preiso.pdf',[
 'Title/release and printed11–15 §3.2: control/DLC/data, zero CRC initialization and CRC/ACK/EOF fields',
 'Printed22 §5: historical dynamic and fixed CRC stuffing; no protected stuff-count field']),
 ('R-FD-BOSCH2015','Advantages of CAN FD Error detection mechanisms compared to Classical CAN','iCC2015',2015,'Arthur Mutter and Florian Hartwich, Robert Bosch GmbH','https://www.bosch-semiconductors.com/media/ip_modules/pdf_2/papers/icc_2015_mutter_1.pdf','build/can-fd-source-access/bosch-2015.pdf',[
 'Printed2–3 §§1.3–1.7: CRC/ACK, dynamic/fixed stuff, protected count introduction and bounded error-model claims']),
 ('R-FD-TCAN4550','TCAN4550 CAN FD controller with integrated transceiver','SLLSF91A revisedJanuary2020',2020,'Texas Instruments','https://www.ti.com/lit/ds/symlink/tcan4550.pdf','build/can-fd-source-access/tcan4550.pdf',[
 'Printed5 §6.4: recommended VSUP and VIO supply domains',
 'Printed41–43 §§8.5–8.5.2: MRAM/ECC initialization, mode0, word/partial transaction semantics, alignment/map and access opcodes/word counts']),
 ('R-FD-MCP2518','MCP2518FD External CAN FD Controller with SPI Interface','DS20006027B',2020,'Microchip Technology','https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/External-CAN-FD-Controller-with-SPI-Interface-DS20006027B.pdf','build/can-fd-source-access/mcp2518fd.pdf',[
 'Printed5 §1.3: typical3.3V controller/logic and separate5V transceiver supply',
 'Printed70–72 §§4.1–4.3.1: byte SFR, aligned word RAM, command/address/partial-word and opening CRC instruction scope',
 'Printed78 table7-1: supply and digital input/output recommended operating characteristics'])]
 rows=[]
 for ident,title,edition,year,author,url,capture,locators in specs:
  rows.append(dict(id=ident,title=title,authors=[author],edition=edition,year=year,kind='standard' if author=='ISO' else 'primary-document',url=url,access='Actual selected primary passages read; licensed ISO bytes remain private',domains=['T38'],role='primary',reason='Separate exact historical/current wire fields, metadata and selected controller/power contracts',prerequisites=['Classic CAN, CRC and SPI transactions','C17 fixed storage'],locators=locators,assignable=True,inspection=dict(date='2026-10-18',method='Selected primary PDF text read; named current ISO figures rendered and viewed; archived original Bosch2012 publication retrieved successfully',scope=locators,limits='Selected passages only; no full MAC, physical controller or silicon errata inspection',capture=dict(path=capture,sha256=sha(ROOT/capture)))))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8')
 private=[ROOT/'build/can-fd-source-access/catalog_search.json',ROOT/'build/can-source-access/source_identity.json']
 (HERE/'source_access.json').write_text(json.dumps(dict(private_provenance={p.relative_to(ROOT).as_posix():sha(p) for p in private},retrieval_note='Exact historical original archived Bosch URL fetched by Invoke-WebRequest after web open failed; successful PDF title and actual clauses inspected. No library mutation.'),indent=2)+'\n',encoding='utf8')
 gate=dict(schema_version=1,status='passage-inspected',scope='PR-T38-10 ordinary/changed authoring; not independent or physical acceptance',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'source_access.json',HERE/'audit.md',pathlib.Path(__file__)]},limitations=['No physical compatible-peer exchange or independent learner/external review.','MCP SPI CRC-extended commands and complete driver outside these inspected construction scopes.','Exact published ISO C17/C23 access not established by these CAN sources.'])
 (HERE/'research_gate.json').write_text(json.dumps(gate,indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
