"""Independent finite T48 contracts; this does not execute C or a guest."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def oracle(kind,i):
 if kind=='boot':return {'entries':i['mapped_bytes']//i['large_page']}
 if kind=='tags':return {'next':i['offset']+((i['size']+7)//8)*8}
 if kind=='frame':return {'bytes':(i['gpr']+i['metadata']+i['hardware'])*i['word']}
 if kind=='pages':return {'count':(i['end']-i['begin'])//i['page']}
 if kind=='fault':return {'code':int(i['present'])+2*int(i['write'])+4*int(i['user'])}
 if kind=='schedule':
  out=[];p=i['current'];a=i['alive']
  for _ in range(i['steps']):
   for delta in range(1,len(a)+1):
    if a[(p+delta)%len(a)]:p=(p+delta)%len(a);break
   out.append(p)
  return {'sequence':out}
 if kind=='elf':return {'zero_bytes':i['memsz']-i['filesz']}
 if kind=='handle':return {'token':(i['generation']<<16)|(i['slot']+1)}
 if kind=='queue':
  n=(i['index']+1)&65535
  return {'next':n,'before_slot':i['index']%i['slots'],'after_slot':n%i['slots']}
 if kind=='disk':return {'pages':i['bytes']//i['page'],'sectors_per_page':i['page']//i['sector'],'slice_end':i['slice_start']+i['slice_pages']}
 if kind=='archive':
  off=i['header']+i['entries']*i['entry_size'];return {'offset':off,'total':off+i['payload']}
 if kind=='vm':return {'value':i['input']+i['host_increment'],'remaining':i['host_budget']-1}
 if kind=='commit':return {'after_recovery':i['committed'],'reply_proves_absence':False}
 if kind=='scope':return {'physical_complete':i['physical']}
 raise ValueError(kind)
def main():
 paths=sorted((ROOT/'fixtures/kernel').glob('PR-T48-*.json'))
 assert len(paths)==14
 for n,p in enumerate(paths,1):
  d=json.loads(p.read_text(encoding='utf-8'));assert d['id']==f'FX-KERNEL-{n:02d}';assert oracle(d['kind'],d['input'])==d['expected'],p
 print('14 kernel finite fixtures pass; actual C and guest execution remain separate')
if __name__=='__main__':main()
