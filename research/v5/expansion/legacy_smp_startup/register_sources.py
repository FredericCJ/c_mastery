"""Register actually inspected startup ISA/ABI/model passages, not full manuals."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
def write(p,v):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
def main():
 records=[
 ('R-LSS-INTEL','Intel64 and IA-32 Architectures Software Developer Manual, Volume3A','https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html','253668-060US, September2016',
  ['8.4.4.1–8.4.4.2/Table8-1, printed8-20–8-22/PDF276–278: low4KiB startup page, vector, INIT/SIPI sequence and acknowledgement','8.4.5, printed8-23/PDF279: APIC ID identification versus initial IDs','9.8.5–9.8.5.3, printed9-11–9-12/PDF323–324: PAE, CR3, EFER.LME, paging, identity mapping, descriptor and interrupt-transition requirements'],
  'Read the actual retained historical primary PDF pages and visually inspected the startup page/table rendering. Exact SHA retained in source_access.json.',
  'Historical selected passages only, not current CPU errata. The original teaching adapter uses one SIPI with no retry; the typical two-SIPI sequence and physical timing remain separately identified limitations.'),
 ('R-LSS-AMD64','System V AMD64 Architecture Processor Supplement','https://refspecs.linuxfoundation.org/elf/x86_64-abi-0.99.pdf','Draft0.99.6, July2,2012',
  ['3.2.1–3.2.2, printed15–16/PDF16–17: preserved registers, direction flag, stack alignment and red zone','3.2.3 passing rules and Figure3.4, printed20–21/PDF21–22: scalar INTEGER argument registers and register preservation'],
  'Read actual private primary PDF text at the specified pages and visually inspected rendered Figure3.3/3.2.2. Fresh URL open timed out; the already-local document was read and hashed.',
  'Scalar argument/entry scope only, not full variadic/vector ABI. The freestanding kernel deliberately uses -mno-red-zone; hosted red-zone assumptions are not imposed on interrupt handlers.'),
 ('R-LSS-QMP','QEMU QMP Reference Manual: CPU model expansion','https://www.qemu.org/docs/master/interop/qemu-qmp-ref.html','Actual current manual inspected2026-10-17; execution QEMU10.2.1',
  ['CpuModelInfo and CpuModelExpansionType: full versus static model/property expansion','query-cpu-model-expansion: model/machine/accelerator dependence and warning against mixing CPU-changing options','CpuModelInfo unavailable-features: reported feature unavailability and limitations'],
  'Opened and read actual official QMP page sections, including full-expansion environmental caveats and unavailable-features semantics; not just search snippets.',
  'Model expansion is only a narrow APIC/PAE/LM/NX preflight for the tested TCG/machine configuration; it is not proof that every CPU feature combination or hardware platform is runnable.')]
 resources=[]
 for rid,title,url,edition,locators,method,limits in records:
  resources.append({'id':rid,'title':title,'authors':['Primary ISA, ABI or emulator documentation contributors'],'url':url,'edition':edition,'locators':locators,'domains':['T53'],'kind':'manual','access':'Actual bounded primary passages inspected','role':'primary','assignable':True,'reason':'Separate ISA transition requirements, selected scalar ABI, C synchronization and emulator capability observations.','prerequisites':['T53.01 checked processor discovery','C17 lifetime and acquire/release','x86 control registers and page tables'],'inspection':{'date':'2026-10-17','method':method,'scope':'; '.join(locators),'limits':limits}})
 write(BASE/'resources.json',{'schema_version':1,'resources':resources,'reference_gaps':['Current physical-CPU errata and physical INIT/SIPI timing are not inspected or validated.','No independent human assessment or delayed learner attempt.'],'source_access':'research/v5/expansion/legacy_smp_startup/source_access.json'})
 write(BASE/'source_access.json',{'schema_version':1,'date':'2026-10-17','actual_documents':[
  {'path':'build/legacy_concurrency/intel3a.pdf','sha256':'135cfa8dcbbfe957e6fe1045b52423c14d3542920b62f9b0f397ee7113c55257','edition':'253668-060US September2016','pdf_pages':[276,277,278,279,323,324],'rendered_page':'build/smp_sources/intel-ap-startup.png'},
  {'path':'research/private/systems/web/psabi.pdf','sha256':'69e117a13b560b1b957c15f05a61b130ca56828f05dca951e27cb8e2ada14283','edition':'AMD64 draft0.99.6 July2,2012','pdf_pages':[16,17,21,22],'rendered_page':'build/smp_sources/sysv-stack.png'}],
  'access_attempts':[{'url':'https://refspecs.linuxfoundation.org/elf/x86_64-abi-0.99.pdf','result':'Fresh browser open timed out; actual local primary PDF separately read.'},{'url':'https://www.qemu.org/docs/master/interop/qemu-qmp-ref.html','result':'Opened actual primary sections; selected lines16168–16359 read, not the whole manual.'}],
  'scope':'Author inspection only. Local document availability and text extraction do not imply other passages were inspected; no redistributed book/PDF contents.'})
 write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T53'],'inspection_kind':'Bounded author source inspection, not independent human review','research_input_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in (BASE/'resources.json',BASE/'source_access.json')}})
 print(json.dumps({'resources':len(resources),'inspected':'selected actual ISA/ABI/QMP passages'}))
if __name__=='__main__':main()
