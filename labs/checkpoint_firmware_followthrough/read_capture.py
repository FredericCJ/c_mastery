"""Decode a stopped selected F439 RAM snapshot; no target interaction."""
import argparse,json,struct
from pathlib import Path
HEADER=['magic','version','trace_enabled','mode','ring_capacity','received','accepted','dropped','completed','state','clock_start','clock_end','first_fault','fault_generation','retained','retries']
ROW=['sequence','isr_cycle','publish_cycle','frame_cycle','dma_cycle','tc_cycle','flags','generation']
def decode(data):
 if len(data)!=2112:raise ValueError('exact2112-byte selected capture required')
 values=struct.unpack('<528I',data);d=dict(zip(HEADER,values[:16]));d['rows']=[dict(zip(ROW,values[16+8*i:24+8*i])) for i in range(64)]
 if d['magic']!=0x50344331 or d['version']!=1 or d['received']>64:raise ValueError('wrong selected capture ABI')
 return d
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('snapshot',type=Path);p.add_argument('--output',type=Path,required=True);a=p.parse_args();a.output.write_text(json.dumps(decode(a.snapshot.read_bytes()),indent=2)+'\n')
