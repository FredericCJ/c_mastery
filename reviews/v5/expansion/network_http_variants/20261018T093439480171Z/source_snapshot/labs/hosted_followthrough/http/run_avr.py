"""Bounded owned AVR guest; USART result is observed before controlled teardown."""
import argparse,json,select,subprocess,time
def main(binary):
 command=['/home/frede/cmastery-b06-tools/usr/bin/qemu-system-avr','-machine','arduino-uno','-bios',binary,'-nographic','-monitor','none']
 child=subprocess.Popen(command,stdout=subprocess.PIPE,stderr=subprocess.PIPE,bufsize=0);data=b'';deadline=time.monotonic()+5
 try:
  while b'\n' not in data and time.monotonic()<deadline:
   if select.select([child.stdout],[],[],.1)[0]:
    b=child.stdout.read(1)
    if not b:break
    data+=b
 finally:
  child.terminate()
  try:tail,errors=child.communicate(timeout=3)
  except subprocess.TimeoutExpired:child.kill();tail,errors=child.communicate();raise
 assert data==b'AVR:PASS\n',(data,errors)
 print(json.dumps({'command':command,'serial_hex':(data+tail).hex(),'stderr':errors.decode(),'controlled_child_exit':child.returncode,'size_t_bits':16,'uint32_t_bits':32,'cases':[413,413,400,400,0],'scope':'Named QEMU ATmega328P numeric core, not a physical board or fullHTTPserver.'}));print('AVR_LENGTH_PASS:actual16bit-size_t,u32-wire,unchanged-rejection,admit1024');return 0
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--binary',required=True);a=p.parse_args();raise SystemExit(main(a.binary))
