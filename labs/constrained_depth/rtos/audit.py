"""Observe actual ELF call edges/local frames without inventing whole-stack bounds."""
import re
from pathlib import Path

def inspect(directory,fp):
    directory=Path(directory);assembly=(directory/'assembly.txt').read_text(encoding='utf-8');blocks={};name=None
    for line in assembly.splitlines():
        m=re.match(r'^[0-9a-f]+ <([^>]+)>:',line)
        if m:name=m.group(1);blocks[name]=[]
        elif name:blocks[name].append(line)
    frames=[]
    for path in sorted(directory.glob('*.su')):
        for line in path.read_text(encoding='utf-8').splitlines():
            source,size,kind=line.rsplit('\t',2);frames.append({'function':source.rsplit(':',1)[-1],'bytes':int(size),'kind':kind,'source':source})
    if not frames or any(n not in {r['function'] for r in frames} for n in ('producer','consumer','controller','cr_irq0_body','cr_irq1_body')):raise ValueError('Missing actual local stack reports')
    port='\n'.join(blocks.get('xPortPendSVHandler',[])).lower()
    if any(s not in port for s in ('psp','basepri','vstmdbeq','vldmiaeq','vtaskswitchcontext','r11','lr')):raise ValueError('Missing actual conditional port context assembly')
    if fp:
        probe='\n'.join(blocks.get('cr_fp_probe',[])).lower()
        if any(s not in probe for s in ('s16','s31','vpush','vpop','vtaskdelay')):raise ValueError('Missing complete actual FP probe')
    elif 'cr_fp_probe' in blocks:raise ValueError('Integer variant retained unused FP probe')
    direct=[];indirect=[]
    for caller,lines in blocks.items():
        for line in lines:
            m=re.search(r'\bbl(?:x)?(?:\.w)?\s+[^<]+<([^>]+)>',line)
            if m:direct.append([caller,m.group(1)])
            elif re.search(r'\bblx\s+r\d+',line):indirect.append({'caller':caller,'instruction':line.strip()})
    if not any(a=='cr_irq0_body' and b=='xQueueGenericSendFromISR' for a,b in direct):raise ValueError('No actual ISR queue edge')
    return {'local_frames':frames,'generated_direct_calls':direct,'generated_indirect_calls':indirect,
            'architectural_nesting':['thread on PSP -> IRQ0 first frame on PSP; IRQ0 C work on MSP -> IRQ1 hardware frame and C work on MSP','PendSV uses PSP task context, BASEPRI masks admitted kernel interrupts, then scheduler selection and restore'],
            'manual_assembly_frames':{'cr_fp_probe':72 if fp else 'dead-stripped','port_task_context_excluding_C':204 if fp else 68,'possible_alignment':4},
            'whole_program_stack_bound':'unknown: local .su files plus call edges are evidence, not a closed proof for asynchronous nesting, indirect callbacks or all kernel paths',
            'unresolved_requirements':['Supply bounded callees for every indirect/cyclic path before claiming a complete stack bound.','Treat startup/semihosting failure, idle and nested handler stacks separately.']}
