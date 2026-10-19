"""Build an explicit learner target and probe only its private QEMU UNIX socket."""
import argparse,hashlib,json,re,socket,subprocess,tempfile,time
from pathlib import Path
HERE=Path(__file__).resolve().parent
def build(args,run):
 args.build.mkdir(parents=True,exist_ok=True)
 cc=['arm-none-eabi-gcc'] if args.compiler=='gcc' else ['clang','--target=arm-none-eabi']
 flags=['-std=c17',args.optimization,'-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fstack-usage','-DCA_WIDE='+str(args.wide),'-DCA_TARGET_MUTANT='+str(args.mutant),'-DCA_RELOAD='+str(args.reload),'-I'+str(HERE),'-I'+str(args.runtime_include)]
 if args.compiler=='gcc':flags+=['-fno-tree-loop-distribute-patterns']
 sources=[HERE/'m4.c',args.source,args.helpers]
 if args.wide:sources +=[args.runtime/'runtime64_arm.c',args.runtime/'divide.c',args.runtime/'aeabi_uldivmod.S']
 objects=[]
 for i,source in enumerate(sources):
  obj=args.build/(str(i)+'.o');run([*cc,*flags,'-c',source,'-o',obj]);objects.append(obj)
 image=args.build/'analysis.elf';mapping=args.build/'analysis.map'
 link=['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nostartfiles','-nodefaultlibs',*objects,'-T',args.linker,'-Wl,--gc-sections','-Wl,-Map='+str(mapping),'-o',image]
 run(link);symbols=run(['arm-none-eabi-nm','-n',image]);undefined=run(['arm-none-eabi-nm','-u',image]);assembly=run(['arm-none-eabi-objdump','-d','-M','reg-names-std',image]);size=run(['arm-none-eabi-size',image]);imports=run(['arm-none-eabi-nm','-u',objects[1]])
 if undefined.strip() or re.search(r'lib(?:gcc|c|atomic)\.a|compiler-rt|crt\w*\.o',mapping.read_text(encoding='utf-8')):raise ValueError('Undeclared target runtime')
 for name,text in [('symbols.txt',symbols),('undefined.txt',undefined),('assembly.txt',assembly),('target-imports.txt',imports),('size.txt',size)]:(args.build/name).write_text(text,encoding='utf-8',newline='\n')
 return image,{'wide':args.wide,'reload':args.reload,'mutant':args.mutant,'image_sha256':hashlib.sha256(image.read_bytes()).hexdigest(),'sources':[str(p) for p in sources],'target_imports':imports.splitlines(),'size':size,'link_command':list(map(str,link))}
class Remote:
 def __init__(self,s):self.s=s;self.transcript=[]
 def byte(self):
  b=self.s.recv(1)
  if not b:raise ValueError('Unexpected debugger EOF')
  return b
 def send(self,text):
  b=text.encode();self.s.sendall(b'$'+b+b'#'+f'{sum(b)&255:02x}'.encode());self.transcript.append({'send':text})
  if self.byte()!=b'+':raise ValueError('Missing debugger ACK')
 def receive(self):
  if self.byte()!=b'$':raise ValueError('Missing debugger packet')
  raw=bytearray()
  while True:
   b=self.byte()
   if b==b'#':break
   raw+=b
  checksum=int(self.byte()+self.byte(),16)
  if checksum!=sum(raw)&255:raise ValueError('Wrong debugger checksum')
  self.s.sendall(b'+');expanded=bytearray();i=0
  while i<len(raw):
   c=raw[i];i+=1
   if c==42:
    if not expanded or i==len(raw):raise ValueError('Invalid debugger RLE')
    expanded.extend([expanded[-1]]*(raw[i]-29));i+=1
   elif c==125:
    if i==len(raw):raise ValueError('Invalid debugger escape')
    expanded.append(raw[i]^32);i+=1
   else:expanded.append(c)
  text=expanded.decode();self.transcript.append({'receive':text});return text
 def command(self,text):self.send(text);return self.receive()
 def word(self,address):
  text=self.command(f'm{address:x},4')
  if not re.fullmatch('[0-9a-fA-F]{8}',text):raise ValueError('Wrong exact four-byte target read')
  return int.from_bytes(bytes.fromhex(text),'little')
def probe(image):
 symbols=subprocess.check_output(['arm-none-eabi-nm','-n',image],text=True);names={row.split()[-1]:int(row.split()[0],16) for row in symbols.splitlines() if len(row.split())==3}
 result={'image_sha256':hashlib.sha256(image.read_bytes()).hexdigest(),'clock_hz':25000000,'machine':'mps2-an386','cpu':'cortex-m4','probe':'GDB all-stop via exclusive temporary UNIX socket','clock_resolution_ns':time.get_clock_info('monotonic').resolution*1e9,'samples':[]}
 with tempfile.TemporaryDirectory(prefix='ca-debug-') as directory:
  path=Path(directory)/'gdb.sock';cmd=['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-nic','none','-kernel',str(image),'-chardev','socket,path='+str(path)+',server=on,wait=off,id=gdb0','-gdb','chardev:gdb0','-S'];result['qemu_command']=cmd
  proc=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
  try:
   deadline=time.monotonic()+5
   while not path.exists():
    if proc.poll() is not None or time.monotonic()>deadline:raise ValueError('QEMU failed to create private debugger endpoint')
    time.sleep(.005)
   with socket.socket(socket.AF_UNIX,socket.SOCK_STREAM) as sock:
    sock.settimeout(5);sock.connect(str(path));remote=Remote(sock)
    if not remote.command('?').startswith(('S','T')):raise ValueError('Wrong initial stopped target')
    address=names['ca_checkpoint']&~1
    if remote.command(f'Z0,{address:x},2')!='OK':raise ValueError('Breakpoint unavailable')
    if not remote.command('c').startswith(('S','T')):raise ValueError('No readiness breakpoint')
    if remote.command(f'z0,{address:x},2')!='OK':raise ValueError('Cannot remove owned breakpoint')
    result['ready']=remote.word(names['ca_ready']);result['result']=remote.word(names['ca_result']);result['fault']=remote.word(names['ca_fault'])
    def sample(label):
     start=time.monotonic_ns();row={'label':label,'ticks':remote.word(names['ca_ticks']),'value':remote.word(0x40000004),'reload':remote.word(0x40000008),'control':remote.word(0x40000000)};row['host_read_cost_ns']=time.monotonic_ns()-start;row['host_monotonic_ns']=time.monotonic_ns();result['samples'].append(row)
    sample('initial')
    for cycle in range(2):
     remote.send('c');time.sleep(.06);sock.sendall(b'\x03')
     if not remote.receive().startswith(('S','T')):raise ValueError('No explicit interrupt stop')
     sample('run'+str(cycle));time.sleep(.08);sample('halt'+str(cycle))
    result['transcript']=remote.transcript
  finally:
   proc.terminate()
   try:out,err=proc.communicate(timeout=3)
   except subprocess.TimeoutExpired:proc.kill();out,err=proc.communicate()
   result['qemu_exit']=proc.returncode;result['qemu_stdout']=out;result['qemu_stderr']=err
 return result
def validate_probe(value,variant):
 s=value['samples']
 if len(s)!=5 or [x['label'] for x in s]!=['initial','run0','halt0','run1','halt1']:raise ValueError('Missing debugger stop/run observations')
 if variant=='stub':
  if value['ready']!=2 or value['result'] or value['fault']:raise ValueError('Wrong intended unsolved target result')
 else:
  if value['ready']!=1 or value['result']!=1431655765 or value['fault']!=496:raise ValueError('Wrong actual critical-path results')
  for a,b in ((s[1],s[2]),(s[3],s[4])):
   if a['ticks']!=b['ticks'] or a['value']!=b['value'] or b['host_monotonic_ns']-a['host_monotonic_ns']<70000000:raise ValueError('Selected virtual timer did not remain stopped across observed host pause')
  for a,b in ((s[0],s[1]),(s[2],s[3])):
   if variant==1:
    if a['ticks']!=b['ticks'] or a['control']!=0:raise ValueError('Wrong disabled-timer control')
   elif b['ticks']<=a['ticks'] or b['control']!=9:raise ValueError('No actual timer IRQ progress after resume')
 if any(x['host_read_cost_ns']<=0 or x['value']>x['reload'] for x in s):raise ValueError('Invalid raw probe values')
 return {'classification':'intended-stub-rejection' if variant=='stub' else 'intended-disabled-timer-rejection' if variant==1 else 'pass','scope':'Actual selected QEMU timer all-stop behavior, not physical MCU halt/freeze behavior or a WCET bound.'}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--probe',type=Path,required=True);a=p.parse_args();print(json.dumps(probe(a.probe)))
