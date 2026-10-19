from pathlib import Path
import copy,json,sys
ROOT=Path(__file__).resolve().parents[3];sys.path.insert(0,str(ROOT/'labs/peripheral_depth/spi'))
from analyze_capture import analyze

def trace(mode,version):
 tx=[0x9f if not version else 0xdf,0,0];rx=[0xa5,0xef,0x40] if not version else [0xa6,0x40,0xef]
 e=[dict(ns=0,sck=mode//2,mosi=0,miso=0,cs=1),dict(ns=100,sck=mode//2,mosi=0,miso=0,cs=0)]
 t=2100
 for a,b in zip(tx,rx):
  for abit,bbit in zip(f'{a:08b}',f'{b:08b}'):
   for level in [1-mode//2,mode//2]:
    e.append(dict(ns=t,sck=level,mosi=int(abit),miso=int(bbit),cs=0));t+=500
 e.append(dict(ns=t+3000,sck=mode//2,mosi=0,miso=0,cs=1))
 return dict(origin='synthetic',mode=mode,tx=tx,rx=rx,events=e,uncertainty_ns=10,setup_ns=1980,hold_ns=3480,half_period_min_ns=480)
rows=[analyze(trace(m,v)) for m in range(4) for v in range(2)];controls=[]
for name in ['mode','first-idle','missing-clock','split-cs','wrong-byte','early-cs','setup','hold','uncertainty','nonmonotonic','physical-provenance']:
 d=trace(0,0)
 if name=='mode':d['mode']=4
 elif name=='first-idle':d['events'][0]['sck']=1
 elif name=='missing-clock':d['events'].pop(3)
 elif name=='split-cs':d['events'][10]['cs']=1
 elif name=='wrong-byte':d['rx'][1]^=1
 elif name=='early-cs':d['events'][-1]['ns']=d['events'][-2]['ns']+100
 elif name=='setup':d['setup_ns']+=1
 elif name=='hold':d['hold_ns']+=1
 elif name=='uncertainty':d['uncertainty_ns']=11
 elif name=='nonmonotonic':d['events'][3]['ns']=d['events'][2]['ns']
 else:d['origin']='physical'
 try:analyze(d)
 except ValueError:controls.append(dict(control=name,rejected=True))
 else:raise AssertionError(name)
print(json.dumps(dict(passed=True,synthetic=rows,negative_controls=controls),indent=2))
