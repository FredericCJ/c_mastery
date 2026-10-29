"""Conservative native call-chain accounting for the explicit VM probe image.

This is not general binary verification. It requires compiler static .su rows,
closed ELF imports, direct-call disassembly and the single audited callback
target. An unexpected indirect call, missing row or C call cycle is a refusal.
Interrupt entry uses the separately owned kernel/IST stacks, not this user stack.
"""
import argparse,json,re
from pathlib import Path

def audit(work):
    usage={}
    for p in work.glob('vm-*.su'):
        for line in p.read_text().splitlines():
            where,count,kind=line.split('\t')
            assert kind=='static',(p,line)
            name=where.split(':')[-1]
            usage[name]=max(usage.get(name,0),int(count))
    graph={};indirect=[];current=None
    for line in (work/'vm-disassembly.txt').read_text().splitlines():
        m=re.match(r'^[0-9a-f]+ <([^>]+)>:',line)
        if m:current=m[1];graph.setdefault(current,set());continue
        if current and re.search(r'\bcallq?\s',line):
            target=re.search(r'<([^>+]+)(?:\+[^>]*)?>',line)
            if target:graph[current].add(target[1])
            else:
                assert current.startswith('cvm_run'),(current,line)
                graph[current].add('callback');indirect.append({'from':current,'line':line.strip(),'audited_target':'callback'})
        # Tail jumps retain no new caller frame, but adding both frames is a
        # conservative upper bound. Ignore branches inside the same function.
        if current and re.search(r'\bjmpq?\s',line):
            target=re.search(r'<([^>+]+)>',line)
            if target and target[1]!=current:graph[current].add(target[1])
    def measure(name,path):
        assert name not in path,('native recursion requires new proof',path,name)
        assert name in usage,('missing static stack row',name)
        choices=[(0,[])]
        for child in graph.get(name,()):choices.append(measure(child,path+[name]))
        n,chain=max(choices,key=lambda x:x[0])
        # Additional return-address allowance is deliberately conservative;
        # GCC rows commonly already include it.
        return usage[name]+8+n,[name]+chain
    maximum,chain=measure('_start',[])
    return {'passed':True,'method':'static compiler rows + conservative direct/tail call graph + audited single cvm_host_fn callback; every call adds eight extra bytes','maximum_native_chain_bytes':maximum,'maximum_chain':chain,'usage':usage,'calls':{k:sorted(v) for k,v in graph.items()},'indirect_calls':indirect,'one_page_user_capacity_bytes':4088,'fits_one_page':maximum<=4088,'separate_stacks':{'normal_user_bytes':131072,'kernel_task_bytes':8192,'double_fault_IST_bytes':8192,'exception_frame_bytes':176},'scope':'Fixed probe source and compiler profile only; no claim about an arbitrary future callback, physical latency, SMP or unlisted compiler.'}

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('work',type=Path);a=p.parse_args();r=audit(a.work);(a.work/'stack-audit.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps({k:r[k] for k in ('passed','maximum_native_chain_bytes','maximum_chain','fits_one_page')}))
