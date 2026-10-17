#!/usr/bin/env python3
"""Independent finite mathematical oracles; host ctypes is not a target dependency."""
import argparse
import ctypes as C
import itertools
import json
from pathlib import Path

U=C.c_uint32
Z=C.c_size_t
I=C.c_int
P=C.POINTER
MAX=2**32-1

def check(condition, description):
    if not condition:
        raise AssertionError(description)

def b04(lib):
    lib.b4_serial.argtypes=[U,U,C.c_uint,P(I)]
    lib.b4_window.argtypes=[U,U,U,C.c_uint,P(I)]
    count=0
    for bits in range(2,8):
        modulus=2**bits
        for a,b in itertools.product(range(modulus),repeat=2):
            out=I(71);delta=(b-a)%modulus
            status=lib.b4_serial(a,b,bits,C.byref(out))
            expected=0 if delta==0 else -1 if delta<modulus//2 else 1
            check((status==3 and out.value==71) if delta==modulus//2 else
                  (status==0 and out.value==expected),('serial',bits,a,b))
            count+=1
    class Graph(C.Structure):
        _fields_=[('cost',(U*6)*6),('vertices',C.c_uint)]
    lib.b4_certificate.argtypes=[P(Graph),C.c_uint,P(U),P(C.c_uint)]
    # All 3-vertex directed graphs with weights absent,0,1. Enumerate simple paths.
    edges=[(a,b) for a in range(3) for b in range(3) if a!=b]
    for weights in itertools.product((MAX,0,1),repeat=6):
        g=Graph();g.vertices=3
        for a,b in itertools.product(range(6),repeat=2):g.cost[a][b]=MAX
        for (a,b),w in zip(edges,weights):g.cost[a][b]=w
        ds=[0,MAX,MAX];ps=[MAX]*3
        for end in (1,2):
            paths=[]
            for k in range(3):
                for inner in itertools.permutations([x for x in (1,2) if x!=end],k):
                    nodes=(0,)+inner+(end,)
                    ws=[g.cost[a][b] for a,b in zip(nodes,nodes[1:])]
                    if all(w!=MAX for w in ws):paths.append((sum(ws),nodes))
            if paths:
                # Choose fewest edges among equal paths to obtain a rooted tree.
                ds[end],path=min(paths,key=lambda x:(x[0],len(x[1]),x[1]))
                ps[end]=path[-2]
        d=(U*3)(*ds);p=(C.c_uint*3)(*ps)
        check(lib.b4_certificate(C.byref(g),0,d,p)==0,('graph',weights,ds,ps))
        count+=1
    lib.b4_cache_extent.argtypes=[Z,Z,Z,Z,P(Z),P(Z)]
    for line in (1,2,4,8,16):
        for off in range(66):
            for n in range(66):
                a=Z(999);b=Z(999)
                status=lib.b4_cache_extent(off,n,line,64,C.byref(a),C.byref(b))
                valid=off<=64 and n<=64-off
                expected=(off,off) if n==0 else (min(range(off,off+n))//line*line,(max(range(off,off+n))//line+1)*line)
                valid=valid and expected[1]<=64
                check((status==0 and (a.value,b.value)==expected) if valid else
                      (status==2 and (a.value,b.value)==(999,999)),('cache',line,off,n))
                count+=1
    return count

def b05(lib):
    count=0
    class Task(C.Structure):_fields_=[('cost',U),('period',U),('jitter',U)]
    lib.b5_response.argtypes=[U,U,U,P(Task),Z,P(U)]
    # Independent candidate inequality enumeration, not a repeated recurrence.
    for own,blocking,cost,period,jitter in itertools.product(range(1,5),range(3),range(4),range(1,7),range(3)):
        tasks=(Task*1)(Task(cost,period,jitter));out=U(999)
        candidates=[r for r in range(own+blocking,41)
                    if own+blocking+len(range(0,r+jitter,period))*cost<=r]
        status=lib.b5_response(own,blocking,40,tasks,1,C.byref(out))
        check((status==0 and out.value==candidates[0]) if candidates else
              (status==2 and out.value==999),('response',own,blocking,cost,period,jitter))
        count+=1
    class Instruction(C.Structure):_fields_=[('opcode',C.c_uint),('target',C.c_uint)]
    lib.b5_verify.argtypes=[P(Instruction),Z,P(C.c_uint)]
    # Path-state exploration records ALL (pc,height) states, then detects joins.
    choices=[(0,0),(1,0),(2,0),(5,0)]+[(op,t) for op in (3,4) for t in range(3)]
    for program in itertools.product(choices,repeat=3):
        pending=[(0,0)];seen=set();valid=True;peak=0
        while pending and valid:
            pc,h=pending.pop()
            if (pc,h) in seen:continue
            if any(prior_pc==pc and prior_h!=h for prior_pc,prior_h in seen):valid=False;break
            seen.add((pc,h));op,target=program[pc]
            if (op in (1,4) and h<1) or (op==2 and h<2):valid=False;break
            h+=1 if op==0 else -1 if op in (1,2,4) else 0
            if h>8:valid=False;break
            peak=max(peak,h)
            if op==5:
                valid=h==1;continue
            successors=([target] if op==3 else [pc+1,target] if op==4 else [pc+1])
            if any(x>=3 for x in successors):valid=False;break
            pending.extend((x,h) for x in successors)
        code=(Instruction*3)(*(Instruction(*x) for x in program));out=C.c_uint(999)
        status=lib.b5_verify(code,3,C.byref(out))
        check((status==0 and out.value==peak) if valid else
              (status!=0 and out.value==999),('verify',program,valid,peak,status,out.value))
        count+=1
    class Page(C.Structure):_fields_=[('present',C.c_uint),('user',C.c_uint),('writable',C.c_uint),('bytes',C.c_ubyte*16)]
    lib.b5_copy_to_user.argtypes=[P(Page),Z,P(C.c_ubyte),Z]
    payload=(C.c_ubyte*64)(*range(64))
    for mask in range(16):
        for address in range(65):
            for n in (0,1,15,16,17,32,64):
                pages=(Page*4)()
                for i in range(4):
                    pages[i].present=pages[i].user=1;pages[i].writable=bool(mask&(1<<i))
                    pages[i].bytes[:]=[99]*16
                before=bytes(pages);status=lib.b5_copy_to_user(pages,address,payload,n)
                inrange=address+n<=64
                allowed=inrange and all(mask&(1<<(x//16)) for x in range(address,address+n))
                if allowed:
                    expected=[99]*64;expected[address:address+n]=range(n)
                    check(status==0 and [v for p in pages for v in p.bytes]==expected,('copy-ok',mask,address,n))
                else:check(status==(3 if inrange else 2) and bytes(pages)==before,('copy-reject',mask,address,n))
                count+=1
    return count

def main():
    parser=argparse.ArgumentParser();parser.add_argument('family',choices=['b04_depth','b05_depth']);parser.add_argument('library')
    args=parser.parse_args();lib=C.CDLL(str(Path(args.library).resolve()))
    n=(b04 if args.family=='b04_depth' else b05)(lib)
    print(json.dumps({'family':args.family,'passed':True,'independent_cases':n,
                      'limits':'Finite host mathematical differential checks; no independent human review or hardware evidence.'}))
if __name__=='__main__':main()
