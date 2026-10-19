"""Explicit grammar, deterministic mutations and same-site triage policy."""
import json
from pathlib import Path
HERE=Path(__file__).resolve().parent
NAMES=['ok','length','opcode','count','flags','invalid','session','sequence','range']
def expected(profile,data):
 n=len(data);header=3 if profile else 2;out=[0,0,0,0,0,0,0,0]
 if n>(12 if profile else 10) or n<header:out[0]=1
 elif data[0]!=(2 if profile else 1):out[0]=2
 elif data[1]>4:out[0]=3
 elif profile and data[2]>1:out[0]=4
 elif n!=header+2*data[1]+(data[2] if profile else 0):out[0]=1
 else:
  out[1]=data[1];out[2]=data[2] if profile else 0;out[3]=data[-1] if out[2] else 0
  for i in range(out[1]):out[4+i]=int.from_bytes(data[header+2*i:header+2*i+2],'big')
 return out
def inputs():
 config=json.loads((HERE/'seeds.json').read_text(encoding='utf-8'));rows=[]
 for seed in config['seeds']:
  data=bytes.fromhex(seed['hex'])
  if NAMES[expected(seed['profile'],data)[0]]!=seed['expected']:raise ValueError('Wrong independent seed decision')
  rows.append((seed['profile'],data))
 for profile in range(2):
  for opcode in range(3):
   for count in range(6):
    for flag in range(3):
     for n in range(17):
      data=bytes([opcode,count,flag,*range(3,16)])[:n];rows.append((profile,data))
 state=0xC0FFEE;seeds=[bytes.fromhex(s['hex']) for s in config['seeds']]
 for i in range(2048):
  state=(1664525*state+1013904223)&0xffffffff;data=bytearray(seeds[state%len(seeds)]);op=(state>>16)%3
  if op==0 and data:data[(state>>8)%len(data)]^=(state>>24)&255
  elif op==1 and data:del data[(state>>8)%len(data)]
  elif len(data)<16:data.insert((state>>8)%(len(data)+1),(state>>24)&255)
  rows.append((i%2,bytes(data)))
 return rows
def stream():return ''.join(f'{p} {len(d)} '+ ' '.join(map(str,d))+'\n' for p,d in inputs())
def check(text):
 lines=text.splitlines();rows=inputs()
 if len(lines)!=len(rows):raise ValueError('Missing deterministic grammar observations')
 for line,(profile,data) in zip(lines,rows):
  if list(map(int,line.split()))!=expected(profile,data):raise ValueError('Grammar/count/output disagreement')
 return {'cases':len(rows),'mutation_seed':0xC0FFEE,'mutation_version':'lcg32-seed-byte-insert-delete-xor-v1','mutations':2048,'seed_count':11,'scope':'Declared two grammars and selected deterministic changes; not every input'}
def bounds_diagnostic(text,code):
 return code==86 and 'ERROR: AddressSanitizer: heap-buffer-overflow' in text and 'READ of size 1' in text and 'tf_teaching_bounds' in text and 'defect.c:' in text
def reduce_bytes(original,predicate):
 if not predicate(original):raise ValueError('Missing diagnosed original failure')
 current=original;attempts=[];changed=True
 while changed:
  changed=False
  for i in range(len(current)):
   candidate=current[:i]+current[i+1:];same=predicate(candidate);attempts.append({'hex':candidate.hex(),'same_failure':same})
   if same:current=candidate;changed=True;break
 for i in range(len(current)):
  for value in range(current[i]):
   candidate=current[:i]+bytes([value])+current[i+1:];same=predicate(candidate);attempts.append({'hex':candidate.hex(),'same_failure':same})
   if same:current=candidate;break
 return {'original_hex':original.hex(),'minimized_hex':current.hex(),'attempts':attempts,'scope':'Same diagnosed access, deletion then byte-value reduction; minimal length justified separately from arbitrary crash minimization'}
