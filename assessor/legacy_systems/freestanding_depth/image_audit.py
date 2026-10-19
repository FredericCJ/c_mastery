"""Bounded GNU binutils/x86-64 ELF audit; no universal semantic guarantee."""
import hashlib,re
FIELDS={'symbols','undefined','dynamic','segments','sections','disassembly','map'}
MAX_REPORT=8*1024*1024
def digest(value):return hashlib.sha256(value.encode()).hexdigest()
def validate(bundle,expected_binary_sha256,expected_policy_sha256):
 errors=[]
 if bundle.get('schema')!=1:return ['schema']
 if bundle.get('binary_sha256')!=expected_binary_sha256:return ['binary identity']
 policy=bundle.get('policy',{});text=bundle.get('policy_text','')
 if digest(text)!=expected_policy_sha256 or bundle.get('policy_sha256')!=expected_policy_sha256:return ['policy identity']
 import json
 try:
  if json.loads(text)!=policy:return ['policy content']
 except ValueError:return ['policy encoding']
 reports=bundle.get('reports',{})
 if set(reports)!=FIELDS:return ['missing report']
 for name,row in reports.items():
  body=row.get('text');status=row.get('exit')
  if status!=0 or not isinstance(body,str) or len(body.encode())>MAX_REPORT or '\0' in body or (body and not body.endswith('\n')) or any(len(line)>8192 for line in body.splitlines()):errors.append('invalid report '+name);continue
  if digest(body)!=row.get('sha256'):errors.append('report identity '+name)
 if errors:return errors
 get=lambda name:reports[name]['text']
 if get('undefined').strip():errors.append('unresolved helper')
 if 'NEEDED' in get('dynamic') or 'INTERP' in get('segments'):errors.append('external runtime')
 if 'There is no dynamic section in this file.' not in get('dynamic') and not ('Dynamic section at offset' in get('dynamic') and 'Tag' in get('dynamic')):errors.append('invalid dynamic report')
 if '.symtab' not in get('sections'):errors.append('missing symbol table')
 if 'file format elf64-x86-64' not in get('disassembly'):errors.append('unsupported disassembly format')
 if not re.search(r'Elf file type is (EXEC|DYN)',get('segments')):errors.append('missing ELF program headers')
 symbols=set()
 for line in get('symbols').splitlines():
  fields=line.split()
  if len(fields) not in (3,4) or len(fields[1])!=1 or not fields[1].isalpha() or any(re.fullmatch('[0-9a-fA-F]+',v) is None for v in fields[2:]):errors.append('invalid symbol report');continue
  symbols.add(fields[0])
 if not symbols or '_start' not in symbols:errors.append('missing final symbols')
 forbidden=set(policy.get('forbidden_symbols',[]))
 if symbols&forbidden:errors.append('resolved forbidden symbol')
 # Each supported map syntax is explicit; no empty fallback on another linker.
 if policy.get('map_format')=='bfd' and 'Linker script and memory map' in get('map'):
  loads=re.findall(r'^LOAD (.+)$',get('map'),re.M)
 elif policy.get('map_format')=='lld' and 'VMA' in get('map').splitlines()[0] and 'Symbol' in get('map').splitlines()[0]:
  loads=re.findall(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+(.+):\(',get('map'),re.M)
 else:loads=[];errors.append('unsupported origin map')
 if not policy.get('allowed_loads'):errors.append('unsupported origin map')
 if not loads:errors.append('missing origins')
 if set(loads)-set(policy.get('allowed_loads',[])):errors.append('forbidden origin')
 inputs=bundle.get('build_inputs',{})
 if not inputs or inputs!=policy.get('approved_build_inputs'):errors.append('unapproved build input')
 # Header/line parser deliberately bounds its claim to direct edges. An
 # unresolved indirect edge is visible uncertainty, never a silent proof.
 current=None;edges=[]
 for line in get('disassembly').splitlines():
  match=re.match(r'^\s*[0-9a-f]+ <([^>]+)>:$',line)
  if match:current=match[1]
  call=re.search(r'\b(callq?|jmpq?)\s+[0-9a-f]+\s+<([^>]+)>',line)
  if call and current and '+' not in call[2]:edges.append((current,call[2]))
  if current in policy.get('provider_symbols',[]) and re.search(r'\bj\w+\s+[0-9a-f]+\s+<'+re.escape(current)+r'>',line):errors.append('provider self cycle')
 graph={}
 for left,right in edges:graph.setdefault(left,set()).add(right)
 for provider in policy.get('provider_symbols',[]):
  todo=list(graph.get(provider,()));seen=set()
  while todo:
   node=todo.pop()
   if node==provider:errors.append('provider call cycle');break
   if node not in seen:seen.add(node);todo.extend(graph.get(node,()))
 return sorted(set(errors))
