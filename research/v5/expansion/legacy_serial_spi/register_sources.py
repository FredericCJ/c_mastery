"""Bounded primary source inspection; no private extracts distributed."""
import hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def main():
 locators=['RM0090 Rev21 printed pp881–882 Figure248: all four SPI edge modes; p882 rendered and visually inspected','Printed p885 and pp889–891: master setup, full duplex, RX draining and TXE/BSY completion','Printed pp897–902 and919–923: DMA/disable/error transitions and legal SPI register access widths']
 pdf=ROOT/'build/legacy_runtime_sources/rm0090.pdf'
 r=dict(id='R-SS-RM21',title='SPI framing and completion evidence',authors=['STMicroelectronics'],edition='RM0090 Rev21 June2024',year=2024,kind='manual',url='https://www.st.com/resource/en/reference_manual/dm00031020.pdf',access='Named primary passages inspected',domains=['T38'],role='primary',reason='Separates controller sampling, memory ownership, wire completion and peer protocol',prerequisites=['Actual PsSpi/PuRates producer','T38.06 retained contract'],locators=locators,assignable=True,inspection=dict(date='2026-10-18',method='Actual pinned local primary PDF text and rendered Figure248 inspected',scope=locators,limits='Selected passages only; no physical, final ISO or independent assessment claim',external_source_sha256={str(pdf):sha(pdf)}))
 write(HERE/'resources.json',dict(resources=[r]));files=[HERE/'resources.json',HERE/'audit.md',pathlib.Path(__file__)]
 write(HERE/'research_gate.json',dict(status='passage-inspected',scope='Author inspection; not independent acceptance',resource_ids=[r['id']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in files},limitations=['Actual board/peer electrical observations remain required','Frozen producer errata/routing records are imported premises, not a fresh board inspection','Published final C17/C23 text not newly inspected']))
if __name__=='__main__':main()
