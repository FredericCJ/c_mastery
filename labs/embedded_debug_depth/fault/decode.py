"""Offline original ELF32/Arm record attribution, refusing a mismatched digest.

The capture token is not a digest. The caller must supply the acquisition
envelope's independently recorded actual ELF SHA256; a source filename is not
an identity. This parser never executes or dereferences a captured address.
"""
import argparse,hashlib,json,struct
from pathlib import Path
def decode(text,image,expected_sha256):
 data=image.read_bytes()
 if hashlib.sha256(data).hexdigest()!=expected_sha256:raise ValueError('Image identity mismatch')
 if len(data)<52 or data[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',data,18)[0]!=40:raise ValueError('Selected ELF32 little-endian Arm only')
 lines=[s for s in text.splitlines() if s.startswith('DF_CAPTURE:')]
 if len(lines)!=1:raise ValueError('Exactly one complete capture required')
 values=[int(x,16) for x in lines[0][11:].split()]
 if len(values)!=14 or any(v<0 or v>0xffffffff for v in values):raise ValueError('Wrong bounded record')
 version,valid,cfsr,hfsr,er,mmfar,*frame=values
 if version!=1:raise ValueError('Unsupported record version')
 if valid!=29 or cfsr!=0x82 or hfsr or er not in (0xfffffff9,0xfffffffd) or not frame[7]&(1<<24) or frame[6]&1:raise ValueError('Not the selected complete precise MPU capture')
 pc=frame[6];phoff=struct.unpack_from('<I',data,28)[0];entry_size,count=struct.unpack_from('<HH',data,42);matches=[]
 if entry_size<32 or count>128 or phoff>len(data) or count*entry_size>len(data)-phoff:raise ValueError('Invalid program headers')
 for i in range(count):
  kind,offset,address,_,size,_,flags,_=struct.unpack_from('<8I',data,phoff+i*entry_size)
  if offset>len(data) or size>len(data)-offset:raise ValueError('Invalid ELF segment extent')
  if kind==1 and flags&1 and pc>=address and pc-address<size and size-(pc-address)>=2:matches.append(data[offset+pc-address:offset+pc-address+2])
 if len(matches)!=1:raise ValueError('Instruction not uniquely bound to executable bytes')
 instruction=int.from_bytes(matches[0],'little');opcode=instruction&0xf800
 if opcode not in (0x6000,0x6800):raise ValueError('Unsupported selected instruction')
 rn=(instruction>>3)&7;rt=instruction&7;displacement=((instruction>>6)&31)*4
 if rn>3 or opcode==0x6000 and rt>3:raise ValueError('Uncaptured operand')
 address=frame[rn]+displacement
 if address>0xffffffff or address!=mmfar:raise ValueError('Captured effective address mismatch')
 if opcode!=0x6000 or frame[rt]!=0x12345678:raise ValueError('Wrong original store operand')
 return {'version':version,'elf_sha256':expected_sha256,'pc':pc,'thumb16_hex':matches[0].hex(),'effective_address':address,'write':True,'operand':frame[rt],'confidence':'precise selected MPU data access; status was cleared before this owned instruction','physical_observation':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--capture',type=Path,required=True);p.add_argument('--image',type=Path,required=True);p.add_argument('--sha256',required=True);a=p.parse_args();print(json.dumps(decode(a.capture.read_text(encoding='utf-8'),a.image,a.sha256),indent=2))
