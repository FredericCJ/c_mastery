"""Check actual ELF vector bytes against actual linked startup symbols."""
import json,pathlib,struct,subprocess,sys
d=pathlib.Path(sys.argv[1]);image=d/'image.elf';binary=d/'vectors.bin';subprocess.run(['arm-none-eabi-objcopy','--dump-section','.vectors='+str(binary),image],check=True)
s={}
for line in subprocess.check_output(['arm-none-eabi-nm',image],text=True).splitlines():
 fields=line.split()
 if len(fields)==3:s[fields[2]]=int(fields[0],16)
words=struct.unpack('<256I',binary.read_bytes());assert words[0]==s['__stack_top']==0x20010000 and words[1]==(s['xf_reset']|1)
assert words[16:19]==tuple(s[n]|1 for n in ['xf_irq_a','xf_irq_b','xf_irq_c'])
record=dict(passed=True,vector_base=0,initial_msp=words[0],reset_vector=words[1],reset_pc=words[1]&~1,irq_vectors=list(words[16:19]),limits=['Actual MPS2 image bytes, not F439 boot-pin or alias observation.']);(d/'vectors.json').write_text(json.dumps(record,indent=2)+'\n');print(json.dumps(record))
