"""Extract measured image/stack facts without inventing whole-stack or WCET bounds."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[1];REPORT='reviews/v5/expansion/legacy_advanced_diagnostic/current_validation.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
d=json.loads((ROOT/REPORT).read_text());assert d['passed'];rows={r['label']:r for r in d['checks']};cells=[];commands=[];pins={REPORT:sha(ROOT/REPORT),'tools/summarize_diagnostic_costs.py':sha(Path(__file__))}
for cc in ['arm-none-eabi-gcc','clang']:
 for opt in ['-O0','-O3']:
  for kind in ['core','usb']:
   label=cc+opt+' '+kind+' 0';link=rows[label+' link']['command'];elf=Path(link[link.index('-o')+1]);cmd=['arm-none-eabi-nm','-S','--size-sort',str(elf)];r=subprocess.run(cmd,capture_output=True,text=True);commands.append(dict(command=cmd,returncode=r.returncode,stdout=r.stdout,stderr=r.stderr));assert r.returncode==0
   functions=[]
   for line in r.stdout.splitlines():
    v=line.split()
    if len(v)==4 and v[2] in ['T','t'] and v[3].startswith('dg_'):functions.append(dict(function=v[3],linked_bytes=int(v[1],16)))
   frames=[]
   for p in elf.parent.glob('*.su'):
    pins[p.relative_to(ROOT).as_posix()]=sha(p)
    for line in p.read_text().splitlines():
     fields=line.split('\t')
     if len(fields)>=3:
      name=fields[0].rsplit(':',1)[-1]
      if name.startswith('dg_'):frames.append(dict(function=name,bytes=int(fields[1]),classification=fields[2]))
   assert functions and frames
   sizepath=ROOT/rows[label+' size']['raw'];sections=[]
   for line in sizepath.read_text().splitlines():
    v=line.split()
    if len(v)==3 and v[0].startswith('.') and v[1].isdigit():sections.append(dict(section=v[0],bytes=int(v[1])))
   for p in [elf,sizepath]:pins[p.relative_to(ROOT).as_posix()]=sha(p)
   cells.append(dict(profile=label,functions=functions,linked_dg_function_bytes=sum(f['linked_bytes'] for f in functions),local_frames=frames,max_local_frame_bytes=max(f['bytes'] for f in frames),whole_test_image_sections=sections))
out=dict(passed=len(cells)==8,cells=cells,commands=commands,input_sha256=pins,derivations={'codec':'4+n wire octets, n<=32; encoding36-byte staging, decoding FCursors and staged DGMessage; O(n) time, no heap.', 'stream':'38-byte buffer; prefix-bound one-frame consume. Across a stream O(total consumed bytes), with application admission per complete frame.', 'Ethernet':'60-byte local frame and original two1514-byte TX slots; descriptor snapshot copies at most60 bytes, reported FCS excluded; no PHY/time prediction.', 'USB':'36-byte owned IN plus36-byte OUT accumulation; per-message floor((4+n)/8)+1 packets in this explicit short/ZLP-delimited protocol. Retries can repeat without a bounded wall-time progress guarantee.', 'guest':'Kernel owns endpoint/request allocations; portable core remains allocation-free. Request buffers EP0=64 OUT=40 IN=36; kernel object/FIFO overhead depends on pinned implementation.'},limits=['Measured linked dg function bytes can include a test main renamed dg_guest_main; per-function entries distinguish it. Whole image includes test/original helper/provider code.', 'Compiler .su entries are local frames, including functions that may be discarded; neither maxima nor their sum proves worst-case call-chain/ISR/kernel stack.', 'No physical latency, bandwidth, allocation timing or USB conformance measurement.','Native hosted, PPC64 and M4 results are different environments, not equivalent performance comparisons.'])
(ROOT/'reviews/v5/expansion/legacy_advanced_diagnostic/cost_analysis.json').write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=out['passed'],profiles=len(cells)))
