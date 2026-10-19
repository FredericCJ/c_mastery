"""Independent packed bytes test the public decoder; no board observation."""
from pathlib import Path
import importlib.util,json,struct,sys
lab=Path(sys.argv[1]);s=importlib.util.spec_from_file_location('decoder',lab/'read_capture.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
data=bytearray(2112);struct.pack_into('<I',data,0,0x50344331);struct.pack_into('<I',data,4,1);struct.pack_into('<I',data,20,1);struct.pack_into('<8I',data,64,0,0xfffffffa,6,9,11,13,29,1)
d=m.decode(data);assert d['received']==1 and d['rows'][0]['isr_cycle']==0xfffffffa and d['rows'][0]['publish_cycle']==6 and d['rows'][63]['flags']==0
controls=[]
for name,value in [('short',data[:-1]),('long',data+b'\0'),('endian',bytes(data[:4][::-1])+data[4:]),('version',data[:4]+struct.pack('<I',2)+data[8:]),('count',data[:20]+struct.pack('<I',65)+data[24:])]:
 try:m.decode(value)
 except ValueError:controls.append(name)
 else:raise AssertionError(name)
print(json.dumps(dict(passed=True,negative_controls=controls,physical=False)))
