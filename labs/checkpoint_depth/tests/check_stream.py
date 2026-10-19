"""Independent whole-buffer zlib oracle versus the incremental C implementation."""
import random
import struct
import subprocess
import sys
import zlib

def encode(sequence, payload):
    body = struct.pack('<BHH', 1, sequence, len(payload)) + payload
    return b'\xa5' + body + struct.pack('<I', zlib.crc32(body))

def oracle(data):
    result = []
    start = 0
    while start < len(data):
        if data[start] != 0xa5:
            start += 1; continue
        if len(data) - start < 2: break
        if data[start+1] != 1:
            start += 1; continue
        if len(data) - start < 6: break
        sequence, size = struct.unpack_from('<HH', data, start+2)
        if size > 32:
            start += 1; continue
        end = start + 10 + size
        if end > len(data): break
        body = data[start+1:end-4]
        if zlib.crc32(body) != struct.unpack_from('<I', data, end-4)[0]:
            start += 1; continue
        result.append(f'{sequence}:{data[start+6:end-4].hex()}')
        start = end
    return result

def main(executable):
    result = subprocess.run([executable, 'stream'])
    if result.returncode: return result.returncode
    randomizer = random.Random(0x5033)
    cases = [b'', b'\xa5'*256]
    for size in range(33):
        frame = encode(65535-size, bytes(randomizer.randrange(256) for _ in range(size)))
        damaged = bytearray(frame); damaged[-1] ^= 1
        cases += [frame, frame[:-1], bytes(damaged)+frame,
                  bytes([0xa5,2,0,0,0,0])+frame, bytes([0xa5,1,0,0,33,0])+frame]
    for _ in range(50):
        inner = encode(randomizer.randrange(65536), bytes(randomizer.randrange(256) for _ in range(3)))
        payload = inner + bytes(randomizer.randrange(256) for _ in range(32-len(inner)))
        bad = bytearray(encode(4,payload)); bad[-1] ^= 1
        cases.append(bytes(bad)+encode(4,b'\x00\xa5\xff'))
    observations = 0
    for data in cases:
        expected = oracle(data)
        for chunk in (1, 7, 256):
            result = subprocess.run([executable,'oracle',data.hex(),str(chunk)],capture_output=True,text=True)
            if result.returncode or result.stdout.splitlines() != expected:
                print('CP_CHECK: differential recovery',data.hex(),chunk,expected,result.stdout,result.stderr)
                return 1
            observations += 1
    print(f'Independent zlib whole-buffer comparisons: {observations}; deterministic seed 0x5033')
    return 0

if __name__ == '__main__':
    raise SystemExit(main(sys.argv[1]))
