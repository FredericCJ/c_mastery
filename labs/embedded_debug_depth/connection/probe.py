"""Observe only a supplied original image through an owned private QEMU socket.

No flash programming, external target, TCP listener or debugger credential.
"""
import argparse,json,select,socket,subprocess,tempfile,threading,time
from pathlib import Path
def probe(image,gdb,kind='state',delay_ms=0):
 if not 0<=delay_ms<=10:raise ValueError('Use bounded local synthetic transport delay')
 with tempfile.TemporaryDirectory(prefix='dc-probe-') as directory:
  directory=Path(directory);sock=directory/'gdb.sock';report=directory/'observed.json';script=directory/'probe.gdb'
  program='''import gdb,json,time
rows=[]
def execute(command):
 start=time.monotonic_ns()
 try:
  value=gdb.execute(command,to_string=True);rows.append({'command':command,'ok':True,'output':value,'elapsed_ns':time.monotonic_ns()-start});return value
 except gdb.error as error:
  rows.append({'command':command,'ok':False,'output':str(error),'elapsed_ns':time.monotonic_ns()-start});return None
def capture(label):
 values={}
 for name in ('dc_output','dc_counter','dc_done','dc_input'):
  try:values[name]=int(gdb.parse_and_eval(name))
  except gdb.error as error:values[name]={'unavailable':str(error)}
 rows.append({'label':label,'values':values,'pc':int(gdb.parse_and_eval('$pc')),'sp':int(gdb.parse_and_eval('$sp')),'lr':int(gdb.parse_and_eval('$lr')),'symbol':gdb.execute('info symbol $pc',to_string=True)})
execute('break dc_checkpoint')
execute('continue');capture('initialized')
execute('x/4ub dc_bytes');execute('x/wx 0xe000ed00');execute('info registers')
if KIND=='result':
 execute('continue');capture('checksum-one');execute('continue');capture('checksum-changed');execute('continue');execute('continue');execute('continue');capture('finished')
elif KIND=='state':
 execute('break dc_checksum');execute('continue');capture('checksum-entry')
 execute('info locals');execute('info args');execute('disassemble /s dc_checksum');execute('stepi');execute('info locals');execute('info line *$pc');execute('delete 2')
 execute('continue');capture('checksum-one');execute('continue');capture('checksum-changed')
 execute('continue');execute('continue')
 execute('break dc_leaf');execute('continue');capture('tail-leaf');execute('bt');execute('info frame');execute('frame 1');execute('info frame');execute('frame 0');execute('disassemble dc_tail');execute('delete 3')
 execute('continue');capture('finished')
elif KIND=='access':
 execute('continue');execute('continue')
 execute('awatch dc_counter');execute('info watchpoints');execute('continue');capture('first-write');execute('x/i $pc');execute('continue');capture('first-write-checkpoint');execute('continue');capture('same-value-write');execute('x/i $pc')
elif KIND=='limits':
 execute('set remote hardware-watchpoint-limit 2');execute('set remote hardware-watchpoint-length-limit 4');execute('show remote hardware-watchpoint-limit');execute('show remote hardware-watchpoint-length-limit')
 execute('awatch *(unsigned long long *)&dc_counter');execute('awatch dc_counter');execute('awatch dc_output');execute('awatch dc_input');execute('info watchpoints')
elif KIND=='change':
 execute('continue');execute('continue');execute('watch dc_counter');execute('continue');capture('changed-write');execute('continue');capture('first-checkpoint');execute('continue');capture('same-value-skipped-to-checkpoint')
execute('detach')
with open(REPORT,'w') as output:json.dump({'kind':KIND,'rows':rows},output,indent=2)
'''
  endpoint=directory/'proxy.sock' if delay_ms else sock
  script.write_text('set pagination off\nset confirm off\nset debuginfod enabled off\nfile '+str(image)+'\ntarget remote '+str(endpoint)+'\npython\nKIND='+repr(kind)+'\nREPORT='+repr(str(report))+'\n'+program+'\nend\nquit\n',encoding='utf-8')
  command=['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-nic','none','-kernel',str(image),'-chardev','socket,path='+str(sock)+',server=on,wait=off,id=gdb0','-gdb','chardev:gdb0','-S'];process=subprocess.Popen(command,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
  relay=None;listener=None;stop=threading.Event();relay_errors=[]
  try:
   until=time.monotonic()+5
   while not sock.exists():
    if process.poll() is not None or time.monotonic()>until:raise ValueError('No private debugger socket')
    time.sleep(.005)
   if delay_ms:
    listener=socket.socket(socket.AF_UNIX,socket.SOCK_STREAM);listener.bind(str(endpoint));listener.listen(1);listener.settimeout(.2)
    def forward():
     try:
      while not stop.is_set():
       try:client,_=listener.accept();break
       except TimeoutError:continue
      else:return
      with client,socket.socket(socket.AF_UNIX,socket.SOCK_STREAM) as upstream:
       upstream.connect(str(sock));peers={client:upstream,upstream:client}
       while not stop.is_set():
        ready,_,_=select.select(list(peers),[],[],.2)
        for source in ready:
         data=source.recv(65536)
         if not data:return
         time.sleep(delay_ms/1000);peers[source].sendall(data)
     except OSError as error:
      if not stop.is_set():relay_errors.append(str(error))
    relay=threading.Thread(target=forward);relay.start()
   observed=subprocess.run([str(gdb),'-nx','--batch','-x',str(script)],capture_output=True,text=True,timeout=30)
   if observed.returncode or not report.exists():raise ValueError('Debugger failed: '+observed.stdout+observed.stderr)
   if relay_errors:raise ValueError('Local transport relay failed: '+str(relay_errors))
   result=json.loads(report.read_text(encoding='utf-8'));result.update(debugger_command=[str(gdb),'-nx','--batch','-x','<owned-script>'],qemu_command=command,script=script.read_text(encoding='utf-8'),debugger_output=observed.stdout+observed.stderr,transport={'kind':'owned UNIX-domain QEMU GDB endpoint','added_delay_ms_per_chunk':delay_ms,'credentials':'none','timing':'actual host monotonic command elapsed; added chunk delay is not physical network latency'})
   return result
  finally:
   stop.set()
   if relay:relay.join(timeout=3)
   if listener:listener.close()
   process.terminate()
   try:process.communicate(timeout=3)
   except subprocess.TimeoutExpired:process.kill();process.communicate()
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--image',type=Path,required=True);p.add_argument('--gdb',type=Path,required=True);p.add_argument('--kind',choices=['state','result','access','change','limits'],default='state');p.add_argument('--delay-ms',type=float,default=0);p.add_argument('--report',type=Path);a=p.parse_args();result=probe(a.image.resolve(),a.gdb.resolve(),a.kind,a.delay_ms)
 if a.report:a.report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'kind':result['kind'],'rows':len(result['rows']),'errors':[r for r in result['rows'] if r.get('ok') is False]}))
 else:print(json.dumps(result,indent=2))
