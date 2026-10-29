"""Source-bound CMK successor matrix; every guest is freshly compiled and booted."""
import argparse,hashlib,json,re,subprocess,sys,time
from pathlib import Path
from audit_kernel_runtime_stack import audit
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main(work):
 work=work.resolve();assert work.is_relative_to(ROOT/'reviews/v5/expansion/legacy_kernel_followthrough');work.mkdir(parents=True,exist_ok=False)
 dirs=['labs/kernel_followthrough/contracts','assessor/kernel_followthrough/contracts','labs/kernel_followthrough/device','assessor/kernel_followthrough/device','labs/kernel_followthrough/guest','assessor/kernel_followthrough/guest','labs/kernel_followthrough/runtime','assessor/kernel_followthrough/runtime']
 paths=[p for d in dirs for p in (ROOT/d).glob('*') if p.is_file() and p.suffix in ('.c','.h','.S','.py')]
 paths += [ROOT/p for p in ('assessor/kernel_followthrough/boot/boot.S','assessor/kernel/model.c','labs/kernel/include/cmk_contract.h','labs/kernel/include/cmk_abi.h','labs/kernel/platform/link.ld','labs/kernel/platform/user.ld','labs/kernel/tools/guest.py','assessor/runtime/cvm.c','labs/runtime/include/cvm.h','assessor/memory_followthrough/byte_support.c','assessor/storage/store.c','labs/storage/include/store.h','labs/runtime_followthrough/services/services.h','assessor/runtime_followthrough/services/services.c','labs/runtime/adapters/store_host.c','labs/runtime/adapters/store_host.h','tools/run_kernel_guest_followthrough.py','tools/audit_kernel_runtime_stack.py','tools/run_kernel_matrix_followthrough.py')]
 before={p.relative_to(ROOT).as_posix():sha(p) for p in paths};commands=[];checks=[];guests=[]
 def check(name,value):checks.append({'name':name,'passed':bool(value)});assert value,name
 def run(name,cmd,timeout=180):
  cmd=list(map(str,cmd));t=time.monotonic();r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=timeout);p=work/(name+'.txt');p.write_text(r.stdout+r.stderr);commands.append({'label':name,'command':cmd,'exit_code':r.returncode,'expected_exit_code':0,'seconds':time.monotonic()-t,'output':p.relative_to(ROOT).as_posix()});assert r.returncode==0,(name,r.stdout,r.stderr);return r.stdout+r.stderr
 try:
  for tool in ('gcc','clang','ld','qemu-system-x86_64','grub-mkrescue','objdump','readelf','nm'):run(tool+'-version',[tool,'--version'])
  plans=[(cc,'O2',case,11932,0) for cc in ('gcc','clang') for case in range(36)]
  plans += [(cc,'O0',case,11932,0) for cc in ('gcc','clang') for case in (0,16,26,27,29,34)]
  plans += [('gcc','O2',15,q,d) for q,d in ((5966,0),(11932,2000000),(5966,2000000))]
  for cc,opt,case,q,d in plans:
   name=f'{cc}-{opt}-case{case}-q{q}-d{d}';folder=work/name
   run(name,[sys.executable,ROOT/'tools/run_kernel_guest_followthrough.py','--work',folder,'--cc',cc,'--optimization',opt,'--case',case,'--quantum',q,'--completion-delay',d]);receipt=json.loads((folder/'result.json').read_text());check(name+'-passed',receipt['passed']);out=(folder/'guest.txt').read_text();guests.append({'name':name,'receipt':(folder/'result.json').relative_to(ROOT).as_posix(),'sha256':sha(folder/'result.json'),'case':case,'compiler':cc,'optimization':opt,'quantum':q,'delay':d})
   check(name+'-fresh-PCI-snapshot','KF:PCI_VALIDATED:' in out or case in (2,3,4))
   check(name+'-kernel-no-hosted-loader','kf_load_image' not in (folder/'kernel-symbols.txt').read_text())
   if case not in (2,4):check(name+'-MMIO-fence-in-binary','mfence' in (folder/'kernel-disassembly.txt').read_text())
   if case==3:check(name+'-first-fault-retained','nested=0x0000000000000001' in out)
   if case==4:check(name+'-IST','DOUBLE_FAULT_IST_OWNED_STACK' in out)
   if case==5:check(name+'-minimal-before-truncation',out.index('KF:FIRST:')<out.index('KF:DIAG_TRUNCATED_FIRST_RETAINED'))
   if case==15:
    records=re.findall(r'KF:(?:DEVICE_INTERVAL|NONPREEMPTIBLE):[^\n]*ticks_before=0x([0-9a-f]+) ticks_after=0x([0-9a-f]+)',out);check(name+'-two-independent-nonpreemptible-intervals',len(records)==2 and all(a==b for a,b in records));check(name+'-injected-completion-delay',f'injected_delay=0x{d:016x}' in out)
   if case==22:check(name+'-all-16-rollback-prefixes',out.count('KF:ALLOC_ROLLBACK:')==16)
   if case in (26,27,29):
    record=audit(folder);(folder/'stack-audit.json').write_text(json.dumps(record,indent=2)+'\n');check(name+'-native-stack-premise',record['fits_one_page']==(case==27));guests[-1]['stack_audit']=(folder/'stack-audit.json').relative_to(ROOT).as_posix();guests[-1]['stack_audit_sha256']=sha(folder/'stack-audit.json')
   print(json.dumps({'completed':name,'commands':len(receipt['commands'])}),flush=True)
  check('all-source-identities-stable',all(sha(ROOT/p)==h for p,h in before.items()))
 except Exception as e:passed=False;error=repr(e)
 else:passed=True;error=None
 result={'passed':passed,'error':error,'commands':commands,'checks':checks,'guests':guests,'inspected_paths':before,'scope':'Author execution on one-struct lr_cpu TCG using owned guest code and ordinary disk files; models, physical observations, external review and delayed learner evidence remain distinct.'};(work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':passed,'error':error,'guests':len(guests),'checks':len(checks)}));return int(not passed)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);raise SystemExit(main(p.parse_args().work))
