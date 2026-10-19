"""Owned-reference integer model; exclude fabricated/double-drop histories."""
import argparse
import hashlib
import json
import subprocess
def histories():
    rows=[]
    def visit(word,strong,explicit,destructions,reclamations,trace):
        rows.append((word or '-',trace))
        if len(word)==7:return
        for op in 'rwdux':
            if op in 'rwd' and not strong:continue
            if op in 'ux' and not explicit:continue
            s,e,d,r=strong,explicit,destructions,reclamations;status=0
            if op=='r':s+=1
            elif op=='w':e+=1
            elif op=='d':
                s-=1
                if not s:d+=1;r+=int(not e)
            elif op=='u':
                if s:s+=1
                else:status=6
            elif op=='x':e-=1;r+=int(not e and not s)
            weak=e+int(s>0);visit(word+op,s,e,d,r,trace+f'{s},{weak},{d},{r},{status};')
    visit('',1,0,0,0,'');return rows
def run(driver):
    rows=histories();inputs=[r[0] for r in rows];answers=[t+'|'+t for _,t in rows]
    p=subprocess.run([driver,'oracle'],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=90);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(answers)
    for i,(a,b) in enumerate(zip(actual,answers)):assert a==b,(i,inputs[i],a,b)
    return dict(passed=True,histories=len(rows),max_operations=7,implementations=2,trace_sha256=hashlib.sha256(('\n'.join(inputs+answers)+'\n').encode()).hexdigest(),limits='Valid owned-reference sequential histories; actual racing executions and bounded RC11 model are separate.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
