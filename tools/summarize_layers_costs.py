"""Compare emitted equal-contract fresh-context costs, not physical latency."""
from pathlib import Path
import hashlib,json,re
ROOT=Path(__file__).resolve().parents[1];RECEIPT='reviews/v5/expansion/legacy_advanced_layers/current_validation.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 d=json.loads((ROOT/RECEIPT).read_text());assert d['passed'];rows={r['label']:r for r in d['checks']};profiles=[]
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['-O0','-O3']:
   tag=cc+opt;comparisons=[]
   for kind in ['direct','generated','vendor']:
    r=rows[tag+' cost '+kind+' size'];text=(ROOT/r['raw']).read_text();sections=[dict(name=n,bytes=int(s),address=int(a)) for n,s,a in re.findall(r'^(\S+)\s+(\d+)\s+(\d+)\s*$',text,re.M)];assert sections
    symbols=(ROOT/rows[tag+' cost '+kind+' symbols']['raw']).read_text();assert 'pl_cost_'+kind in symbols
    comparisons.append(dict(kind=kind,sections=sections,retained_symbols=symbols,linked_imports=(ROOT/rows[tag+' cost '+kind+' closure']['raw']).read_text()))
   stack=[]
   for path,h in d['outputs_sha256'].items():
    if '/'+tag+'/' in path and path.endswith('.su'):
     assert sha(ROOT/path)==h
     for line in (ROOT/path).read_text().splitlines():
      parts=line.split('\t')
      if len(parts)==3:stack.append(dict(function=parts[0].split(':')[-1],local_frame_bytes=int(parts[1]),kind=parts[2],source=path))
   assert stack;profiles.append(dict(profile=tag,equal_contract_images=comparisons,local_stack_frames=stack,original_target_sections=(ROOT/rows[tag+' cost']['raw']).read_text()))
 out=dict(passed=True,input_sha256={RECEIPT:sha(ROOT/RECEIPT),'tools/summarize_layers_costs.py':sha(Path(__file__))},profiles=profiles,analysis=dict(service='Fixed8-byte frame,3-byte parser candidate; constant work per byte, fixed memory, no allocation. Encode CRC16bit-steps plus nibble conversion.',posted='Bounded8-entry queue; each flush O(k),k<=8; irreversible prefix and quarantine metadata are part of the contract.',comparison='Same preconfigured8N1 single-byte TXE/write/TC contract with fresh per-call context. HAL handle initialization and original struct pl_byte_port construction both charged. Persistent-handle amortization is a different experiment.',maintenance='Literal G4 adapter changes status/data indices to7/10; generated approach updates schema/selection and regenerates; vendor approach changes pinned implementation/device headers/config and audits the additional BUSY_TX abort check. Equal selected traces do not imply concurrent-abort equivalence.'),limits=['Local frames are not maximum call-chain/IRQ stack. Guest harness code is separate from these three GC-linked cost images.','No physical latency/bandwidth or CPU cycle timing inferred.','Unused object functions may appear in .su listings; retained symbol maps identify selected runtime closure.'])
 (ROOT/'reviews/v5/expansion/legacy_advanced_layers/cost_analysis.json').write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=True,profiles=len(profiles),local_frames=sum(len(p['local_stack_frames']) for p in profiles)))
if __name__=='__main__':main()
