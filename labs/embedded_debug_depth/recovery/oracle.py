"""Independent byte/CRC interrupted-update and consume-before-permit oracle."""
import struct,zlib
def image(version):
 b=bytearray(64);struct.pack_into('<7I',b,0,0x31554944,version,16,0x40000,0x40009,0x20010000,25000000);struct.pack_into('<II',b,28,0x20010000,0x40009);b[36]=42;struct.pack_into('<I',b,52,version);struct.pack_into('<II',b,56,zlib.crc32(b[:56]),0xc04d17ed);return bytes(b)
def valid(b):return b[60:]==bytes.fromhex('ed174dc0') and int.from_bytes(b[56:60],'little')==zlib.crc32(b[:56]) and b in (image(1),image(3))
def record(seq,value):
 b=struct.pack('<IIII',seq,value,seq^0xffffffff,value^0xffffffff);return b+struct.pack('<I',zlib.crc32(b))+bytes.fromhex('a55a3cc3')
def check(text):
 expected=[];old=image(1);fresh=image(3)
 for kind in ('I','B'):
  for unit in range(17 if kind=='I' else 16):
   for value in range(33 if kind=='I' else 16):
    mask=(1<<value)-1 if kind=='I' else sum(255<<(8*b) for b in range(4) if value&(1<<b));candidate=bytearray(b'\xff'*64);candidate[:unit*4]=fresh[:unit*4]
    if unit<16:
     for bit in range(32):
      if mask&(1<<bit):candidate[unit*4+bit//8]&=fresh[unit*4+bit//8]|(255^(1<<(bit%8)))
    expected.append(f'{kind} {unit} {value} {0 if unit==16 else 5} 0 {1 if valid(candidate) else 0} {old.hex()} {candidate.hex()}')
 for cut in range(25):
  for discarded in range(2):
   # A committed attempt may be lost before permission reaches its caller;
   # retirement remains durable, so loss reduces availability, never the bound.
   expected.append(f'R {cut} {discarded} {0 if cut==24 else 5} {1 if cut==24 and discarded else 2} 4 2 {record(3,2).hex()} {record(2,1).hex()}')
 if text.splitlines()!=expected:raise ValueError('Incomplete or wrong independent update/retry replay')
 return {'bit_prefix_cuts':561,'byte_subset_cuts':256,'retry_restart_cases':50,'physical_power_loss':False}
