"""Pure additive T48.01 boot-only support; no historical object replacement."""
import argparse,copy,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent
LAB=ROOT/'labs/kernel_followthrough/boot';REF=ROOT/'assessor/kernel_followthrough/boot'
SUPPORT='research/v5/expansion/legacy_kernel_boot/support.json';CAMPAIGN='reviews/v5/expansion/legacy_kernel_followthrough/boot_campaign.json';CONTRACT='labs/kernel_followthrough/boot/README.md';PROJECTS=['PR-T48-01']
NOTES={
'T48.01.a':('The boot-only probe checks all2048 supervisor large-page entries, saved boot arguments, DF,CR0/CR3/CR4/EFER,CS and actual pre-prologue entry RSP. Intel transition and pinned scalar ABI premises are explicit.','The6MiB linked image moves all absolute references, page-table/GDT/stack symbols and reserved kernel extent; its GRUB info pointer is separately observed. Identity mapping still covers each moved reference below4GiB.',['gcc-O0-correct-guest','clang-O2-correct-guest','gcc-O2-correct-gdb','clang-O2-correct-gdb','gcc-O2-relocated-guest','clang-O2-relocated-guest']),
'T48.01.b':('Only assembler, entry recorder and owned C17 probe are linked. Both compilers atO0/O2 produce actual C-entry/paging success, zero undefined symbols, discoverable aligned24-byte checksum-valid headers, ELF/maps/disassembly.','The reordered linker script places rodata before the header while preserving first32768-byte discovery,8-byte alignment and checksum. Actual images boot; header file offsets are retained.',['gcc-O0-correct-undefined','clang-O2-correct-undefined','gcc-O2-reordered-guest','clang-O2-reordered-guest']),
'T48.01.c':('Omitted EBX preservation retains nonzero entry EBX but supplies zero info to C; omitted far transfer leaves CS32 despite active long-mode paging. GDB distinguishes that first divergence; corrupted partial serial output is not a valid C entry. Matching repaired controls pass.','The exact stack-layout defect reaches an explicit alignment refusal. The frozen original assembler is also executed with the entry probe and its misalignment is retained, without altering original sources or receipts. Independent seven-day diagnosis remains unobserved.',['gcc-O2-omit-info-gdb','gcc-O2-omit-info-guest','clang-O2-omit-far-gdb','clang-O2-omit-far-guest','gcc-O2-misaligned-guest','clang-O2-misaligned-guest','gcc-O2-baseline-guest'])}
def write(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf-8',newline='\n')
def record():return json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T48-01.json').read_text())
def public_paths():return sorted(p.relative_to(ROOT).as_posix() for p in LAB.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
def attach(r):
 if r['project'] not in PROJECTS:return r
 for c in r['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 a=r.setdefault('additional_successor_support',[]);a[:]=[x for x in a if x.get('path')!=SUPPORT];a.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in r['criteria']],'scope':'Exact boot-only transition,build,causal mutation and changed-condition support;independent gates remain separate.'});a.sort(key=lambda x:x['path'])
 if CONTRACT not in r['shared_contracts']:r['shared_contracts'].append(CONTRACT)
 for path in public_paths():
  if path not in r['supplied_artifacts']:r['supplied_artifacts'].append(path)
 return r
def prepare():
 r=record();write(LAB/'criteria.json',{'schema_version':1,'criteria':[{'id':c['id'],'objective':c['objective'],'original_task':c['task'],'original_expected':c['verification']['expected'],'changed_conditions':c['variants'],'teaching':CONTRACT,'mechanism_checks':NOTES[c['objective']][2]} for c in r['criteria']]})
def main():
 sys.path[:0]=[str(ROOT),str(ROOT/'tools')];from check_kernel_boot_followthrough import validate_campaign
 v=json.loads((ROOT/CAMPAIGN).read_text());validate_campaign(v);r=record();path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T48-01.json';write(path,attach(r));updates=[]
 for c in r['criteria']:
  base,variant,selectors=NOTES[c['objective']];row={'id':c['id'],'project':r['project'],'original_objective':c['objective'],'original_source':path.relative_to(ROOT).as_posix(),'closure_rationale':base,'mechanism_checks':selectors,'variant_ids':[x['id'] for x in c['variants']],'dimensions':{'teaching':{'state':'complete-exact-boot-contracts','evidence':[CONTRACT,'labs/kernel_followthrough/boot/criteria.json']},'implementation':{'state':'implemented-original-boot-only-successor-and-controls','evidence':['assessor/kernel_followthrough/boot/boot.S','assessor/kernel_followthrough/boot/probe.c','assessor/kernel_followthrough/boot/ASSESSOR.md']},'author_validation':{'state':'passed-source-bound-GCC-Clang-actual-guest-and-GDB','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run;TCG-guest-separate','evidence':[]}},'external_blockers':v['open_gates'],'evidence_limits':['Only boot transition,not full kernel or SMP.','Historical Intel/AMD64 authority scopes retained.','Executed variants prepare independent delayed assessment;they do not constitute it.']};updates.append(row)
  for x in c['variants']:
   child=copy.deepcopy(row);child.update(id=x['id'],parent_id=c['id'],original_variant_task=x['task'],minimum_days=x['minimum_days'],closure_rationale=variant,variant_readiness='Exact author control executed;independent delayed learner gate unobserved.');updates.append(child)
 pins=[path,ROOT/CAMPAIGN,Path(__file__),HERE/'resources.json',HERE/'source_access.json',ROOT/'tools/check_kernel_boot_followthrough.py',*[ROOT/p for p in v['inspected_paths']]]
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in pins},'source_records':[{'id':x['id'],'path':(HERE/'resources.json').relative_to(ROOT).as_posix(),'passages':x['locators']} for x in json.loads((HERE/'resources.json').read_text())['resources']],'campaigns':[{'producer':'tools/run_kernel_boot_followthrough.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':v['open_gates'],'scope':'All3 originalT48.01 criteria and3 exact delayed variants only.'});print('Prepared6 exact boot updates')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--prepare',action='store_true');a=p.parse_args();prepare() if a.prepare else main()
