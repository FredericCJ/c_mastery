"""Actually inspected primary scopes; never publish the held standard's extracts."""
import hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 specs=[('R-CN-ISO2024','Road vehicles — Controller area network — Part1: Data link layer and physical coding sublayer','ISO11898-1:2024 third edition',2024,'ISO','https://www.iso.org/standard/86384.html','build/can-source-access/iso11898-1-2024.pdf',[
 'Printed66–71, clauses8.1.1–8.1.5: error-signalling states, all eight fault-counter rules and exceptions, passive/off transitions and restart conditions',
 'Printed31–33, clauses6.6.10–6.6.11 opening: Classic format, CRC/tail and current receiver limits; FD opening only',
 'Printed42–45, clauses6.6.13–6.6.19 opening: dynamic/fixed stuffing, ACK, validation, arbitration, restricted/monitoring modes and error handling opening']),
 ('R-CN-RM21','STM32F4 selected bxCAN controller','RM0090 Rev21 June2024',2024,'STMicroelectronics','https://www.st.com/resource/en/reference_manual/dm00031020.pdf','build/legacy_runtime_sources/rm0090.pdf',[
 'Printed65–67: selected memory/peripheral map including CAN1 APB1 boundary',
 'Printed1084–1092,1094–1097: test modes, TX/abort/NART, FIFO handling/overflow, filter organization, mailbox storage, bus-off recovery and bit timing',
 'Printed1100–1116: 32-bit access protection, control/status/timing and TX/RX mailbox registers',
 'Printed1117–1120: filter configuration, ownership, activation and mask register encoding'])]
 rows=[]
 for ident,title,edition,year,author,url,capture,locators in specs:
  rows.append(dict(id=ident,title=title,authors=[author],edition=edition,year=year,kind='standard' if author=='ISO' else 'reference-manual',url=url,access='Actually read selected primary passages; held standard remains private',domains=['T38'],role='primary',reason='Current fault rules and selected controller semantics, explicitly separated from local codec subset and physical evidence',prerequisites=['T38 Classic CAN and MMIO','C17 fixed storage and integer arithmetic'],locators=locators,assignable=True,inspection=dict(date='2026-10-18',method='Read selected pdftotext primary passages; exact held record resolved through pinned read-only library census',scope=locators,limits='Selected passages only, no full standard/figure/physical/errata inspection',capture=dict(path=capture,sha256=sha(ROOT/capture)))))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8')
 inputs=[HERE/'resources.json',HERE/'audit.md',pathlib.Path(__file__),ROOT/'build/can-source-access/catalog_pin.json',ROOT/'build/can-source-access/source_identity.json']
 # Research gate canonical inputs stay in this folder; private source bytes
 # and lookup identity are separately pinned in the resource/inspection record.
 provenance={p.relative_to(ROOT).as_posix():sha(p) for p in inputs[-2:]}
 (HERE/'source_access.json').write_text(json.dumps(dict(private_provenance=provenance,limits='Private catalog and held payload only; not distributed source extracts'),indent=2)+'\n',encoding='utf8')
 gate=dict(schema_version=1,status='passage-inspected',scope='PR-T38-09 six original criterion/V1 author mechanisms; not external acceptance',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [*inputs[:3],HERE/'source_access.json']},limitations=['No physical controller/bus captures or independent assessment.','No full current FD/pre-ISO comparison claimed in this package.','Exact published C17/C23 access remains outside these device/ISO CAN scopes.'])
 (HERE/'research_gate.json').write_text(json.dumps(gate,indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
