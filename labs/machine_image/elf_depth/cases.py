"""Create one original, public ELF64LE segment fixture; no assessor files used."""
import argparse,struct
from pathlib import Path

def image():
    data=bytearray(0x1030);data[:7]=b'\x7fELF\x02\x01\x01'
    struct.pack_into('<HHIQQQIHHHHHH',data,16,2,62,1,0x400000,64,0,0,64,56,1,0,0,0)
    struct.pack_into('<IIQQQQQQ',data,64,1,5,0x1000,0x400000,0,0x30,0x50,0x1000)
    data[0x1000:]=bytes(range(0x30));return bytes(data)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('output',type=Path);a=p.parse_args()
    with a.output.open('xb') as stream:stream.write(image())
