"""Actual selected object access widths and closure, not silicon execution."""
import json,pathlib,re,subprocess,sys
p=pathlib.Path(sys.argv[1]);asm=subprocess.check_output(['arm-none-eabi-objdump','-dr',str(p)],text=True);undefined=subprocess.check_output(['arm-none-eabi-nm','-u',str(p)],text=True)
assert not undefined.strip(),undefined
assert all('<'+n+'>:' in asm for n in ['read_reg','write_reg','ep_f439_bus']),asm
literal='40028010' in asm.lower()
split=('#32784' in asm and '#16386' in asm)
# GCC O0 factors (base + reg*4) into (base/4 + reg)<<2.
factored='1000a004' in asm.lower() and re.search(r'\blsls?\s[^\n]*#2\b',asm)
assert literal or split or factored,asm
# These adapter accesses deliberately use 32-bit words. RM0090 also permits
# 8/16-bit register accesses; this check does not claim those are forbidden.
assert re.search(r'\bldr(?:\.w)?\s',asm) and re.search(r'\bstr(?:\.w)?\s',asm),asm
assert not re.search(r'\b(?:ldrb|ldrh|strb|strh)(?:\.w)?\s',asm),asm
print(json.dumps(dict(passed=True,register_base='0x40028010',selected_access_bits=32,undefined=[],disassembly=asm,limits='Object inspection only; no physical F439 bus effects observed')))
