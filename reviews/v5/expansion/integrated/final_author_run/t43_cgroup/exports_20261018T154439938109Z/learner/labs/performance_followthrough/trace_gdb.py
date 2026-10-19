"""GDB Python: single-step ONLY retained owned lower-bound function."""
import gdb,json
gdb.execute('set pagination off')
gdb.execute('set disable-randomization off')
gdb.execute('break algo_asm_lower')
gdb.execute('run')
rows=[]
while len(rows)<256:
 pc=int(gdb.parse_and_eval('$pc')); ins=gdb.selected_frame().architecture().disassemble(pc,count=1)[0]
 rows.append({'pc':pc,'asm':ins['asm'],'eax':int(gdb.parse_and_eval('$eax')),'ecx':int(gdb.parse_and_eval('$ecx'))})
 if ins['asm'].split()[0].startswith('ret'):break
 gdb.execute('stepi',to_string=True)
else:raise RuntimeError('bounded trace exhausted')
gdb.write('PF_TRACE:'+json.dumps(rows)+'\n')
gdb.execute('continue')
