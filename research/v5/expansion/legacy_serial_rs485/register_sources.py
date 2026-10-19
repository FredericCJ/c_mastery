"""Register actually read selected primary passages without publishing extracts."""
import hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 specs=[('R-R5-1410','THVD1410 differential bus transceiver','SLLSEY3E May2019',2019,'https://www.ti.com/lit/ds/symlink/thvd1410.pdf','build/transceiver-sources/thvd1410.pdf',[
 'Printed5,9–11: SOIC pin functions, recommended limits, receiver thresholds and condition-specific driver/receiver enable/disable timing',
 'Printed19–21,23–24: selected THVD1410 operation, enable truth table, failsafe and eighth-unit loading; other family diagrams not adopted']),
 ('R-R5-1400','THVD1400 differential bus transceiver','SLLSF78B October2021',2021,'https://www.ti.com/lit/ds/symlink/thvd1400.pdf','build/transceiver-sources/thvd1400.pdf',[
 'Printed3–7: selected pin functions, recommended bus input/differential/load conditions and condition-specific enable/disable timing',
 'Printed13–14,16: functional operation, failsafe and one-eighth-unit load scope']),
 ('R-R5-GPIO','STM32F4 selected GPIO and SRAM mapping','RM0090 Rev21',2024,'https://www.st.com/resource/en/reference_manual/dm00031020.pdf','build/legacy_runtime_sources/rm0090.pdf',[
 'Printed68: SRAM mapping and CPU-only CCM compared with AHB access',
 'Printed284–285,287–289: mode, output type/speed/pull, BSRR write semantics and alternate-function register offsets'])]
 rows=[]
 for ident,title,edition,year,url,capture,locators in specs:
  rows.append(dict(id=ident,title=title,authors=['Texas Instruments' if ident!='R-R5-GPIO' else 'STMicroelectronics'],edition=edition,year=year,kind='datasheet' if ident!='R-R5-GPIO' else 'reference-manual',url=url,access='Actual selected cached primary PDF passages read',domains=['T38'],role='primary',reason='Separate selected physical timing/register premises from C model and actual adapter behavior',prerequisites=['T38 UART ownership','C17 fixed storage and modular arithmetic'],locators=locators,assignable=True,inspection=dict(date='2026-10-18',method='Actual selected PDF pages extracted with pdftotext and read; no bibliography-only inspection',scope=locators,limits='Selected passages only; no physical bus, full TIA standard or board-specific errata inspection',capture=dict(path=capture,sha256=sha(ROOT/capture)))))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8')
 gate=dict(schema_version=1,status='passage-inspected',scope='T38.09 additive author work; not external acceptance',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md',pathlib.Path(__file__)]},limitations=['Physical board/cable/load and instrument observations remain required.','Exact published ISO C17/C23 and full TIA standards are not supplied by these primary device passages.'])
 (HERE/'research_gate.json').write_text(json.dumps(gate,indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
