"""Versioned linear address decoder adds actual Thumb MOV immediates/registers and signed address offsets.

Extends the UART decoder without changing its frozen evidence. This remains a
structural check, not a control-flow/device-semantics proof.
"""
import re

def table_addresses(section):
 """Follow emitted TBB cases independently; a linear scan joins unrelated paths.

 Only the compiler's bounded rd/wr switch is handled. Decode the actual little
 endian table bytes and branch destinations, not the desired register offsets.
 Unsupported instructions fail to contribute evidence rather than guess an
 address. Native tests separately assess each selector's semantic mapping.
 """
 if not re.search(r'<(?:rd|wr)>:',section):return set()
 instructions={};data={};observed=set();regs={};table=None
 for line in section.splitlines():
  m=re.match(r'^\s*([0-9a-f]+):\s+(?:[0-9a-f]{4,8}\s+)+([a-z.][a-z0-9.]*)\s+(.*)',line)
  if not m:continue
  address,op,arg=int(m[1],16),m[2].split('.')[0],m[3]
  if m[2] in ('.word','.short'):
   value=int(re.search(r'0x([0-9a-f]+)',arg)[1],16)
   for i in range(4 if m[2]=='.word' else 2):data[address+i]=(value>>(8*i))&255
  else:instructions[address]=(op,arg)
 for address,(op,arg) in instructions.items():
  if op=='movw':regs[arg.split(',')[0]]=int(re.search(r'#(\d+)',arg)[1])
  elif op=='movt':
   dest=arg.split(',')[0]
   if dest in regs:regs[dest]=(regs[dest]&65535)|(int(re.search(r'#(\d+)',arg)[1])<<16)
  elif op=='tbb':table=address+4;break
 if table is None or any(table+i not in data for i in range(10)):return set()
 order=sorted(instructions);next_pc={a:b for a,b in zip(order,order[1:])}
 for index in range(10):
  pc=table+2*data[table+index];state=dict(regs);seen=set()
  while pc in instructions and pc not in seen:
   seen.add(pc);op,arg=instructions[pc];dest=arg.split(',')[0].strip()
   if op in ('adds','add','subs','sub'):
    m=re.match(r'(r\d+|ip|lr),\s*(?:(r\d+|ip|lr),\s*)?#(\d+)',arg)
    if not m or (m[2] or m[1]) not in state:break
    state[m[1]]=state[m[2] or m[1]]+(1 if op.startswith('add') else -1)*int(m[3])
   elif op.startswith(('ldr','str')):
    m=re.search(r'\[(r\d+|ip|lr)(?:,\s*#(-?\d+))?\]',arg)
    if not m or m[1] not in state:break
    address=state[m[1]]+int(m[2] or 0)
    if 0x40000000<=address<0x60000000:observed.add((address,op))
    if op.startswith('ldr'):state.pop(dest,None)
   elif op=='b':
    m=re.match(r'([0-9a-f]+)\s+<',arg)
    if not m:break
    pc=int(m[1],16);continue
   elif op=='bx':break
   elif op!='nop':break
   pc=next_pc.get(pc,-1)
 return observed
def addresses(text):
 observed=set()
 for section in re.split(r'Disassembly of section ',text):
  observed.update(table_addresses(section))
  words={int(a,16):int(v,16) for a,v in re.findall(r'^\s*([0-9a-f]+):\s+[0-9a-f]+\s+\.word\s+0x([0-9a-f]+)',section,re.M)};regs={}
  for line in section.splitlines():
   if re.match(r'^[0-9a-f]+ <',line):regs={}
   m=re.match(r'^\s*[0-9a-f]+:\s+(?:[0-9a-f]{4,8}\s+)+([a-z][a-z0-9.]*)\s+(.*)',line)
   if not m:continue
   op,arg=m.groups();op=op.split('.')[0];parts=arg.split(',');dest=parts[0].strip()
   if op in ('mov','movs'):
    m=re.search(r'^(r\d+|ip|lr),\s*#(\d+)',arg)
    if m:regs[m[1]]=int(m[2]);continue
    m=re.search(r'^(r\d+|ip|lr),\s*(r\d+|ip|lr)$',arg)
    if m and m[2] in regs:regs[m[1]]=regs[m[2]];continue
   if op=='movw':regs[dest]=int(re.search(r'#(\d+)',arg)[1]);continue
   if op=='movt':
    if dest in regs:regs[dest]=(regs[dest]&65535)|(int(re.search(r'#(\d+)',arg)[1])<<16)
    continue
   if op.startswith(('ldr','str')):
    pool=re.search(r'\[pc,.*@\s*\(([0-9a-f]+)',arg)
    if pool and int(pool[1],16) in words:regs[dest]=words[int(pool[1],16)];continue
    mem=re.search(r'\[(r\d+|ip|lr)(?:,\s*#(-?\d+))?\]',arg)
    if mem and mem[1] in regs:
     address=regs[mem[1]]+int(mem[2] or 0)
     if 0x40000000<=address<0x60000000:observed.add((address,op))
    if op.startswith('ldr'):regs.pop(dest,None)
    continue
   if op in ('add','adds','sub','subs'):
    m=re.search(r'^(r\d+|ip|lr),\s*(r\d+|ip|lr),\s*#(\d+)',arg)
    if m and m[2] in regs:regs[m[1]]=regs[m[2]]+(1 if op.startswith('add') else -1)*int(m[3]);continue
   if op not in ('cmp','tst','b','beq','bne','bcc','bcs','bhi','bls','it','itt','ite','push','pop','bx','cbz','cbnz','tbb','nop') and not op.startswith('str'):regs.pop(dest,None)
 return observed
REQUIRED={0x40023820,0x40012000,0x40012004,0x40012008,0x40012010,0x4001202c,0x40012034,0x4001204c,0x40012304,0x40000404,0x40007408,0x40007404}
def validate(text):
 observed=addresses(text);missing=REQUIRED-{a for a,op in observed};narrow=[(hex(a),op) for a,op in observed if op.startswith(('ldrb','ldrh','strb','strh'))]
 return dict(passed=not missing and not narrow,missing=[hex(a) for a in sorted(missing)],narrow=narrow,observed=[dict(address=hex(a),instruction=op) for a,op in sorted(observed)],limits='Structural mapping check; native register-effect tests remain necessary. No physical execution.')

COMP_REQUIRED={0x4002104c,0x40021060,0x40010200,0x48000000,0x4800000c}
def validate_comparator(text):
 observed=addresses(text);missing=COMP_REQUIRED-{a for a,op in observed};narrow=[(hex(a),op) for a,op in observed if op.startswith(('ldrb','ldrh','strb','strh'))]
 return dict(passed=not missing and not narrow,missing=[hex(a) for a in sorted(missing)],narrow=narrow,limits='Actual object address/word check, not analog evidence or a control-flow proof.')
