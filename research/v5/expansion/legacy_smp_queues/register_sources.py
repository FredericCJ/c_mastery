"""Register exact bounded queue/ordering readings; do not infer whole-source inspection."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,v):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
def main():
 prior=json.loads((ROOT/'research/v5/expansion/legacy_concurrency/resources.json').read_text(encoding='utf-8'));book=next(r for r in prior['resources'] if r['id']=='R-LCON-OSPP')
 rows=[
 ('R-LSQ-LAMPORT','Time, Clocks, and the Ordering of Events in a Distributed System','https://lamport.azurewebsites.net/pubs/time-clocks.pdf','CACM21(7), July1978, pp558–565',['Printed558–559/PDF1–2: process order, actual message edges, transitivity and concurrent events','Printed559 footnote1/PDF2: event granularity, interrupt arrival versus handler execution','Printed559–560/PDF2–3: logical clock condition implies ordered values, not its converse'],'Only these opening passages read; no full paper or physical clock synchronization theorem inspection claimed.'),
 ('R-LSQ-OSPP',book['title'],book['url'],'Second edition VolumeII, private document identity below',book['locators'],'Actual selected pages reread; lock-group invariants and textbook condition discipline, not universal POSIX signaling requirements.'),
 ('R-LSQ-C11','WG14 N1570 C11 draft','https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf','12April2011 draft',['5.1.2.4 paragraphs5–13/PDF36: synchronization, modification order and release sequence','5.1.2.4 paragraphs16–25/PDF37–38: happens-before, coherence and data races'],'Actual bounded draft passages already inspected and applied; exact published readable C17 remains unavailable.'),
 ('R-LSQ-RETAINED','Retained CMK cooperative and ring-3 scheduling source','assessor/kernel_smp/smp.c','Accepted retained source identities pinned below',['assessor/kernel_smp/smp.c: struct thread, yield_thread and scheduler bodies, fixed two-task CPU subsets under coarse lock','assessor/kernel_smp/ap.S: cmk2_switch six callee-save pushes, saved RSP and reverse restore','assessor/kernel_smp/kernel.c: activate and schedule bodies, global thread scan and affinity'],'Actual bounded source bodies read; not a fresh audit of every kernel/runtime/interrupt path. Restricted source evidence, never learner solution download.')]
 resources=[]
 for rid,title,url,edition,locators,limits in rows:
  r={'id':rid,'title':title,'authors':['Named primary author or retained curriculum contributors'],'url':url,'edition':edition,'domains':['T53'],'locators':locators,'kind':'book' if rid.endswith('OSPP') else 'documentation','access':'Actual bounded passages inspected','role':'primary','assignable':True,'reason':'Separate queue ownership, context publication and observable causal order.','prerequisites':['PR-T53-05','PR-T53-02','PR-T50-04','PR-T51-03'],'inspection':{'date':'2026-10-18','method':'Read actual named source bodies, private PDF pages and primary web PDF passages before final authorship.','scope':'; '.join(locators),'limits':limits}}
  if rid.endswith('OSPP'):r['inspection'].update({k:book['inspection'][k] for k in ('local_document_id','local_sha256')})
  resources.append(r)
 write(BASE/'resources.json',{'schema_version':1,'resources':resources,'reference_gaps':['Exact readable published C17 is unavailable; C11 draft passage provenance remains explicit.','No physical task-migration timing, calibrated cross-CPU clock or board evidence.']})
 paths=[ROOT/'assessor/kernel_smp'/p for p in ('smp.c','ap.S','kernel.c')]
 write(BASE/'source_access.json',{'schema_version':1,'date':'2026-10-18','actual_web_passages':[rows[0][2]],'actual_private_documents':[{'path':str(ROOT/'build/legacy_runtime_sources/n1570.pdf'),'sha256':sha(ROOT/'build/legacy_runtime_sources/n1570.pdf')},{'path':book['inspection']['local_document_id'],'sha256':book['inspection']['local_sha256'],'inspection':'Physical PDF89/98 reread; inherited identity verified in owning source register.'}],'actual_local_source_sha256':{p.relative_to(ROOT).as_posix():sha(p) for p in paths},'scope':'Opening Lamport partial-order/clock paragraphs and exact listed C/kernel/book passages only; private publications not redistributed.'})
 write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T53'],'inspection_kind':'Author source inspection, not independent human review','research_input_sha256':{p.relative_to(ROOT).as_posix():sha(p) for p in (BASE/'resources.json',BASE/'source_access.json')}})
 print(json.dumps({'resources':len(resources)}))
if __name__=='__main__':main()
