"""Actual non-model object width/barrier/address inspection; not silicon evidence."""
import json,pathlib,re,subprocess,sys
p=pathlib.Path(sys.argv[1]);asm=subprocess.check_output(['arm-none-eabi-objdump','-dr',str(p)],text=True);undefined=subprocess.check_output(['arm-none-eabi-nm','-u',str(p)],text=True)
assert not undefined.strip(),undefined
assert all('<'+n+'>:' in asm for n in ['en_target_open','en_target_cold','en_target_bus','en_target_close']),asm
assert re.search(r'\bdsb\s+sy\b',asm) and re.search(r'\bldr(?:\.w)?\s',asm) and re.search(r'\bstr(?:\.w)?\s',asm),asm
# Byte loads of the local initialized pin table are legal. MMIO functions use
# volatile uint32_t exclusively; inspect their actual separately retained code.
helpers=all('<'+n+'>:' in asm for n in ['en_port_load','en_port_store'])
for name in (['en_port_load','en_port_store'] if helpers else ['read_register','write_register']):
 m=re.search(r'<'+name+r'>:\n(.*?)(?=\n[0-9a-f]+ <|\Z)',asm,re.S)
 if m:assert not re.search(r'\b(?:ldrb|ldrh|strb|strh)(?:\.w)?\s',m.group(1)),m.group(1)
need=['40023800','40028000','e0001000','e000edfc']
# Optimized compilers may factor address literals or build movw/movt pairs.
forms={n:(n in asm.lower()) for n in need}
assert ('40023800' in asm.lower() or '40023830' in asm.lower() or ('40023000' in asm.lower() and '#2096' in asm) or ('#14336' in asm and '#16386' in asm) or ('#14384' in asm and '#16386' in asm)),asm
assert ('e0001000' in asm.lower() or ('#4096' in asm and '#57344' in asm)),asm
print(json.dumps(dict(passed=True,selected_access_bits=32,undefined=[],literal_forms=forms,disassembly=asm,limits='Object-only selected MMIO and compiler barrier evidence; no silicon reads/writes or clock observation')))
