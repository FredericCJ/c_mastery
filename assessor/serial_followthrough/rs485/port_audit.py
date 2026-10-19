"""Actual selected port access forms; no execution or device physics claim."""
import json,subprocess,sys
p=sys.argv[1];s=subprocess.check_output(['arm-none-eabi-objdump','-dr',p],text=True)
assert all('<'+name+'>:' in s for name in ['r5_f439_prepare','r5_f439_send','r5_f439_io'])
assert 'dsb' in s and ('40020c00' in s or '40020c18' in s or '#3072' in s)
assert 'pu_f439_pins' in s and 'pu_f439_io' in s and 'r5_send' in s
assert 'strb' not in s and 'strh' not in s
assert '20000000' in s or '#536870912' in s
print(json.dumps(dict(passed=True,scope='actual 32-bit MMIO stores, DSB, provider calls and SRAM-bound admission; physical execution not claimed',disassembly=s)))
