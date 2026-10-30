"""Independent byte-layout oracle and target-independent generated vectors.

This uses Python integers, struct unpacking and explicit record slicing. It does
not call the C parser or extract expected data from its output.
"""
import argparse,hashlib,json,random,struct
from pathlib import Path

def table(records,lapic=0xfee00000,flags=0):
 data=bytearray(44);data[:4]=b'APIC';data[8]=7;struct.pack_into('<II',data,36,lapic,flags)
 data.extend(b''.join(records));struct.pack_into('<I',data,4,len(data));return checksum(data)

def checksum(data):
 data=bytearray(data)
 if len(data)>9:data[9]=0;data[9]=(-sum(data))%256
 return bytes(data)

def record(kind,uid,apic,flags):
 if kind==0:return struct.pack('<BBBBI',0,8,uid,apic,flags)
 return struct.pack('<BBHIII',9,16,0,apic,flags,uid)

def expected(data,capacity,profile):
 fail=lambda status:[status,0,0,0,0,0,0]
 if len(data)<44 or not 1<=capacity<=16 or profile not in (0,1) or data[:4]!=b'APIC':return fail(1)
 size=struct.unpack_from('<I',data,4)[0]
 if not 44<=size<=min(65536,len(data)):return fail(1)
 data=data[:size]
 if sum(data)%256:return fail(2)
 lapic,flags=struct.unpack_from('<II',data,36)
 if not lapic or lapic%4096:return fail(3)
 remaining=data[44:];cpus=[];disabled=online=unknown=0
 while remaining:
  if len(remaining)<2:return fail(1)
  kind,size=remaining[:2]
  if not 2<=size<=len(remaining):return fail(1)
  entry,remaining=remaining[:size],remaining[size:]
  if kind==5:return fail(3)
  if kind==0:
   if len(entry)!=8:return fail(1)
   _,_,uid,apic,bits=struct.unpack('<BBBBI',entry)
  elif kind==9:
   if profile==0:return fail(3)
   if len(entry)!=16:return fail(1)
   _,_,_,apic,bits,uid=struct.unpack('<BBHIII',entry)
  else:unknown+=1;continue
  if bits&1:
   if len(cpus)==capacity:return fail(5)
   if any(uid==c[0] or apic==c[1] for c in cpus):return fail(4)
   cpus.append((uid,apic,kind))
  elif bits&2:online+=1
  else:disabled+=1
 if not cpus:return fail(6)
 return [0,lapic,flags,len(cpus),disabled,online,unknown,*[v for c in cpus for v in c]]

def cases():
 rows=[];sparse=table([record(0,10+i,i*2,0 if i==2 else 1) for i in range(4)])
 for n in range(len(sparse)+1):rows.append((sparse[:n],16,0))
 for offset in range(len(sparse)):
  changed=bytearray(sparse);changed[offset]^=255;rows.append((bytes(changed),16,0));rows.append((checksum(changed),16,0))
 for flags in (0,1,2,3,0x80000000,0xffffffff):
  for apic in (0,255,256,0x12345678,0xffffffff):
   for profile in (0,1):rows.append((table([record(9,apic^0xffffffff,apic,flags)]),16,profile))
 for cap in (0,1,2,3,4,16,17):
  for mode in (0,1,2):rows.append((sparse,cap,mode))
 for length in (0,1,2,7,8,9,15,16,17,255):
  for kind in (0,5,9,127):rows.append((table([bytes([kind,length])+bytes(14)]),16,1))
 rows.append((sparse+b'ignored-tail',16,0))
 rows.append((table([record(0,17,31,1),record(9,17,0x12345678,1)]),16,1))
 rows.append((table([record(0,17,31,1),record(9,0x12345678,31,1)]),16,1))
 for count in (16,17):rows.append((table([record(0,i,i*2,1) for i in range(count)]),16,0))
 records=[record(0,17,31,1)]+[bytes([127,255])+bytes(253)]*256+[bytes([127,204])+bytes(202)]
 largest=table(records);assert len(largest)==65536;rows.append((largest,16,0));changed=bytearray(largest);struct.pack_into('<I',changed,4,65537);rows.append((checksum(changed),16,0))
 rng=random.Random(5301)
 for index in range(1600):
  records=[]
  for _ in range(rng.randrange(0,20)):
   kind=rng.choice((0,0,9,127))
   if kind==127:records.append(bytes([127,2]));continue
   uid=rng.randrange(0,12) if kind==0 else rng.getrandbits(32);apic=rng.randrange(0,12) if kind==0 else rng.getrandbits(32)
   records.append(record(kind,uid,apic,rng.choice((0,1,2,3,0x80000001))))
  data=table(records,lapic=rng.choice((0xfee00000,0,0xfee00001)),flags=rng.getrandbits(32));rows.append((data,rng.choice((1,2,4,16)),rng.randrange(2)))
 return rows

def inputs(rows=None):
 for data,capacity,profile in rows if rows is not None else cases():yield f'{profile} {capacity} {len(data)} '+data.hex(' ')

def check(text,rows=None):
 rows=cases() if rows is None else rows;lines=text.splitlines()
 if len(lines)!=len(rows):raise ValueError('Missing/extra independent oracle row')
 for index,(line,(data,capacity,profile)) in enumerate(zip(lines,rows)):
  if line.split()!=['SA',*map(str,expected(data,capacity,profile))]:raise ValueError('Wrong wire decode at row '+str(index))
 return {'rows':len(rows),'input_sha256':hashlib.sha256(('\n'.join(inputs(rows))+'\n').encode()).hexdigest()}

def header(path,rows=None):
 rows=cases() if rows is None else rows;data=bytearray();entries=[]
 for value,capacity,profile in rows:
  want=expected(value,capacity,profile);entries.append((len(data),len(value),capacity,profile,want));data.extend(value)
 lines=['/* Generated independent expected bytes/fields by oracle.py. */','static const unsigned char sa_wire[]={',','.join(map(str,data)),'};',
 'struct SaExpected {unsigned offset,length,capacity,profile;uint32_t fields[55];};','static const struct SaExpected sa_expected[]={']
 for offset,length,capacity,profile,want in entries:lines.append('{'+','.join(map(str,(offset,length,capacity,profile)))+',{'+','.join(map(str,want))+'}},')
 lines+=['};','static int sa_oracle_cases(void){',
 'for(size_t i=0;i<sizeof sa_expected/sizeof sa_expected[0];++i){const struct SaExpected *e=&sa_expected[i];struct sa_topology t={0};int r=sa_parse(sa_wire+e->offset,e->length,e->capacity,e->profile,&t);',
 'if(r!=(int)e->fields[0]||t.lapic!=e->fields[1]||t.flags!=e->fields[2]||t.count!=e->fields[3]||t.disabled!=e->fields[4]||t.online_capable!=e->fields[5]||t.unknown!=e->fields[6])return 1;',
 'for(unsigned j=0;j<t.count;++j)if(t.cpu[j].uid!=e->fields[7+3*j]||t.cpu[j].apic!=e->fields[8+3*j]||t.cpu[j].kind!=e->fields[9+3*j])return 1;}', 'return 0;}']
 path.write_text('\n'.join(lines)+'\n',encoding='utf-8',newline='\n');return len(rows)

def capture_header(path,captures):
 lines=['/* Exact captured bytes; expected fields independently decoded in Python. */'];checks=[];observations=[]
 if len(captures)!=6 or {(r['machine'],r['cpus']) for r in captures}!={(m,c) for m in ('pc-i440fx-10.2','pc-q35-10.2') for c in (1,2,4)}:raise ValueError('Missing changed-machine/CPU condition')
 for i,capture in enumerate(captures):
  rsdp=bytes.fromhex(capture['rsdp']);regions={r['address']:bytes.fromhex(r['bytes']) for r in capture['regions']}
  if rsdp[:8]!=b'RSD PTR ' or sum(rsdp[:20])%256 or len(regions)!=len(capture['regions']):raise ValueError('Invalid captured RSDP/region uniqueness')
  if rsdp[15]==0:root_address=struct.unpack_from('<I',rsdp,16)[0];stride=4;signature=b'RSDT'
  elif rsdp[15]==2 and len(rsdp)==36 and sum(rsdp)%256==0:root_address=struct.unpack_from('<Q',rsdp,24)[0];stride=8;signature=b'XSDT'
  else:raise ValueError('Unsupported captured RSDP')
  root=regions[root_address]
  if root[:4]!=signature or sum(root)%256 or (len(root)-36)%stride:raise ValueError('Invalid root')
  tables=[]
  for at in range(36,len(root),stride):
   address=int.from_bytes(root[at:at+stride],'little');table_bytes=regions[address]
   if len(table_bytes)<36 or struct.unpack_from('<I',table_bytes,4)[0]!=len(table_bytes) or sum(table_bytes)%256:raise ValueError('Invalid captured child')
   if table_bytes[:4]==b'APIC':tables.append(table_bytes)
  if len(tables)!=1:raise ValueError('Expected unique MADT')
  wanted=expected(tables[0],16,0)
  if wanted[0]!=0 or wanted[3]!=capture['cpus'] or wanted[8::3]!=list(range(capture['cpus'])):raise ValueError('Actual firmware inventory disagrees with requested virtual CPUs')
  lines.append('static const unsigned char sa_rsdp_'+str(i)+'[]={'+','.join(map(str,rsdp))+'};')
  entries=[]
  for j,(address,data) in enumerate(regions.items()):
   name=f'sa_region_{i}_{j}';lines.append('static const unsigned char '+name+'[]={'+','.join(map(str,data))+'};');entries.append('{UINT64_C('+str(address)+'),'+name+',sizeof '+name+'}')
  lines.append('static const struct sa_region sa_regions_'+str(i)+'[]={'+','.join(entries)+'};')
  checks.append('{struct sa_topology t={0};if(sa_discover(sa_rsdp_'+str(i)+',sizeof sa_rsdp_'+str(i)+',sa_regions_'+str(i)+','+str(len(regions))+',16,0,&t)!=SA_OK)return 1;')
  for field,value in zip(('lapic','flags','count','disabled','online_capable','unknown'),wanted[1:7]):checks.append('if(t.'+field+'!=UINT32_C('+str(value)+'))return 1;')
  for j in range(wanted[3]):
   for field,value in zip(('uid','apic','kind'),wanted[7+j*3:10+j*3]):checks.append('if(t.cpu['+str(j)+'].'+field+'!=UINT32_C('+str(value)+'))return 1;')
  checks.append('}');observations.append({'machine':capture['machine'],'cpus':capture['cpus'],'region_count':len(regions),'expected':wanted,'madt_sha256':hashlib.sha256(tables[0]).hexdigest()})
 lines+=['static int sa_capture_cases(void){',*checks,'return 0;}'];path.write_text('\n'.join(lines)+'\n',encoding='utf-8',newline='\n');return observations

if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--header',type=Path);args=parser.parse_args()
 if args.header:print(json.dumps({'rows':header(args.header)}))
 else:print('\n'.join(inputs()))
