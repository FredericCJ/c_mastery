"""Independent identity, allocation-schedule, packing and wire-encoding models."""
import argparse
import itertools
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def expected():
    alphabet = [b'', b'a', b'b', b'\0a', b'a\0']
    rows = []
    for sequence in range(625):
        for fail in range(4):
            word, capacity, attempts = sequence, 0, 0
            identities = {}
            for step in range(4):
                text = alphabet[word % 5]
                word //= 5
                status, slot, generation = 0, 99, 99
                if text not in identities:
                    new_capacity = capacity or 4
                    if capacity and len(identities) + 1 > capacity // 2:
                        new_capacity *= 2
                    if new_capacity != capacity:
                        attempts += 1
                        if attempts == fail:
                            status = 2
                    if status == 0:
                        capacity = new_capacity
                        identities[text] = len(identities)
                if status == 0:
                    slot, generation = identities[text], 1
                rows.append('I '+ ' '.join(map(str, [sequence,fail,step,status,slot,generation,capacity,attempts,int(capacity != 0)])))
    for mask in range(256):
        survivors = [i for i in range(8) if mask >> i & 1]
        moved = sum(position != old for position, old in enumerate(survivors))
        fields = [mask, 8-len(survivors), moved]
        for i in range(8):
            fields += [1,1,i+11] if i in survivors else [0,2,0]
        rows.append('P '+' '.join(map(str, fields)))
    for sign in [0,1]:
        for fraction in range(256):
            bits = (sign << 31) + 0x7f800000 + fraction
            rows.append(f'V {bits} {0x7fc00000 if fraction else bits}')
    return rows

def main():
    p=argparse.ArgumentParser();p.add_argument('--driver',type=Path);args=p.parse_args()
    fixture=json.loads((ROOT/'fixtures/object_runtime/extensions.json').read_text(encoding='utf8'))
    model=expected()
    assert len(model)==fixture['oracle_rows']==10768
    if args.driver:
        result=subprocess.run([str(args.driver),'oracle'],capture_output=True,text=True,timeout=90)
        assert result.returncode==0,result.stderr
        actual=result.stdout.splitlines()
        assert len(actual)==len(model),(len(actual),len(model))
        for n,(got,want) in enumerate(zip(actual,model)):
            assert got==want,(n,got,want)
    shape_model=[]
    for index,order in enumerate(itertools.permutations(range(1,5))):
        for remove,shadow,readonly in itertools.product(range(5),range(3),range(2)):
            base={key:key*10 for key in order}
            status=3 if readonly and remove==2 else 0
            if remove and status==0:del base[remove]
            child={1:91} if shadow==1 else {}
            for key in range(1,6):
                result=child.get(key,base.get(key,777));found=0 if key in child or key in base else 5
                shape_model.append('S '+' '.join(map(str,[index,remove,shadow,readonly,key,status,found,result])))
    assert len(shape_model)==json.loads((ROOT/'fixtures/object_runtime/shapes_ext.json').read_text())['oracle_rows']==3600
    if args.driver:
        result=subprocess.run([str(args.driver.with_name('object-runtime-shapes')),'oracle'],capture_output=True,text=True,timeout=90)
        assert result.returncode==0,result.stderr
        actual=result.stdout.splitlines();assert len(actual)==len(shape_model)
        for n,(got,want) in enumerate(zip(actual,shape_model)):assert got==want,(n,got,want)
    dictionary_model=[]
    for sequence in range(7776):
        code=sequence;values={}
        for step in range(5):
            operation=code%6;code//=6;status=0
            if operation<4:values[operation+1]=10*(operation+1)+step
            else:
                key=operation-3
                if key in values:del values[key]
                else:status=5
            fields=[sequence,step,operation,status,len(values)]
            for key in range(1,6):fields.extend([0 if key in values else 5,values.get(key,777)])
            dictionary_model.append('D '+' '.join(map(str,fields)))
    assert len(dictionary_model)==38880
    if args.driver:
        result=subprocess.run([str(args.driver.with_name('object-runtime-alternatives')),'oracle'],capture_output=True,text=True,timeout=90)
        assert result.returncode==0,result.stderr
        actual=result.stdout.splitlines();assert len(actual)==len(dictionary_model)
        for n,(got,want) in enumerate(zip(actual,dictionary_model)):assert got==want,(n,got,want)
    marking_model=[]
    for graph,mask,policy in itertools.product(range(64),range(8),range(3)):
        code=graph;edges={4:[1]}
        for node in range(1,4):edges[node]=[code%4] if code%4 else [];code//=4
        reachable={node for node in range(1,4) if mask>>(node-1)&1}|{4}
        pending=list(reachable)
        while pending:
            for child in edges[pending.pop()]:
                if child not in reachable:reachable.add(child);pending.append(child)
        live=sum(1<<(node-1)for node in reachable)
        marking_model.append('M '+' '.join(map(str,[graph,mask,policy,live,15^live])))
    assert len(marking_model)==1536
    if args.driver:
        result=subprocess.run([str(args.driver.with_name('object-runtime-marking')),'oracle'],capture_output=True,text=True,timeout=90)
        assert result.returncode==0,result.stderr
        actual=result.stdout.splitlines();assert len(actual)==len(marking_model)
        for n,(got,want) in enumerate(zip(actual,marking_model)):assert got==want,(n,got,want)
    generation_model=[]
    for graph,mask,target,ages in itertools.product(range(64),range(16),range(4),range(8)):
        code=graph;edges={4:target}
        for node in range(1,4):edges[node]=code%4;code//=4
        roots=[node for node in range(1,5) if mask>>(node-1)&1]
        def closure(initial):
            found=set(initial);pending=list(found)
            while pending:
                child=edges[pending.pop()]
                if child and child not in found:found.add(child);pending.append(child)
            return found
        minor=closure(roots+[4]);major=closure(roots)
        for tag,live_set,count in [('N',minor,len(minor)-1),('J',major,len(minor-major))]:
            live=sum(1<<(node-1) for node in live_set)
            young=sum(1<<(node-1) for node in live_set if node<4 and not(ages>>(node-1)&1))
            fields=[graph,mask,target,ages,live,young,count]+roots+[0]*(4-len(roots))
            fields.extend(edges[node] if node in live_set else 0 for node in range(1,5))
            generation_model.append(tag+' '+' '.join(map(str,fields)))
    assert len(generation_model)==65536
    if args.driver:
        result=subprocess.run([str(args.driver.with_name('object-runtime-generational')),'oracle'],capture_output=True,text=True,timeout=90)
        assert result.returncode==0,result.stderr
        actual=result.stdout.splitlines();assert len(actual)==len(generation_model)
        for n,(got,want) in enumerate(zip(actual,generation_model)):assert got==want,(n,got,want)
    large_model=[]
    for code,release,pins in itertools.product(range(81),range(16),range(16)):
        word=code;occupied=set();blocks={};statuses=[]
        for node in range(4):
            length=1+word%3;word//=3
            choices=[set(range(start,start+length)) for start in range(9-length)]
            chosen=next((p for p in choices if not(p&occupied)),None)
            statuses.append(2 if chosen is None else 0)
            if chosen is not None:blocks[node]=chosen;occupied|=chosen
        alive={node for node in blocks if not(release>>node&1) or pins>>node&1}
        used=set().union(*(blocks[node] for node in alive))
        large_model.append('L '+' '.join(map(str,[code,release,pins]+statuses+[sum(1<<n for n in alive),sum(1<<n for n in used),len(blocks)-len(alive)])))
    assert len(large_model)==20736
    if args.driver:
        result=subprocess.run([str(args.driver.with_name('object-runtime-generational')),'large-oracle'],capture_output=True,text=True,timeout=90)
        assert result.returncode==0,result.stderr
        actual=result.stdout.splitlines();assert len(actual)==len(large_model)
        for n,(got,want) in enumerate(zip(actual,large_model)):assert got==want,(n,got,want)
    card_model=[]
    for width,padded,object,field,target in itertools.product([1,2,4,8],range(2),range(8),range(2),range(17)):
        status,roots,scanned=(0,1<<(target-1),2*width) if 1<=target<=8 else (4,777,888)
        card_model.append('C '+' '.join(map(str,[width,padded,object,field,target,status,roots,scanned])))
    assert len(card_model)==2176
    if args.driver:
        result=subprocess.run([str(args.driver.with_name('object-runtime-card-epoch')),'oracle'],capture_output=True,text=True,timeout=90)
        assert result.returncode==0,result.stderr
        actual=result.stdout.splitlines();assert len(actual)==len(card_model)
        for n,(got,want) in enumerate(zip(actual,card_model)):assert got==want,(n,got,want)
    finalizer_model=[]
    for graph,mask,v0,v1 in itertools.product(range(64),range(16),range(5),range(5)):
        code=graph;edges={4:0}
        for node in range(1,4):edges[node]=code%4;code//=4
        live={node for node in range(1,5) if mask>>(node-1)&1}
        while True:
            prior=set(live)
            live|={edges[node] for node in prior if edges[node]}
            for key,value in [(3,v0),(1,v1)]:
                if value and key in prior:live.add(value)
            if live==prior:break
        bits=sum(1<<(node-1) for node in live)
        finalizer_model.append('F '+' '.join(map(str,[graph,mask,v0,v1,bits,15^bits])))
    assert len(finalizer_model)==25600
    if args.driver:
        result=subprocess.run([str(args.driver.with_name('object-runtime-finalizers')),'oracle'],capture_output=True,text=True,timeout=90)
        assert result.returncode==0,result.stderr
        actual=result.stdout.splitlines();assert len(actual)==len(finalizer_model)
        for n,(got,want) in enumerate(zip(actual,finalizer_model)):assert got==want,(n,got,want)
    rc_model=[]
    for graph,mask,mode in itertools.product(range(4096),range(8),range(2)):
        code=graph;edges={}
        for node in range(1,4):
            edges[node]=[]
            for field in range(2):edges[node].append(code%4);code//=4
        live={node for node in range(1,4) if mask>>(node-1)&1};pending=list(live)
        while pending:
            for child in edges[pending.pop()]:
                if child and child not in live:live.add(child);pending.append(child)
        refs={node:int(bool(mask>>(node-1)&1)) for node in range(1,4)}
        for node in live:
            for child in edges[node]:
                if child:refs[child]+=1
        bits=sum(1<<(node-1) for node in live)
        rc_model.append('R '+' '.join(map(str,[graph,mask,mode,bits]+[refs[node] if node in live else 0 for node in range(1,4)])))
    assert len(rc_model)==65536
    if args.driver:
        result=subprocess.run([str(args.driver.with_name('object-runtime-rc')),'oracle'],capture_output=True,text=True,timeout=90)
        assert result.returncode==0,result.stderr
        actual=result.stdout.splitlines();assert len(actual)==len(rc_model)
        for n,(got,want) in enumerate(zip(actual,rc_model)):assert got==want,(n,got,want)
    vm_model=[]
    for graph,mask in itertools.product(range(4096),range(8)):
        code=graph;edges={}
        for node in range(3):
            edges[node]=[]
            for field in range(2):edges[node].append(code%4);code//=4
        live={node for node in range(3) if mask>>node&1};pending=list(live)
        while pending:
            for child in edges[pending.pop()]:
                if child and child-1 not in live:live.add(child-1);pending.append(child-1)
        forwarding={node:index+1 for index,node in enumerate(sorted(live))}
        fields=[]
        for node in sorted(live):
            fields.extend(forwarding[child-1] if child else 100+node for child in edges[node])
        fields.extend([-1]*(6-len(fields)))
        vm_model.append('VM '+' '.join(map(str,[graph,mask,len(live)]+[forwarding.get(node,0) for node in range(3)]+fields)))
    assert len(vm_model)==32768
    if args.driver:
        result=subprocess.run([str(args.driver.with_name('object-runtime-vm')),'oracle'],capture_output=True,text=True,timeout=90)
        assert result.returncode==0,result.stderr
        actual=result.stdout.splitlines();assert len(actual)==len(vm_model)
        for n,(got,want) in enumerate(zip(actual,vm_model)):assert got==want,(n,got,want)
    mechanisms={'storage':len(model),'prototype_shapes':len(shape_model),'dictionary':len(dictionary_model),'active_marking':len(marking_model),'generational_composition':len(generation_model),'pinned_large_objects':len(large_model),'card_epochs':len(card_model),'ephemeron_value_graphs':len(finalizer_model),'reference_count_lifecycles':len(rc_model),'actual_vm_relocation':len(vm_model)}
    print(json.dumps({'executed':bool(args.driver),'oracle_rows':sum(mechanisms.values()),'by_mechanism':mechanisms,'limits':'Original bounded runtime only; no arbitrary-pointer, physical capability-target, independent-learner or all-interleaving claim.'}))

if __name__=='__main__':main()
