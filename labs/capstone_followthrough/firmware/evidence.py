"""Strict claim-category and release-byte checks; no hardware inference."""
import hashlib,pathlib,subprocess,tempfile,zlib
def sha(p):return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
REQUIRED={'parser':{'host-oracle'},'helpers':{'target-link'},'dma-lifetime':{'device-model'},'startup':{'target-execution'},'reset-recovery':{'cut-model'},'safe-output':{'state-model'}}

def timing(before,after,wraps,hz,claimed_ticks,maximum_ticks):
    if not (0<=before<65536 and 0<=after<65536 and 0<=wraps<=4 and hz>0 and maximum_ticks>=0):return False
    ticks=after-before+65536*wraps
    return ticks>=0 and ticks==claimed_ticks and ticks<=maximum_ticks
def matrix(rows,model):
    if {r['claim'] for r in rows}!=set(REQUIRED):return False
    for r in rows:
        if not REQUIRED[r['claim']]<=set(r.get('evidence',[])):return False
        if r.get('physical') and not {'physical-capture','deployed-byte-readback','board-identity'}<=set(r.get('evidence',[])):return False
        if r['claim']=='dma-lifetime' and model=='cpu-only' and r.get('supported_by_emulator'):return False
    return True
def release(elf,binary,manifest,objcopy='arm-none-eabi-objcopy'):
    elf=pathlib.Path(elf);binary=pathlib.Path(binary)
    for name in ['source','elf','binary','map']:
        entry=manifest.get(name,{})
        if not entry.get('path') or not pathlib.Path(entry['path']).is_file() or sha(entry['path'])!=entry.get('sha256'):return False
    if pathlib.Path(manifest['elf']['path']).resolve()!=elf.resolve() or pathlib.Path(manifest['binary']['path']).resolve()!=binary.resolve():return False
    if not manifest.get('flags') or manifest.get('physical_observation')!='not-run':return False
    raw=binary.read_bytes()
    if len(raw)<8 or int.from_bytes(raw[:4],'little')%8 or int.from_bytes(raw[4:8],'little')!=manifest.get('vector'):return False
    with tempfile.TemporaryDirectory() as d:
        generated=pathlib.Path(d)/'generated.bin';p=subprocess.run([objcopy,'-O','binary',str(elf),str(generated)],capture_output=True)
        return p.returncode==0 and generated.read_bytes()==raw
def debug_pair(stripped,symbols,manifest,objcopy='arm-none-eabi-objcopy'):
    if sha(stripped)!=manifest['stripped_sha256'] or sha(symbols)!=manifest['symbols_sha256']:return False
    with tempfile.TemporaryDirectory() as d:
        path=pathlib.Path(d)/'debuglink';p=subprocess.run([objcopy,'--dump-section','.gnu_debuglink='+str(path),str(stripped),str(path.parent/'discard.elf')],capture_output=True)
        if p.returncode or not path.exists():return False
        raw=path.read_bytes();end=raw.find(b'\0');offset=(end+4)&~3
        return end>=0 and raw[:end].decode()==pathlib.Path(symbols).name and len(raw)==offset+4 and int.from_bytes(raw[offset:],'little')==zlib.crc32(pathlib.Path(symbols).read_bytes())&0xffffffff
