"""Selected GNU Arm ELF32 release binding, not a general ELF loader."""
import hashlib,json,struct
def digest(b):return hashlib.sha256(b).hexdigest()
def segments(blob):
 if len(blob)<52 or blob[:7]!=b'\x7fELF\x01\x01\x01':raise ValueError('Expected little-endian ELF32')
 kind,machine,version,entry,phoff=struct.unpack_from('<HHIII',blob,16)
 size,entry_size,count=struct.unpack_from('<HHH',blob,40)
 if kind!=2 or machine!=40 or version!=1 or size!=52 or entry_size!=32 or not 1<=count<=16 or phoff>len(blob) or count*32>len(blob)-phoff:raise ValueError('Unsupported ELF contract')
 rows=[]
 for i in range(count):
  ptype,off,va,pa,fs,ms,flags,align=struct.unpack_from('<IIIIIIII',blob,phoff+32*i)
  if ptype!=1:continue
  if fs>ms or off>len(blob) or fs>len(blob)-off or ms>2**32-va or fs>2**32-pa or flags&~7 or align>1 and (align&(align-1) or va%align!=off%align):raise ValueError('Invalid load segment')
  row={'virtual':va,'load':pa,'file_bytes':fs,'memory_bytes':ms,'flags':flags,'payload_sha256':digest(blob[off:off+fs]),'payload':blob[off:off+fs].hex()}
  for old in rows:
   if ms and old['memory_bytes'] and va<old['virtual']+old['memory_bytes'] and old['virtual']<va+ms:raise ValueError('Overlapping virtual segments')
   if fs and old['file_bytes'] and pa<old['load']+old['file_bytes'] and old['load']<pa+fs:raise ValueError('Overlapping programmed spans')
  rows.append(row)
 if not rows or not any(r['flags']&1 and r['virtual']<=(entry&~1)<r['virtual']+r['file_bytes'] for r in rows):raise ValueError('Missing executable entry')
 return rows
def build(elf,deployment,symbols,mapping,source_revision,flags,target):
 if not all(isinstance(x,bytes) and x for x in (elf,deployment,symbols,mapping)) or not isinstance(source_revision,str) or len(source_revision)!=64 or any(c not in '0123456789abcdef' for c in source_revision) or not isinstance(flags,list) or not flags or any(not isinstance(x,str) for x in flags) or not isinstance(target,str) or not target:raise ValueError('Missing exact build identity')
 rows=segments(elf)
 if segments(deployment)!=rows:raise ValueError('Deployment differs from linked load mapping')
 body={'schema_version':1,'target':target,'source_revision':source_revision,'flags':flags,'files':{n:digest(b) for n,b in [('elf',elf),('deployment',deployment),('symbols',symbols),('mapping',mapping)]},'segments':rows,'verification_scope':'Selected declared GNU Arm physical-load convention; programmed spans only, holes and zero-fill not Flash bytes; no actual programming.'}
 identity=digest(json.dumps(body,sort_keys=True,separators=(',',':')).encode());return {**body,'identity':identity}
def attribute(manifest,record,elf,deployment,symbols,mapping,source_revision,flags,target):
 expected=build(elf,deployment,symbols,mapping,source_revision,flags,target)
 if manifest!=expected or not isinstance(record,dict) or record.get('manifest')!=expected['identity'] or type(record.get('pc')) is not int:raise ValueError('Unbound capture or artifact')
 pc=record['pc']
 if not 0<=pc<2**32 or pc&1:raise ValueError('Invalid saved Thumb instruction address')
 if not any(r['flags']&1 and r['virtual']<=pc<r['virtual']+r['file_bytes'] for r in expected['segments']):raise ValueError('PC outside executable file-backed mapping')
 return {'pc':pc,'manifest':expected['identity'],'authoritative_for':'exact supplied build only','programmed_hardware_verified':False}
