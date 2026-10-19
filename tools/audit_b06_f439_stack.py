"""F439 Thumb stack paths; typed mechanical flash freshness callback set."""
import re

ASM={'st_reset':0,'st_fault':0,'fb_branch':0,'th_app_report':0,'eg_raw':0,'eg_load_insn':0,'eg_abort':0,'eg_busfault':8}
CALLBACKS={'rd','wr','sync','sync_io','h_fresh','emit','read_slot'}
def analyze(disassembly,stacks):
 edges={};indirect=set();current=None
 for line in disassembly.splitlines():
  h=re.match(r'^[0-9a-f]+ <([^>]+)>:',line)
  if h:current=h[1];edges.setdefault(current,set());continue
  if current is None:continue
  # An edge leaving the function is conservatively charged as an ordinary
  # call even if the compiler made it a tail branch. Local loop labels do not
  # create recursive calls. Terminal handoff resets MSP and is a separate root.
  instruction=re.search(r'\b(blx?|bx|b(?:\.[nw])?|b(?:eq|ne|cs|cc|mi|pl|vs|vc|hi|ls|ge|lt|gt|le)(?:\.[nw])?)\s',line)
  if not instruction:continue
  target=re.search(r'<([^>+]+)',line)
  if target:
   name=target[1]
   if name!=current or instruction.group(1) in ('bl','blx'):edges[current].add(name)
  elif re.search(r'\b(?:blx|bx)\s+(?:r[0-9]+|ip|fp|sl|sb|sp)\b',line) and current!='fb_branch':indirect.add(current)
 callbacks=CALLBACKS & edges.keys()
 for name in indirect:
  # f4_fresh receives FHIO, which has read/write/sync only; h_fresh is
  # FMIO.fresh, not a valid member of this callback parameter.
  selected=callbacks & {'rd','wr','sync'} if name=='f4_fresh' else callbacks
  edges[name].update(selected)
 sizes=dict(ASM);sizes.update(stacks);memo={}
 def bound(name,active=()):
  if name in active:raise ValueError('recursive/ambiguous call cycle: '+' -> '.join((*active,name)))
  if name not in edges:raise ValueError('missing linked target '+name)
  if name not in sizes:raise ValueError('missing static stack row '+name)
  if name in memo:return memo[name]
  choices=[(bound(child,(*active,name)),child) for child in edges[name]]
  child=max(choices,default=(0,''));memo[name]=sizes[name]+child[0];return memo[name]
 roots={name:bound(name) for name in ['st_main','th_report_main','eg_busfault']}
 # One precise BusFault basic frame + alignment padding, no nested fault/IRQ,
 # no floating point context. Handler includes its explicit8-byte save.
 exception=40+roots['eg_busfault'];return dict(roots=roots,precise_fault_allowance=exception,startup_bound=roots['st_main']+exception,report_bound=roots['th_report_main']+exception,callback_targets=sorted(callbacks),indirect_functions=sorted(indirect),limits='Selected acyclic linked Thumb paths and closed supplied nonreentrant callbacks; terminal handoff starts a separate stack, no IRQ/NMI/FP nesting or hostile application stack usage')

def self_test():
 def failed(text,rows):
  try:analyze(text,rows);return False
  except ValueError:return True
 text='0000 <st_main>:\n  0: bl 4 <a>\n0004 <a>:\n0008 <th_report_main>:\n000c <eg_busfault>:\n'
 rows={'st_main':8,'a':16,'th_report_main':24}
 positive=analyze(text,rows)['startup_bound']==72
 unknown=failed(text,{'st_main':8,'th_report_main':24})
 cycle=failed(text.replace('0004 <a>:\n','0004 <a>:\n  4: bl 0 <st_main>\n'),rows)
 recursive=failed(text.replace('0004 <a>:\n','0004 <a>:\n  4: bl 4 <a>\n'),rows)
 return dict(passed=positive and unknown and cycle and recursive,positive=positive,missing_stack_rejected=unknown,cycle_rejected=cycle,self_recursion_rejected=recursive)
