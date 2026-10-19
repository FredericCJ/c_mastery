"""Versioned linear address decoder adds actual Thumb MOV immediates/registers.

Extends the UART decoder without changing its frozen evidence. This remains a
structural check, not a control-flow/device-semantics proof.
"""
import re
def addresses(text):
 observed=set()
 for section in re.split(r'Disassembly of section ',text):
  words={int(a,16):int(v,16) for a,v in re.findall(r'^\s*([0-9a-f]+):\s+[0-9a-f]+\s+\.word\s+0x([0-9a-f]+)',section,re.M)};regs={}
  for line in section.splitlines():
   if re.match(r'^[0-9a-f]+ <',line):regs={}
   m=re.match(r'^\s*[0-9a-f]+:\s+(?:[0-9a-f]{4,8}\s+)+([a-z][a-z0-9.]*)\s+(.*)',line)
   if not m:continue
   op,arg=m.groups();op=op.split('.')[0];parts=arg.split(',');dest=parts[0].strip()
   if op in ('mov','movs'):
    m=re.search(r'^(r\d+|ip),\s*#(\d+)',arg)
    if m:regs[m[1]]=int(m[2]);continue
    m=re.search(r'^(r\d+|ip),\s*(r\d+|ip)$',arg)
    if m and m[2] in regs:regs[m[1]]=regs[m[2]];continue
   if op=='movw':regs[dest]=int(re.search(r'#(\d+)',arg)[1]);continue
   if op=='movt':
    if dest in regs:regs[dest]=(regs[dest]&65535)|(int(re.search(r'#(\d+)',arg)[1])<<16)
    continue
   if op.startswith(('ldr','str')):
    pool=re.search(r'\[pc,.*@\s*\(([0-9a-f]+)',arg)
    if pool and int(pool[1],16) in words:regs[dest]=words[int(pool[1],16)];continue
    mem=re.search(r'\[(r\d+|ip)(?:,\s*#(\d+))?\]',arg)
    if mem and mem[1] in regs:
     address=regs[mem[1]]+int(mem[2] or 0)
     if 0x40000000<=address<0x60000000:observed.add((address,op))
    if op.startswith('ldr'):regs.pop(dest,None)
    continue
   if op in ('add','adds','sub','subs'):
    m=re.search(r'^(r\d+|ip),\s*(r\d+|ip),\s*#(\d+)',arg)
    if m and m[2] in regs:regs[m[1]]=regs[m[2]]+(1 if op.startswith('add') else -1)*int(m[3]);continue
   if op not in ('cmp','tst','b','beq','bne','bcc','bcs','bhi','bls','it','itt','ite','push','pop','bx','cbz','cbnz','tbb','nop') and not op.startswith('str'):regs.pop(dest,None)
 return observed
REQUIRED={0x40000400,0x40000418,0x40000428,0x4000042c,0x40000434,0x40000438,0x40000810,0x40000834}
def validate(text):
 observed=addresses(text);missing=REQUIRED-{a for a,op in observed};narrow=[(hex(a),op) for a,op in observed if op.startswith(('ldrb','ldrh','strb','strh'))]
 return dict(passed=not missing and not narrow,missing=[hex(a) for a in sorted(missing)],narrow=narrow,observed=[dict(address=hex(a),instruction=op) for a,op in sorted(observed)],limits='Structural mapping check; native register-effect tests remain necessary. No physical execution.')
