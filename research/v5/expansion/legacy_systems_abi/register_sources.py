"""Bounded primary passages actually inspected; older drafts retain their editions."""
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];sys.path.insert(0,str(ROOT/'tools'))
from common import dump,sha
folder=Path(__file__).parent
entries=[
 ('AMD64','System V AMD64 ABI','https://refspecs.linuxfoundation.org/elf/x86_64-abi-0.99.pdf','Draft0.99.6 July2,2012',['3.2.1–3.2.3, printed14–22: register roles, stack, classification, parameter allocation, return ownership'],'Scalar/aggregate/mixed return contracts and stack-vector alignment; historical draft, not latest ABI.','amd64-0996.pdf'),
 ('DWARF','DWARF Debugging Information Format','https://dwarfstd.org/doc/DWARF5.pdf','Version5 February13,2017',['6.4–6.4.2.2, printed171–177; CFA/rules/CIE/FDE and row instructions'],'Virtual unwinding and code-location-specific recovery rules; .eh_frame encoding is an ABI/tool extension.','dwarf5.pdf'),
 ('AS','GNU assembler CFI directives','https://sourceware.org/binutils/docs/as/CFI-directives.html','Moving manual inspected2026-10-17',['7.13.1–7.13.3','7.13.9–7.13.20: CFA offsets, saved registers, restore and state stack'],'Assembler declaration semantics; no automatic proof that declarations match instructions.',None),
 ('ASM','GCC Extended Asm','https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/Extended-Asm.html','GCC15.2.0',['6.11.2.1 Volatile','6.11.2.2 Assembler Template','6.11.2.6 Clobbers and Scratch Registers, lines566–569'],'Compiler-visible dependencies, tied operands and early-clobber allocation.',None),
 ('ARMPCS','Procedure Call Standard for Arm Architecture','https://raw.githubusercontent.com/ARM-software/abi-aa/2025Q1/aapcs32/aapcs32.rst','2025Q1',['VFP register usage conventions, lines874–921','The Standard Variants, lines1425–1429','Argument Passing Conventions, lines2055–2072'],'Register preservation and base PCS for variadic functions.',None),
 ('ARMFLAGS','GCC ARM Options','https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/ARM-Options.html','GCC15.2.0',['3.20.5 -mfloat-abi, lines26–30'],'Separate instruction selection, argument convention and helper dependencies.',None),
 ('M4','Cortex-M4 Devices Generic User Guide','https://documentation-service.arm.com/static/5f2ac76d60a93e65927bbdc5','DUI0553B ID012616',['3.12.3 DMB, printed3-160','4.6.1 CPACR, printed4-48','4.6.6 Enabling the FPU, printed4-52'],'Exact M4 FPU access prerequisites, register-bank size and memory barrier scope.','dui0553b.pdf'),
 ('CMP','Intel SDM Volume2A','https://cdrdv2-public.intel.com/929353/253666-093-sdm-vol-2a.pdf','253666-093US September2026',['CMP, printed3-161–3-162'],'Operand-width/subtraction interpretation of comparison flags.','intel2a093.pdf'),
 ('CONTROL','Intel SDM Volume2B','https://cdrdv2.intel.com/v1/dl/getContent/671241','253667-093US September2026',['MFENCE, printed4-15','MOV control registers, printed4-32–4-34','SYSCALL, printed4-699–4-701'],'ISA memory fence, privilege checks and explicitly non-FRED versus FRED system-call state.',None),
 ('LLP64','Microsoft Abstract Data Models','https://learn.microsoft.com/en-us/windows/win32/winprog64/abstract-data-models','Updated2022-03-25',['ILP32/LLP64 paragraphs, lines30–35'],'Documented Windows widths compared with compile-only LLVM COFF manifest.',None),
 ('WRAP','GCC code generation conventions','https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html','Moving manual inspected2026-10-17; pinned15.2 HTML unavailable in browser',['3.18 -fwrapv, lines67–69'],'Named compiler extension changes signed arithmetic contract without changing ISO C17.',None),
 ('CLANG-WRAP','Clang command line reference','https://clang.llvm.org/docs/ClangCommandLineReference.html','Moving primary manual inspected2026-10-17',['-fwrapv and -fno-wrapv option entry'],'Clang extension cell explicitly separate from ISO overflow analysis.',None)]
resources=[]
for suffix,title,url,edition,locators,reason,pdf in entries:
    row={'id':'R-LABI-'+suffix,'title':title,'url':url,'authors':['Primary specification/tool authors'],
      'year':None,'edition':edition,'kind':'manual','access':'Named primary passages directly opened and read',
      'domains':['T08'],'role':'oracle','assignable':True,'reason':reason,
      'prerequisites':['C17 value/lifetime rules; original T08 dependency graph'],'locators':locators,
      'inspection':{'date':'2026-10-17','method':'Actual bounded text inspection; DWARF CFA page additionally rendered and visually read.',
        'scope':reason,'limits':'No whole-manual inspection, current-ABI equivalence, published ISO text, independent review or physical measurement inferred.'}}
    if pdf:row['inspection']['download_sha256']=sha(ROOT/'build/source-cache'/pdf)
    resources.append(row)
dump(folder/'resources.json',{'schema_version':1,'resources':resources})
dump(folder/'research_gate.json',{'status':'passage-inspected','domains':['T08'],'inspection_kind':'Author primary-passage inspection, not external review',
 'research_input_sha256':{(folder/'resources.json').relative_to(ROOT).as_posix():sha(folder/'resources.json')}})
lines=['# T08 inspected source passages','',
 'These bounded primary passages support this successor. The source inspection is author work, not independent review. Drafts retain their actual identity.','']
for row in resources:
    lines+=['- **'+row['id']+'**: ['+row['title']+']('+row['url']+'), '+row['edition']+'. '+ '; '.join(row['locators'])+'. '+row['reason']]
lines+=['','The exact published C17/C23 texts, uninspected private book passages and full runtime/loader bodies remain separate gaps. No whole-document inspection is claimed.']
(ROOT/'labs/machine_image/abi_depth/SOURCES.md').write_text('\n'.join(lines)+'\n',encoding='utf-8',newline='\n')
print({'inspected_resources':len(resources)})
