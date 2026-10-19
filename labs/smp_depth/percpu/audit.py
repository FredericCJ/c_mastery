"""Actual frame/disassembly evidence; bounded call-tree estimate, not universal proof."""
import re,subprocess
from pathlib import Path
def stack_records(text):
 rows={}
 for line in text.splitlines():
  pieces=line.split('\t')
  if len(pieces)<3:continue
  name=pieces[0].rsplit(':',1)[-1];rows[name]={'bytes':int(pieces[1]),'kind':pieces[2]}
 return rows
def guest_audit(folder,simd,run=None):
 if run is None:
  def run(cmd):return subprocess.run(list(map(str,cmd)),check=True,capture_output=True,text=True).stdout
 records=stack_records((folder/'smp.c.su').read_text(encoding='utf-8'));assembly=run(['objdump','-d',folder/'ap.S.o']);c_assembly=run(['objdump','-d',folder/'smp.c.o'])
 # A whole object scan includes prerequisite functions. Isolate this original
 # mechanism's assembly section from its first labelled entry onward.
 section=assembly[assembly.index('<sp_bsp_enter>'):]
 if 'iretq' not in section or 'sp_irq_common' not in section or 'sp_register_probe' not in section or 'sp_switch' not in section:raise ValueError('Missing actual frame/switch code')
 if simd:
  if 'fxsave64' not in section or 'fxrstor64' not in section or '%xmm15' not in section or 'ldmxcsr' not in section:raise ValueError('Missing actual full selected-state preservation')
 elif re.search(r'%[xyz]mm\d+|fxsave|fxrstor|ldmxcsr',section):raise ValueError('SIMD leaked into general-register-only profile')
 # General-register-only compiler promise checked on actual complete adapted C.
 if re.search(r'%[xyz]mm\d+',c_assembly):raise ValueError('Compiler generated SIMD in C kernel')
 def frame(name):return max((v['bytes'] for k,v in records.items() if k==name or k.startswith(name+'.')),default=0)
 for required in ('sp_nested','sp_task','sp_worker','sp_interrupt','sp_bsp'):
  if not any(k==required or k.startswith(required+'.') for k in records):raise ValueError('Missing compiler stack record '+required)
 selected={k:v for k,v in records.items() if k.startswith(('sp_','ss_parked'))}
 if any(v['kind']!='static' for k,v in selected.items() if k not in ('sp_layout',)):raise ValueError('Dynamic frame needs a new bound')
 # Compiler frame counts include local spills. Inlined helpers have no separate
 # frame. Add every possibly called helper below once plus conservative call/
 # alignment allowance; recursion depth is exactly five for sp_nested.
 helpers=sum(frame(k) for k in ('sp_inside','sp_peer_progress','cmk2_cpu_index','pause_cpu'))
 task=frame('sp_task')+5*frame('sp_nested')+frame('sp_yield')+160
 entry=40+16+120+15+(512 if simd else 0)+frame('sp_interrupt')+helpers+128
 idle=frame('sp_bsp')+frame('sp_worker')+frame('sp_local_init')+frame('ss_parked')+256
 if task+32>8192 or entry+32>4096 or idle+32>16384:raise ValueError('Declared finite frame estimate exceeds available stack')
 return {'compiler_frames':selected,'finite_task_bytes':task,'finite_each_IST_bytes':entry,'finite_idle_bytes':idle,'limits':'Original fixed five-frame call tree and vector244→UD→GP with separate ISTs only; excludes unrecognized-fault logging/termination, arbitrary recursion and physical asynchronous faults. Canary survival is separate observed evidence.','assembly':assembly,'c_assembly':c_assembly}
def abi_audit(text,disassembly,target):
 rows=stack_records(text);matches=[v for k,v in rows.items() if k=='sp_abi_probe' or k.startswith('sp_abi_probe.')]
 if len(matches)!=1 or matches[0]['kind']!='static' or matches[0]['bytes']<=0 or 'sp_abi_probe' not in disassembly:raise ValueError('Missing bounded ABI frame')
 alignment=8 if target=='aapcs32' else 16
 return {'target':target,'public_stack_alignment':alignment,'compiler_frame_bytes':matches[0]['bytes'],'five_frame_bytes':5*matches[0]['bytes'],'scope':'Actual compiled fixed call tree; no exception-frame or target-execution claim.'}
