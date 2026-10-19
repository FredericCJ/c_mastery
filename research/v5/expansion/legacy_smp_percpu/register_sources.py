"""Bind actually read ISA/ABI passages for the per-CPU followthrough."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
def write(p,v):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
def main():
 values=[
 ('R-LSP-INTEL','Intel SDM Volume3A','https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html','253668-060US September2016',
  ['6.14.1–6.14.5, printed6-16–6-19/PDF202–205:64-bit gate, error/frame alignment, unconditional saved SS:RSP, IRET and IST','7.7, printed7-16–7-17/PDF254–255: software task switching and64-bit TSS/RSP/IST layout','13.1.1–13.1.3, printed13-2–13-3/PDF458–459: SSE feature checks and CR0/CR4 initialization','13.3–13.4 opening, printed13-6–13-7/PDF462–463: selected state saving and cooperative versus preemptive responsibility'],
  'Read full listed pages of the actual private primary PDF, not a title/census entry. Exact document hash recorded.',
  'Historical selected passages, not current CPU errata or full XSAVE specification. Selected FXSAVE state excludes other enabled extensions.'),
 ('R-LSP-AMD64','System V AMD64 ABI','https://refspecs.linuxfoundation.org/elf/x86_64-abi-0.99.pdf','Draft0.99.6 July2,2012',
  ['3.2.1–3.2.2, printed15–16/PDF16–17: preserved registers, direction flag, public stack alignment and red zone','3.2.3 passing rules and Figure3.4, printed20–21/PDF21–22: scalar registers and preservation'],
  'Actual retained PDF read for startup and this immediate per-CPU composition; exact same document and bounded passages, not an inferred whole-ABI review.',
  'Scalar/stack scope only. Interrupt preservation exceeds ordinary callee-save requirements; kernel explicitly disables red-zone use.'),
 ('R-LSP-AAPCS32','Procedure Call Standard for the Arm Architecture','https://raw.githubusercontent.com/ARM-software/abi-aa/2025Q1/aapcs32/aapcs32.rst','2025Q1 issued7April2025',
  ['Processes, Memory and the Stack / The Stack, lines923–970: full-descending model and stack extent','Universal stack constraints and public-interface constraints, lines972–1001: SP within extent, word alignment and8-byte public alignment','Stack probing, lines1003–1010: probe interval and no recoverable data below allocated stack'],
  'Opened and read actual official tagged source at the listed sections, including adjacent normative constraints; not a search result or generic ABI recollection.',
  'Changed-ABI compile/frame calculation only; these ABI passages do not define a Cortex exception frame or prove physical execution.')]
 resources=[]
 for rid,title,url,edition,locators,method,limits in values:
  resources.append({'id':rid,'title':title,'authors':['Primary ISA/ABI contributors'],'url':url,'edition':edition,'locators':locators,'domains':['T53'],'kind':'manual','access':'Actual selected primary passages inspected','role':'primary','assignable':True,'reason':'Separate C object extents, compiler frames, ISA exception state and ABI preservation.','prerequisites':['PR-T53-02','C17 ownership/alignment/atomics','x86 descriptors and basic assembly'],'inspection':{'date':'2026-10-17','method':method,'scope':'; '.join(locators),'limits':limits}})
 write(BASE/'resources.json',{'schema_version':1,'resources':resources,'reference_gaps':['Current physical CPU errata and physical timing not established.','No full XSAVE/AVX-state implementation or arbitrary-fault recovery claimed.'],'source_access':'research/v5/expansion/legacy_smp_percpu/source_access.json'})
 write(BASE/'source_access.json',{'schema_version':1,'date':'2026-10-17','actual_documents':[{'path':'build/legacy_concurrency/intel3a.pdf','sha256':'135cfa8dcbbfe957e6fe1045b52423c14d3542920b62f9b0f397ee7113c55257','pdf_pages':[202,203,204,205,254,255,458,459,462,463]},{'path':'research/private/systems/web/psabi.pdf','sha256':'69e117a13b560b1b957c15f05a61b130ca56828f05dca951e27cb8e2ada14283','pdf_pages':[16,17,21,22]}],'web_inspection':{'url':values[2][2],'sections':values[2][4],'status':'actual tagged official source opened/read'},'limits':'Private source texts are not redistributed; only bounded inspection evidence and original analysis.'})
 write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T53'],'inspection_kind':'Author source inspection, not independent human review','research_input_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in (BASE/'resources.json',BASE/'source_access.json')}})
 print(json.dumps({'resources':len(resources)}))
if __name__=='__main__':main()
