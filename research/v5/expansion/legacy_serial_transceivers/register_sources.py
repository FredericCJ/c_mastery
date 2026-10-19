"""Register only named actually inspected primary passages and private captures."""
import hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 specs=[
 ('R-TR-MAX','MAX3232 line driver and receiver','SLLS410O June2021',2021,'max3232',['Printed3–6: pins, recommended operating conditions, driver/receiver guaranteed extrema and switching test conditions','Printed10–12: inversion/open state, Figure9-1 circuit visually inspected and Table9-1 capacitor selection']),
 ('R-TR-DRIVER','AM26LV31E differential line driver','SLLS848C April2024',2024,'am26lv31e',['Printed3–6: selected SOIC pin diagram visually inspected, recommended operating limits and output/load conditions','Printed10–13: enable truth table, one-driver termination and declared example limitations; pin-table5V label differs from3.0..3.6V operating table']),
 ('R-TR-RECEIVER','AM26LV32E differential receiver','SLLS849E August2023',2023,'am26lv32e',['Printed2–6: revision removal of short/terminated failsafe, selected SOIC pin diagram visually inspected, differential/common-mode and threshold limits','Printed8–9: open-input guarantee and enabled/disabled/indeterminate truth table'])]
 rows=[]
 for ident,title,edition,year,name,locators in specs:
  capture='build/transceiver-sources/'+name+'.pdf'
  rows.append(dict(id=ident,title=title,authors=['Texas Instruments'],edition=edition,year=year,kind='datasheet',url='https://www.ti.com/lit/ds/symlink/'+name+'.pdf',access='Actual selected primary PDF passages inspected',domains=['T38'],role='primary',reason='Separate selected electrical guarantees from original synthetic teaching models',prerequisites=['T38 UART/transceiver separation','C17 exact integer operations'],locators=locators,assignable=True,inspection=dict(date='2026-10-18',method='Actual primary web PDF text read; named private cached diagrams rendered and viewed',scope=locators,limits='Selected passages only; no physical measurement or full TIA standard inspection',capture=dict(path=capture,sha256=sha(ROOT/capture)))))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8')
 gate=dict(schema_version=1,status='passage-inspected',scope='T38.07–.08 bounded author investigation, not external acceptance',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md',pathlib.Path(__file__)]},limitations=['No physical circuit/cable/probe measurements, independent learner or external review.','Exact published ISO C17/C23 and full TIA standards are not supplied by these device sources.'])
 (HERE/'research_gate.json').write_text(json.dumps(gate,indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
