"""Register actually read passages; preserve inaccessible normative-source gaps."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
def write(p,v):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
def main():
 records=[
 ('R-LSAC-ACPI','ACPI6.6 MADT and local APIC tables','https://uefi.org/specs/ACPI/6.6/05_ACPI_Software_Programming_Model.html','ACPI6.6',
  ['5.2.12/Table5.19: MADT signature, length, checksum, LAPIC base, flags and records at44','5.2.12.2/Tables5.22–5.23: type0 length8, UID/APIC ID and Enabled/OnlineCapable flags'],
  'Reread archived actual official web-open passages from2026-10-10, lines704–744 and788–804. The archived extract hash is in source_access.json; this is not a bibliography/search-result inspection.',
  'Fresh detailed2026-10-17 requests returned403. Exact normative root/type9 passages remain uninspected; archived bounded text does not establish full ACPI conformance.'),
 ('R-LSAC-ACPICA','ACPICA table layout declarations','https://raw.githubusercontent.com/acpica/acpica/master/source/include/actbl2.h','Frozen Linux7.0.14-cmastery-b06 ACPICA headers, hashes recorded separately',
  ['actbl.h68–78,105–114,143–154: common header, RSDP and RSDT/XSDT entry widths','actbl2.h1258–1262,1317–1322,1363–1367,1411–1417: MADT, local APIC, override and local x2APIC record fields'],
  'Read the actual frozen kernel header lines; also opened the primary ACPICA upstream header and its type9 structure. Implementation layout authority only.',
  'Not a substitute for missing normative ACPI root/x2 semantics, selected hardware ID validity or OS conformance.'),
 ('R-LSAC-ELFV2','OpenPOWER64-Bit ELF V2 ABI Specification','https://files.openpowerfoundation.org/s/BQQamwXzkTAZQ2R/download','Revision1.5, December1,2020 (actual PDF title; website labels the package2.1.5)',
  ['2.2.1.5 and2.2.2.1, printed pp25–28/PDF37–40: caller TOC rules, r12 global entry and stack pointer','2.2.3.1–2.2.3.3, printed pp34–38/PDF46–50:32-byte minimum frame, alignment/backchain and optional parameter area'],
  'Downloaded the actual primary PDF, extracted and read specified complete pages; visually inspected rendered stack-related page. Exact PDF hash retained.',
  'Selected ABI passages only; Linux loader/syscall behavior is separately sourced. No physical Power hardware execution.'),
 ('R-LSAC-SYSCALL','Linux syscall architecture convention and PowerPC number table','https://www.man7.org/linux/man-pages/man2/syscall.2.html','Linux man-pages current page inspected2026-10-17 and frozen kernel syscall table',
  ['syscall(2) Architecture calling conventions tables: powerpc64 sc/r0/r3/cr0.SO and r3–r8 arguments','arch/powerpc/kernel/syscalls/syscall.tbl10–14: exit1,read3,write4'],
  'Opened and read the actual manual tables and the frozen kernel numeric table lines, with source hash recorded.',
  'The raw launcher uses only write/exit; it is a Linux-process verification adapter, not an ISO C or bare-board runtime requirement.')]
 values=[]
 for rid,title,url,edition,locators,method,limits in records:
  values.append({'id':rid,'title':title,'authors':['UEFI Forum' if rid.endswith('ACPI') else 'Primary specification/implementation contributors'],'url':url,'edition':edition,'locators':locators,'domains':['T53'],'kind':'manual','access':'Actual bounded passages inspected; unavailable passages explicitly excluded','role':'primary','assignable':True,'reason':'Separate wire layout, bounded mapping, C representation, target ABI and actual execution claims.','prerequisites':['Checked byte spans, physical versus virtual addresses, C integer widths and ABI conventions'],'inspection':{'date':'2026-10-17','method':method,'scope':'; '.join(locators),'limits':limits}})
 write(BASE/'resources.json',{'schema_version':1,'resources':values,'reference_gaps':['G-LCON-ACPI remains partial: exact ACPI6.6 normative RSDP/root/type9 passages inaccessible; current detailedrequests403.','No independent human review, learner performance or physical platform observation.'],'source_access':'research/v5/expansion/legacy_smp_acpi/source_access.json'})
 write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T53'],'inspection_kind':'Author inspection of bounded actual primary passages, not independent human review','research_input_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in [BASE/'resources.json',BASE/'source_access.json']}})
 print(json.dumps({'resources':len(values),'normative_root_x2':'reference-gap-retained'}))
if __name__=='__main__':main()
