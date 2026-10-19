"""Owned ATmega328P emulator, exact generalized-limit companion marker."""
import argparse,json,select,subprocess,time
def main(binary):
    cmd=['/home/frede/cmastery-b06-tools/usr/bin/qemu-system-avr','-machine','arduino-uno','-bios',binary,'-nographic','-monitor','none']
    child=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,bufsize=0);data=b'';deadline=time.monotonic()+5
    try:
        while b'\n' not in data and time.monotonic()<deadline:
            if select.select([child.stdout],[],[],.1)[0]:
                part=child.stdout.read(1)
                if not part:break
                data+=part
    finally:
        child.terminate()
        try:tail,error=child.communicate(timeout=3)
        except subprocess.TimeoutExpired:child.kill();child.communicate();raise
    assert data==b'AVR:PASS\n',(data,error)
    print(json.dumps(dict(command=cmd,serial_hex=(data+tail).hex(),stderr=error.decode(),controlled_exit=child.returncode,size_t_bits=16,wire_bits=32,cases=['maximum0-digit9-refused','maximum0-zero-admitted','overcap413','malformed-suffix400','UINT32_MAX-wire-both-providers413','UINT32_MAX-plus-one-both-providers400','1024-admitted-before-size_t-conversion'],physical=False)))
    print('NV_AVR_PASS:actual16bit-size_t,generalized-maxima,retained-provider');return 0
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--binary',required=True);raise SystemExit(main(p.parse_args().binary))
