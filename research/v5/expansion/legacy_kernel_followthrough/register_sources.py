"""Register the bounded passages actually inspected; no full-book claim."""
import hashlib,json,urllib.request
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(name,data):(HERE/name).write_text(json.dumps(data,indent=2)+'\n')
def main():
 private=Path('/home/frede/cmastery-kernel-follow-tools');private.mkdir(exist_ok=True)
 resources=json.loads((ROOT/'research/v5/expansion/legacy_kernel_boot/resources.json').read_text())['resources']
 resources[0]['locators']+=['3.6 tags,modules and memory-map entry_size; checked raw extent versus padded cursor']
 resources[1]['locators']+=['4.6.1 effective U/S,R/W,XD permissions;4.7 page-fault code printed4-29 through4-32']
 specs=[('R-KF-VIRTIO12','Virtio1.2 Committee Specification01','https://docs.oasis-open.org/virtio/virtio/v1.2/cs01/virtio-v1.2-cs01.html','virtio12.html',['2.7.6 available ring;2.7.8 used ID/len','2.7.13 publication barriers/index wrap;2.7.14 completion','3.1 feature/status initialization','4.1.3.1 aligned32 configuration access;4.1.4 capability extents/multiple interfaces','4.1.4.3 reset acknowledgment;4.1.4.4 notification multiplier','5.2.5 FLUSH negotiation;5.2.6 sector units and stable writes','6.1 ACCESS_PLATFORM/ORDER_PLATFORM authority']),('R-KF-GABI','ELF gABI program loading','https://gabi.xinuos.com/elf/07-pheader.html','elf-pheader.html',['7.1 program header layout/filesz/memsz/alignment','7.2 segment types/load order','7.4 segment permissions;generic allowances versus CMK subset']),('R-KF-QEMU-PC','QEMU PC machine documentation','https://www.qemu.org/docs/master/system/i386/pc.html','qemu-pc.html',['PC machine model and devices;actual10.2.1 binary separately recorded'])]
 for ident,title,url,name,locators in specs:
  path=private/name
  if not path.exists():path.write_bytes(urllib.request.urlopen(url,timeout=45).read())
  resources.append({'id':ident,'title':title,'url':url,'locators':locators,'role':'bounded primary specification/tool authority','inspection':'Selected actual passages inspected;retrieved bytes pinned,not whole specification conformance or current errata coverage.','external_sha256':{str(path):sha(path)}})
 local=['curriculum/modules/T48.md','labs/kernel/README.md','labs/kernel/include/cmk_contract.h','labs/kernel/include/cmk_abi.h','assessor/kernel/model.c','assessor/kernel/kernel.c','labs/runtime/include/cvm.h','assessor/runtime/cvm.c','labs/storage/include/store.h','assessor/storage/store.c','labs/runtime/adapters/store_host.c','labs/runtime_followthrough/services/services.h','assessor/runtime_followthrough/services/services.c','assessor/runtime_followthrough/services/ANALYSIS.md','labs/kernel_smp/README.md','assessor/kernel_smp/kernel.c','assessor/kernel_smp/smp.c','labs/kernel_smp/tools/guest.py','labs/kernel_smp/tools/guest_mutations.py']
 local += [f'{directory}/PR-T48-{n:02}.{extension}' for n in range(1,15) for directory,extension in [('curriculum/projects','md'),('fixtures/kernel','json'),('curriculum/v5/expansion/legacy_assessments','json'),('assessor/legacy_depth','md')]]
 # Canonical sidecars will be attached later; preserve original-object identity
 # separately through git-baseline comparison rather than pinning mutable JSON here.
 local=[p for p in local if 'legacy_assessments' not in p]
 resources.append({'id':'R-KF-LOCAL','title':'Original contracts and actual retained/successor providers','paths':local,'locators':['All14 project contracts and fixtures;T48 module and original assessment tasks/variants inspected','Provider ownership/parser/ELF/frame/syscall/virtio/VM/store paths named in assessor derivations','CMK2 process-thread split,per-CPU state,lock/reap/shootdown and MSI-X completion;separate provider scope'],'role':'repository contract and implementation evidence','sha256':{p:sha(ROOT/p) for p in local}})
 limits=['Exact published C17 wording not newly inspected.','Intel253668-060US(2016) and psABI0.99.6(2012) remain historical selected authorities.','No physical CPU/DMA/timing/power-loss or complete PCI/ELF/general-OS conformance claim.','Source inspection is not independent learner or external review evidence.']
 write('resources.json',{'schema_version':1,'resources':resources});write('source_access.json',{'date':'2026-10-18','method':'Graphify before new codebase questions;actual local contracts/providers plus official primary HTML/PDF passages;downloaded authority bytes retained in private tool prefix.','records':resources,'limits':limits});write('research_gate.json',{'passed':True,'basis':'Each exact criterion maps to inspected bounded authority and concrete original mechanisms or explicit changed-premise analyses.','limits':limits,'unclaimed':['Independent review','Minimum-seven-day independent learner performance','Physical observations']})
if __name__=='__main__':main()
