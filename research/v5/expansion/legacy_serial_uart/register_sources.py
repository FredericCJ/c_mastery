"""Selected inspected passages only; no bibliography-as-inspection inference."""
import hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def main():
 entries=[
 ('R-UA-RM21','F439 memory, USART format and completion','STMicroelectronics','RM0090 Rev21 June2024',2024,'https://www.st.com/resource/en/reference_manual/dm00031020.pdf',['Printed p68: SRAM bus-master access and CPU-only CCM','Printed pp973–977,981,991: transfer/parity, stop sampling, divisor and aggregate frequency deviation','Printed p1004: DMA completion followed by final USART TC','Printed pp1010–1017: SR/DR clearing, M/PCE/PS and stop configuration'],'Local pinned PDF text; relevant passages inspected'),
 ('R-UA-DS14','USART3 TX/RX alternate-function rows','STMicroelectronics','DS9484 Rev14',2026,'https://www.st.com/resource/en/datasheet/stm32f439zi.pdf',['Table12 printed pp78–79: AF column header and PD8/PD9 AF7 rows'],'Primary web PDF text inspected; no board route observation'),
 ('R-UA-CACHE','Cache visibility versus ownership','STMicroelectronics','AN4839 Rev2 March2018',2018,'https://www.st.com/resource/en/application_note/dm00272913-level-1-cache-on-stm32f7-series-and-stm32h7-series-stmicroelectronics.pdf',['Printed pp3–6: 32-byte lines, write-back behavior and DMA coherency; F7 DTCM example not generalized to H7'],'Primary web PDF named passages inspected in this author session; subsequent ST reads intermittently fail and private download timed out'),
 ('R-UA-MAX','UART controller and line-driver boundary','Texas Instruments','SLLS410O RevO June2021',2021,'https://www.ti.com/lit/ds/symlink/max3232.pdf',['Printed pp1,3–6,9: interface role, DIN/DOUT/ROUT/RIN, inversion, selected supply/load/signaling conditions'],'Actual primary web PDF text inspected'),
 ('R-UA-CAN','SPI versus CAN-controller ownership','Microchip Technology','DS20001801K',2021,'https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP2515-Family-Data-Sheet-DS20001801K.pdf',['Printed pp1,3–4: separate SPI/control/CAN modules and interface','Printed pp15–16: copied transmit buffers, local priority versus arbitration, request/completion and abort distinctions'],'Actual primary web PDF text inspected')]
 rows=[]
 for ident,title,author,edition,year,url,loc,method in entries:
  inspection=dict(date='2026-10-18',method=method,scope=loc,limits='Only named passages; no whole-document, external-review or physical claim')
  if ident=='R-UA-RM21':p=ROOT/'build/legacy_runtime_sources/rm0090.pdf';inspection['external_source_sha256']={str(p):sha(p)}
  rows.append(dict(id=ident,title=title,authors=[author],edition=edition,year=year,kind='manual',url=url,access='Named primary passages inspected',domains=['T38'],role='primary',reason='Separate C ownership, controller framing, DMA/cache and electrical evidence',prerequisites=['T27 layout and clock ledger','T28 UART producer'],locators=loc,assignable=True,inspection=inspection))
 write(HERE/'resources.json',dict(resources=rows));files=[HERE/'resources.json',HERE/'audit.md',pathlib.Path(__file__)];write(HERE/'research_gate.json',dict(status='passage-inspected',scope='Bounded author source inspection, not external assessment',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in files},limitations=['Physical capture remains required','Final published C17/C23 gaps unchanged','Transient AN4839 re-access failure does not expand the actual selected inspection']))
if __name__=='__main__':main()
