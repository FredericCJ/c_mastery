"""Materialize only previously inspected primary Linux scheduler passages."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def main():
 access=json.loads((BASE/'source_access.json').read_text(encoding='utf-8'));records=[]
 for index,row in enumerate(access['sources'],1):
  records.append({'id':'R-LLPC-'+str(index).zfill(2),'title':('Linux EEVDF scheduler documentation','Linux SCHED_DEADLINE documentation','Linux sched(7) manual')[index-1],
   'authors':['Linux documentation contributors'],'url':row['url'],'edition':'Primary live documentation inspected 2026-10-17; exact executed source/configuration recorded separately',
   'locators':row['passages'],'domains':['T52'],'kind':'manual','access':'Actual bounded primary passages inspected','role':'comparison','assignable':True,
   'reason':'Separate fair ordering, reservation/admission, static priority, supervision and observed timing contracts.','prerequisites':['Periodic workloads, scheduler classes, monotonic clocks and process ownership'],
   'inspection':{'date':'2026-10-17','method':'Read actual opened primary documentation passages before authoring; exact selected kernel source/configuration inspected separately in source_access.json.','scope':'; '.join(row['observations']),'limits':'Selected passages only; no full kernel proof, physical timing or PREEMPT_RT claim.'}})
 write(BASE/'resources.json',{'schema_version':1,'resources':records,'reference_gaps':['Published exact C17/C23 texts and physical CPU/board timing remain separate obligations.','The original finite selector omits actual kernel hierarchy, placement, buddy and slice-protection mechanisms.'],'exact_kernel_inspection':'research/v5/expansion/legacy_realtime_linux/source_access.json'})
 inputs=[BASE/'resources.json',BASE/'source_access.json']
 write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T52'],'inspection_kind':'Author source inspection, not independent human review','research_input_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}})
 print(json.dumps({'resources':len(records),'source_passages':'exact selected guest source and config separately bound'}))
if __name__=='__main__':main()
