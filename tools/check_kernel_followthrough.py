"""Read-only exact-source,semantic and exported-package guard for78 T48 IDs."""
import argparse,copy,hashlib,json,re,struct,subprocess,sys,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
from freeze_kernel_followthrough import OPEN
from audit_kernel_runtime_stack import audit
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def need(value,message):
 if not value:raise ValueError(message)
def load(path):return json.loads((ROOT/path).read_text())
def raw(row):return (ROOT/row['output']).read_text(errors='replace')
def validate_host(v,commands,checks,files):
 need(v['passed'] and v['error'] is None,'Failed hosted campaign');need(len(v['commands'])==commands and len(v['checks'])==checks and all(x['passed'] for x in v['checks']),'Missing hosted mechanism');need(all(x['exit_code']==x['expected_exit_code'] for x in v['commands']),'Hosted exit mismatch')
 if files:
  need(all(sha(ROOT/p)==h for p,h in v['inspected_paths'].items()),'Stale hosted source');need(all(sha(ROOT/p)==h for p,h in v['artifact_sha256'].items()),'Stale hosted output')
 rows={r['label']:r for r in v['commands']};need('15.2' in raw(rows['gcc-version']) and '21.1.8' in raw(rows['clang-version']),'Wrong compiler identity');need(rows['stub']['exit_code']==1 and 'CHECK:' in raw(rows['stub']),'Stub passed')
 return rows
def validate_guest(v,files=True):
 need(v['passed'] and v['error'] is None,'Failed actual guest');case=v['case'];expected=35 if case in (2,3,4,13,14,23,24,29,30) else 33
 rows={r['label']:r for r in v['commands']};need(len(rows)==len(v['commands']),'Repeated guest command');need(rows['guest']['exit_code']==rows['guest']['expected_exit_code']==expected,'Wrong guest exit or timeout');need(all(r['exit_code']==r['expected_exit_code'] for r in v['commands']),'Failed guest command')
 if files:need(all(sha(ROOT/p)==h for p,h in v['artifact_sha256'].items()),'Stale image/log/object')
 out=raw(rows['guest']);need('KF:SUCCESSOR_BOOT' in out,'Missing current guest boot')
 if expected==33:need('KF:PASS:GUEST_SUITE' in out and (case in (20,21) or out.count('KF:PASS:WORKER_GPRS_PRIVATE_MEMORY')==2),'Missing actual guest mechanism')
 for name,row in rows.items():
  if name.endswith('-undefined'):need(not raw(row).strip(),'Hosted dependency in guest')
  if name.endswith('-build') and name!='vm-host-build':need(all(x in row['command'] for x in ('-std=c17','-ffreestanding','-fno-builtin','-mno-red-zone','-mgeneral-regs-only')),'Wrong target profile')
 command=rows['guest']['command'];need('pc-i440fx-10.2' in command and 'tcg' in command and command[command.index('-smp')+1]=='1' and '-net' in command and 'none'==command[command.index('-net')+1],'Wrong containment/profile')
 need(not any(x in command for x in ('-enable-kvm','-nic','-netdev')),'Unbound network/acceleration')
 folder=(ROOT/rows['guest']['output']).parent
 if case==3:
  records=re.findall(r'KF:FIRST:vector=(\S+) error=(\S+) rip=(\S+) cs=(\S+) pid=(\S+).*nested=(\S+)',out);need(len(records)==2 and records[0][:5]==records[1][:5] and int(records[1][5],16)==1,'Lost first-fault record')
 markers={4:'DOUBLE_FAULT_IST_OWNED_STACK',5:'DIAG_TRUNCATED_FIRST_RETAINED',13:'KF:FAIL:SAVED_REGISTER',14:'FAIL kernel: isolation/preemption evidence missing',23:'KF:ELF_REJECT:',24:'KF:FAIL:USER',28:'ARCHIVE_TRUNCATION_BEFORE_VM_RUN',29:'baseline verifier exceeds one-page user stack',30:'unsupported changed sector-size profile'}
 if case in markers:need(markers[case] in out,'Missing causal negative/variant marker')
 if case==19:
  data=(folder/'init.cmka').read_bytes();count=struct.unpack_from('<I',data,8)[0]
  for i in range(count):off,n=struct.unpack_from('<II',data,16+i*40+24);elf=data[off:off+n];need(elf[:4]==b'\x7fELF' and struct.unpack_from('<Q',elf,40)[0]==0 and struct.unpack_from('<HHH',elf,58)==(0,0,0),'Section headers not removed')
 if case==22:need(out.count('KF:ALLOC_ROLLBACK:')==16,'Missing allocation prefixes')
 if case in (26,27):need([s for s in out.splitlines() if s.startswith('KF_VM')]==raw(rows['vm-host']).splitlines(),'Host/guest semantic mismatch')
 if case in (26,27,29):need(audit(folder)['fits_one_page']==(case==27),'Native call-chain budget failed')
 if case in (31,32,33,34,35):
  need(rows['second-guest']['exit_code']==33,'Missing fresh second boot');second=raw(rows['second-guest']);need('KF:PASS:GUEST_SUITE' in second,'No second-guest completion');firstcmd=rows['guest']['command'];secondcmd=rows['second-guest']['command'];need(next(x for x in firstcmd if str(x).startswith('if=none'))==next(x for x in secondcmd if str(x).startswith('if=none')),'Different persistence medium')
  if case in (31,32):
   data=(folder/'disk.raw').read_bytes();need(len(data)==4*1024*1024 and 'FRESH_GUEST_ALL_12288_PATTERN_BYTES' in second,'Missing full persistence comparison')
   for page in (0,82,1023):need(data[page*4096:(page+1)*4096]==bytes((page*17+i*13+7)&255 for i in range(4096)),'Pattern byte mismatch')
  else:
   data=(folder/'disk.raw').read_bytes();need(not any(data[:16*4096]) and not any(data[83*4096:]),'Store escaped slice');need('CVM_SERVICE_STORE_PAGE_SYSCALL_VIRTIO_CHAIN' in out and 'CVM_SERVICE_STORE_PAGE_SYSCALL_VIRTIO_CHAIN' in second,'Missing real provider chain');need(('DURABLE_QUERY_41_BEFORE_RETRY' if case==35 else 'DURABLE_QUERY_42_AFTER_LOST_REPLY') in second,'Uncertainty not distinguished')
 return True
def validate_campaign(v,files=True):
 need(v['passed'] and v['error'] is None,'Failed composed campaign');need(v['open_gates']==OPEN,'Inflated external claims');need(v['allowed_guest_exit_codes']==[33,35],'Timeout treated as success');need(v['coverage']=={'original_criteria':39,'original_delayed_variants':39,'guest_profiles':87,'runtime_profiles':12,'smp_cpu_counts':[1,2,4],'host_contract_checks':102,'host_device_checks':79},'Incomplete scope')
 need(set(v['components'])=={'contracts','device','matrix','runtime','smp','services'},'Missing distinct mechanism campaign');need(all(sha(ROOT/p)==h for p,h in v['inspected_paths'].items()),'Stale source closure')
 data={}
 for name,row in v['components'].items():need(sha(ROOT/row['path'])==row['sha256'],'Changed component receipt');data[name]=load(row['path'])
 contracts=validate_host(data['contracts'],134,102,files);devices=validate_host(data['device'],110,79,files)
 for cc in ('gcc','clang'):
  for group in ('tags','reservations','frames','allocation','lifecycle'):need('KF_PASS:'+group in raw(contracts[cc+'-O2-'+group]),'Missing exact host contract')
  for group in ('permissions','scheduling','images','archives','handles','lines'):need('KF_PASS:'+group in raw(contracts[cc+'-boundaries-O2-'+group]),'Missing exact boundary')
  for group in ('pci','queue','geometry','ordering','durability'):need('KF_DEVICE_PASS:'+group in raw(devices[cc+'-O2-'+group]),'Missing exact device model')
 m=data['matrix'];need(m['passed'] and m['error'] is None and len(m['guests'])==87 and len(m['checks'])==375 and all(x['passed'] for x in m['checks']),'Incomplete87-profile matrix')
 plans={(cc,'O2',case,11932,0) for cc in ('gcc','clang') for case in range(36)}|{(cc,'O0',case,11932,0) for cc in ('gcc','clang') for case in (0,16,26,27,29,34)}|{('gcc','O2',15,q,d) for q,d in ((5966,0),(11932,2000000),(5966,2000000))}
 need({(x['compiler'],x['optimization'],x['case'],x['quantum'],x['delay']) for x in m['guests']}==plans,'Missing changed condition/profile')
 for row in m['guests']:
  need(sha(ROOT/row['receipt'])==row['sha256'],'Changed guest receipt');validate_guest(load(row['receipt']),files)
  if 'stack_audit' in row:need(sha(ROOT/row['stack_audit'])==row['stack_audit_sha256'],'Changed native path audit')
 from run_kernel_runtime_followthrough import validate as validate_runtime
 if files:validate_runtime(data['runtime'])
 s=data['smp'];need(s['passed'] and s['error'] is None and [x['cpus'] for x in s['runs']]==[1,2,4],'Missing actual SMP transfer')
 if files:need(bool(s['artifact_sha256']) and all(sha(ROOT/p)==h for p,h in s['artifact_sha256'].items()),'Changed SMP image/log/negative control')
 for row in s['runs']:
  need(sha(ROOT/row['receipt'])==row['sha256'],'Changed SMP receipt');r=load(row['receipt']);need(r['passed'] and len(r['runs'])==3,'Incomplete SMP modes')
  if files:
   for run in r['runs']:
    out=(ROOT/row['receipt']).parent/(run['mode']+'.log');need(run['exit']==33 and all(x in out.read_text() for x in run['expected_markers']),'Missing actual SMP mechanisms')
 mutation=load(str(Path(v['components']['smp']['path']).parent/'mutations/mutations.json'));need(mutation['passed'] and {x['mutation'] for x in mutation['records'] if x['detected']}=={'tlb_ack','msix_mask'},'Missing SMP causal controls')
 control=mutation['positive_control'];need(control['detected'] and control['exit']==0 and control['wait_entered'] and control['wait_completed'],'Missing paired actual TLB wait control')
 mutant=next(x for x in mutation['records'] if x['mutation']=='tlb_ack');need(mutant['wait_entered'] and not mutant['wait_completed'] and mutant['exit']!=0,'TLB deadline before causal precondition')
 from check_vm_services import validate as validate_services
 need(not validate_services(data['services'],files),'Stale/partial imported service cuts/receipts')
 resources=load('research/v5/expansion/legacy_kernel_followthrough/resources.json')
 if files:
  for row in resources['resources']:
   need(all(sha(Path(p))==h for p,h in row.get('external_sha256',{}).items()),'Changed primary source bytes');need(all(sha(ROOT/p)==h for p,h in row.get('sha256',{}).items()),'Changed inspected local authority')
 return True
def main(report):
 from research.v5.expansion.legacy_kernel_followthrough.author_support import attach,SUPPORT,CAMPAIGN,public_paths,records
 from cmastery import assessments
 v=load(CAMPAIGN);validate_campaign(v);checks=[{'name':'all-distinct-source-bound-mechanisms','passed':True}]
 def check(name,value):checks.append({'name':name,'passed':bool(value)})
 controls=[('stale-source',lambda x:x['inspected_paths'].update({'tools/run_kernel_guest_followthrough.py':'0'*64})),('wrong-image-campaign',lambda x:x['components']['matrix'].update(sha256='0'*64)),('missing-device-campaign',lambda x:x['components'].pop('device')),('missing-guest-profile',lambda x:x['coverage'].update(guest_profiles=86)),('timeout-success',lambda x:x.update(allowed_guest_exit_codes=[33,35,'timeout'])),('unearned-external-completion',lambda x:x.update(open_gates=[])),('failed-campaign',lambda x:x.update(passed=False))]
 for name,mutate in controls:
  changed=copy.deepcopy(v);mutate(changed)
  try:validate_campaign(changed,False);rejected=False
  except (ValueError,KeyError,AssertionError):rejected=True
  check(name,rejected)
 first=load(load(v['components']['matrix']['path'])['guests'][0]['receipt'])
 for name,mutate in [('wrong-actual-exit',lambda x:next(r for r in x['commands'] if r['label']=='guest').update(exit_code=0,expected_exit_code=0)),('stale-image',lambda x:x['artifact_sha256'].update({next(p for p in x['artifact_sha256'] if p.endswith('/guest.iso')):'0'*64}))]:
  changed=copy.deepcopy(first);mutate(changed)
  try:validate_guest(changed);rejected=False
  except (ValueError,KeyError,AssertionError):rejected=True
  check(name,rejected)
 support=load(SUPPORT);mapping=load('labs/kernel_followthrough/criteria.json')['criteria'];expected=set()
 index=load(support['evidence_index'])['selectors'];check('all-evidence-selectors-resolve',all(s in index and index[s]['records'] for row in mapping for s in row['mechanism_checks']))
 for r in records():
  path='curriculum/v5/expansion/legacy_assessments/'+r['project']+'.json';old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path],cwd=ROOT,text=True));criteria=copy.deepcopy(r['criteria'])
  for c in criteria:c.pop('successor_support',None)
  check(r['project']+'-original-criteria',criteria==old['criteria']);check(r['project']+'-historical-bindings',all(r[k]==old[k] for k in ('baseline_binding','execution_support','status')));check(r['project']+'-idempotent',attach(copy.deepcopy(r))==r)
  for c in r['criteria']:
   expected.add(c['id']);expected.update(x['id'] for x in c['variants']);row=next(x for x in mapping if x['id']==c['id']);check(c['id']+'-exact-task-variant',row['original_task']==c['task'] and row['original_expected']==c['verification']['expected'] and row['changed_conditions']==c['variants'])
 check('78-exact-identities',len(support['updates'])==78 and {x['id'] for x in support['updates']}==expected);check('support-source-pins',all(sha(ROOT/p)==h for p,h in support['source_sha256'].items()));check('independent-gates-unclaimed',all(x['dimensions']['external_review']['state']=='not-run' and x['dimensions']['independent_learner']['state']=='not-assessed' and x['dimensions']['physical_observation']['state'].startswith('not-run') for x in support['updates']));check('no-declared-author-work-left',support['remaining_author_work']==[])
 with tempfile.TemporaryDirectory(prefix='kernel-follow-export-') as temp:
  work=Path(temp);exports={}
  for r in records():
   target=work/r['project'];assessments.export(ROOT,r['project'],target);exports[r['project']]=target;check(r['project']+'-public-export',all((target/p).read_bytes()==(ROOT/p).read_bytes() for p in public_paths()) and not any('assessor' in p.relative_to(target).parts for p in target.rglob('*')))
  def execute(name,cmd,expected,marker=None):
   p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=90);check(name,p.returncode==expected and (marker is None or marker in p.stdout+p.stderr));return p
  for cc in ('gcc','clang'):
   for kind,project in (('contracts','PR-T48-02'),('boundaries','PR-T48-07'),('device','PR-T48-09')):
    directory=exports[project];runner=directory/'labs/kernel_followthrough/run.py';common=[sys.executable,runner,kind,'--cc',cc]
    execute(cc+'-'+kind+'-learner-stub',[*common,'--build',work/(cc+kind+'stub')],1,'KF_DEVICE_CHECK:' if kind=='device' else 'KF_CHECK:')
    command=[*common,'--build',work/(cc+kind+'reference'),'--source',ROOT/('assessor/kernel_followthrough/device/device.c' if kind=='device' else 'assessor/kernel_followthrough/contracts/contracts.c')]
    if kind!='device':command+=['--model',ROOT/'assessor/kernel/model.c']
    execute(cc+'-'+kind+'-explicit-reference',command,0,'KF_DEVICE_PASS:' if kind=='device' else 'KF_PASS:')
   runner=exports['PR-T48-12']/'labs/kernel_followthrough/run.py';execute(cc+'-runtime-missing-provider',[sys.executable,runner,'runtime','--cc',cc,'--build',work/(cc+'runtimeabsent')],2,'requires --cvm');execute(cc+'-runtime-explicit-provider',[sys.executable,runner,'runtime','--cc',cc,'--cvm',ROOT/'assessor/runtime/cvm.c','--build',work/(cc+'runtime')],0,'KF_VM:5:0:42:1:256:29:0')
   builder=exports['PR-T48-03']/'labs/kernel_followthrough/build_guest.py';execute(cc+'-public-guest-incomplete-build',[sys.executable,builder,'--cc',cc,'--build',work/(cc+'gueststub')],0)
   arguments=[]
   for flag,path in [('kernel','assessor/kernel_followthrough/guest/kernel.c'),('boot','assessor/kernel_followthrough/boot/boot.S'),('interrupts','assessor/kernel_followthrough/guest/interrupts.S'),('model','assessor/kernel/model.c'),('contracts','assessor/kernel_followthrough/contracts/contracts.c'),('device','assessor/kernel_followthrough/device/device.c')]:arguments+=['--'+flag,ROOT/path]
   p=execute(cc+'-public-guest-explicit-reference-run',[sys.executable,builder,'--cc',cc,'--build',work/(cc+'guestref'),'--run',*arguments],0);check(cc+'-public-actual-guest-log',(work/(cc+'guestref')/'guest.txt').exists() and 'KF:PASS:GUEST_SUITE' in (work/(cc+'guestref')/'guest.txt').read_text())
 result={'passed':all(x['passed'] for x in checks),'checks':checks,'inspected_paths':{p:sha(ROOT/p) for p in (CAMPAIGN,SUPPORT,'tools/check_kernel_followthrough.py')},'scope':'78 exact T48.02–14 identities;original criteria preserved;actual mechanisms and public isolated exports;independent and physical gates remain open.'};dest=(ROOT/report).resolve();need(dest.is_relative_to(ROOT/'reviews/v5/expansion/legacy_kernel_followthrough'),'Report escapes');dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[x['name'] for x in checks if not x['passed']]}));return int(not result['passed'])
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
