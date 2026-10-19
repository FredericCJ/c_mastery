"""Independent bounded transition model and exact debugger evidence assertions."""
import re

def history(word):
 phase=observed=status=0;board=1;power=True
 for _ in range(4):
  op=word%8;word//=8;status=0
  if op in (0,4):
   if phase:status=3
   elif not power:status=2
   elif op==4:status=4
   else:phase=1;observed=0x413
  elif op in (1,2):
   if phase!=op:status=3
   else:phase+=1
  elif op==3:
   if phase!=3:status=3
  else:
   changed=op==5 or power!=(op==7)
   if op==5:board+=1
   else:power=op==7
   if changed:phase=observed=0
 return [status,phase,observed,511 if power else 383,board]

def check_replay(text):
 lines=text.splitlines()
 if len(lines)!=4096:raise ValueError('Incomplete transition histories')
 for word,line in enumerate(lines):
  if list(map(int,line.split()))!=[word,*history(word)]:raise ValueError('Transition mismatch '+str(word))
 return {'histories':4096,'steps_per_history':4,'model':'independent finite identity/phase/unknown-power transitions'}

def check_probe(value,profile=''):
 rows=value['rows'];kind=value['kind'];commands=[r for r in rows if 'command' in r];labels={r['label']:r for r in rows if 'label' in r}
 errors=[r for r in commands if not r['ok']]
 if any(not isinstance(r.get('elapsed_ns'),int) or r['elapsed_ns']<0 for r in commands):raise ValueError('Missing observed host timing')
 def command(name):
  return [r['output'] for r in commands if r['command']==name]
 def outcome(label,**fields):
  r=labels[label]
  if any(r['values'][k]!=v for k,v in fields.items()):raise ValueError('DC_RESULT_CHECK:'+label)
  if not isinstance(r['pc'],int) or not isinstance(r['sp'],int) or not isinstance(r['lr'],int):raise ValueError('Missing actual registers')
  return r
 outcome('initialized',dc_input=5,dc_output=0,dc_counter=0,dc_done=0)
 if kind!='limits' and errors:raise ValueError('Unexpected debugger refusal')
 if kind in ('state','result'):
  outcome('checksum-one',dc_output=30);outcome('checksum-changed',dc_output=45);outcome('finished',dc_output=10,dc_counter=9,dc_done=1)
  if kind=='state':
   if not command('info registers') or not command('info line *$pc') or not command('x/4ub dc_bytes'):raise ValueError('Missing correlation evidence')
   if profile in ('gcc-release','gcc-lto') and not any('dead = <optimized out>' in s for s in command('info locals')):raise ValueError('Missing actual unavailable value')
   if profile=='gcc-release' and not any(re.search(r'\bb\.w\b',s) and 'dc_leaf' in s for s in command('disassemble dc_tail')):raise ValueError('Missing actual tail branch')
   if profile=='gcc-release' and not any('tail call' in s for s in command('info frame')):raise ValueError('Missing virtual tail-frame distinction')
   if profile=='gcc-lto' and 'cl_reset' not in labels['checksum-entry']['symbol']:raise ValueError('Missing actual inline attribution')
 elif kind=='access':
  a=outcome('first-write',dc_counter=7);b=outcome('same-value-write',dc_counter=7)
  if a['pc']==b['pc'] or 'dc_checkpoint' in b['symbol']:raise ValueError('No same-value access stop')
 elif kind=='change':
  outcome('changed-write',dc_counter=7);r=outcome('same-value-skipped-to-checkpoint',dc_counter=7)
  if 'dc_checkpoint' not in r['symbol']:raise ValueError('Change watch stopped at equal store')
 elif kind=='limits':
  if len(errors)!=2 or {r['command'] for r in errors}!={'awatch *(unsigned long long *)&dc_counter','awatch dc_input'}:raise ValueError('Wrong resource refusals')
  if not any('cannot be implemented' in r['output'] for r in errors) or not any('not enough available hardware resources' in r['output'] for r in errors):raise ValueError('Unrelated refusal')
  for name in ('awatch dc_counter','awatch dc_output'):
   if not any('Hardware access' in s for s in command(name)):raise ValueError('Missing supported watchpoint')
  if not command('show remote hardware-watchpoint-limit') or not command('show remote hardware-watchpoint-length-limit'):raise ValueError('Missing explicit configured limits')
 else:raise ValueError('Unknown probe kind')
 return {'kind':kind,'observed_stops':len(labels),'observed_commands':len(commands),'scope':'Original QEMU target; resource limit is debugger policy, not physical comparator capacity.'}
