import json,subprocess,sys
s=subprocess.check_output(['arm-none-eabi-objdump','-dr',sys.argv[1]],text=True)
assert all('<'+n+'>:' in s for n in ['port_read','port_write','cn_f439_io'])
assert '40006400' in s or '#25600' in s,s
assert 'strb' not in s and 'strh' not in s and 'ldrb' not in s and 'ldrh' not in s
assert 'str' in s and 'ldr' in s
print(json.dumps(dict(passed=True,disassembly=s,limits='Selected 32-bit register access forms and address materialization only; not physical execution.')))
