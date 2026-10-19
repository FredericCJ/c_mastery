"""Run inside GDB. Saved bounded observations are not native core files."""
import gdb,json,os
mode=os.environ.get('DX_GDB_MODE','counter');records=[]
def cmd(s):
 try:r=gdb.execute(s,to_string=True)
 except gdb.error as e:r='GDB_UNAVAILABLE:'+str(e)
 records.append({'command':s,'output':r});return r
for s in ('set pagination off','set confirm off','set disable-randomization off','set print pretty off','set debuginfod enabled off'):cmd(s)
if mode=='counter':
 for s in ('break dx_marker if stage == 10 && used == capacity','run counter','up','print used','print capacity','info registers rax rbx rbp rsp rip','x/4i $pc','stepi','watch used','continue','print used','bt full','info breakpoints','continue'):cmd(s)
elif mode=='candidate':
 for s in ('break dx_marker if stage == 40','run candidate','up','print candidate','print used','watch used','continue','print used','print violations','bt full'):cmd(s)
elif mode=='scope':
 for s in ('break dx_marker if stage == 20','run scope','up','print status','up','print status','bt full','delete breakpoints','break dx_marker if stage == 11','continue','up','print status','delete breakpoints','break dx_marker if stage == 13','continue','bt full','delete breakpoints','break dx_marker if stage == 22','continue','up','print vanished','bt full','info line *$pc','x/8i $pc'):cmd(s)
elif mode=='relocate':
 for s in ('break dx_marker if stage == 50','run relocate','up','print p','watch *p','delete 1','break dx_marker if stage == 51','continue','delete 2','delete 3','break dx_marker if stage == 52','continue','up','print p','watch *p','continue','info breakpoints','print *p'):cmd(s)
elif mode in ('lost','cycle'):
 for s in ('break dx_wait_snapshot','run '+('cycle' if mode=='cycle' else ''),'info threads','thread apply all bt full','continue','thread apply all bt full','continue'):cmd(s)
elif mode=='thread-watch':
 for s in ('break main','run','watch predicate','continue','info threads','thread apply all bt','info breakpoints','continue'):cmd(s)
print('DX_GDB_JSON:'+json.dumps({'mode':mode,'records':records,'heap':'not-captured','ASLR':'preserved','scope':'bounded debugger observation;not full execution history'}))
