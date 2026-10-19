"""Read-only exact T49 source,mechanism,negative-control and learner-export guard."""
import argparse,copy,hashlib,json,re,subprocess,sys,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];BASE=ROOT/'reviews/v5/expansion/legacy_os_followthrough';LAB=ROOT/'labs/os_followthrough';REF=ROOT/'assessor/os_followthrough';sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
from run_os_followthrough import sources
from labs.os_followthrough.observations import guest,locality
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def need(v,message):
 if not v:raise ValueError(message)
def read(path):
 p=(ROOT/path).resolve();need(p.is_relative_to(ROOT.resolve()),'Escaped path');return p.read_bytes()
def validate_campaign(v):
 need(v.get('passed') is True and v.get('error') is None,'Failed campaign');need(v['inspected_paths']=={p.relative_to(ROOT).as_posix():sha(p) for p in sources()},'Stale source closure');art=v['artifact_sha256'];need(art and all(hashlib.sha256(read(p)).hexdigest()==h for p,h in art.items()),'Stale artifact');rows={r['label']:r for r in v['commands']};need(len(rows)==len(v['commands'])==161,'Missing or duplicate commands');obs=v['observations']
 def text(label,code=0):
  r=rows[label];need(r['exit_code']==r['expected_exit_code']==code,'Wrong outcome '+label);need(r['output'] in art,'Unbound output');return read(r['output']).decode()
 def flags(label,wanted):need(all(flag in rows[label]['command'] for flag in wanted),'Wrong flags '+label)
 need('15.2' in text('gcc-version') and '21.1.8' in text('clang-version') and '3.26' in text('valgrind-version') and '6.19' in text('strace-version') and '10.2.1' in text('qemu-aarch64-version'),'Tool identity changed')
 certificates=json.loads(text('certificates'));need(certificates==obs['certificates'] and certificates['passed'],'Premises unbound');c=certificates['records'];need(c['ownership']['reference_sequence']==[2,3,2,1,0] and c['ownership']['reclaim_at_step']==5,'Ownership certificate');need(c['working-sets']['unique']==[0,1,3] and c['working-sets']['RSS_from_page_ids'] is None and c['working-sets']['grouped_windows']!=c['working-sets']['interleaved_windows'],'Page scope inflated');need(c['EEVDF-negative-lag']['correct']==1 and c['EEVDF-negative-lag']['reset_negative_lag_wrong']==0 and not c['EEVDF-negative-lag']['kernel_equivalence'],'Policy equivalence invented');need(c['quota-boundary']['five_aligned_periods']==100000 and c['quota-boundary']['five_period_duration_midperiod']==120000 and c['quota-boundary']['latency_guarantee'] is None,'Quota boundary changed');need(c['byte-ledgers']['tail_lost'] is None and c['byte-ledgers']['acknowledged_prefix']==6144,'Durability invented')
 for cc in ('gcc','clang'):
  for profile in ('O0','O2'):
   prefix=cc+'-'+profile
   for name in ('core','syscalls','lifecycle','descriptors','readiness','completion','io','locality'):
    text(prefix+'-'+name+'-build');flags(prefix+'-'+name+'-build',['-std=c17','-Werror','-'+profile]);need(('-DNDEBUG' in rows[prefix+'-'+name+'-build']['command'])==(profile=='O2'),'NDEBUG profile changed');raw=text(prefix+'-'+name);need(('OF_PASS:'+name in raw) if name!='locality' else json.loads(raw)['passed'],'Uncompleted host mechanism')
   raw=text(prefix+'-core');need(all(marker in raw for marker in ['SRTF-B3-C4-A11','refusal-preserves','scale28=2,4,8:normalization:yield-mutant','FIFO9-10:CLOCK-provider','done0,2,2,4,4:EINTR-EAGAIN-terminal']),'Missing algorithm mechanism')
   raw=text(prefix+'-syscalls');need('raw-error-9:wrapper-1-errno9:bad-pointer-14' in raw and raw.count('requested8192:accepted4096:remaining4096:EAGAIN')==2,'Raw or short write absent');need('syscall' in text(prefix+'-syscall-assembly'),'Actual x86 instruction absent');trace=text(prefix+'-syscall-trace');need('EAGAIN' in trace and re.search(r'write\([^\n]*8192\)\s*=\s*4096',trace),'Real short syscall absent')
   raw=text(prefix+'-lifecycle');need(all(m in raw for m in ['made1:joined1:count1000','made2:joined2:count2000','channel1:status0:reaped','status7:reaped','pre-fork-flush0:bytes12','pre-fork-flush1:bytes6']),'Lifecycle branch absent')
   raw=text(prefix+'-descriptors');need(all(m in raw for m in ['offsets5,5,4:fork6:SCM_RIGHTS7','inherited-writer-blocks','CLOEXEC0:private-endpoint-absent0','CLOEXEC1:private-endpoint-absent1','stage5:requested5','stage5:requested6']),'Descriptor branch absent')
   need('stolen-ready-EAGAIN:4+4-record:partial2-EOF-reject:atomic-unmask-EINTR:actual-selfpipe' in text(prefix+'-readiness'),'Readiness conflated');need('gen4-only7:cancel-retains:pending-active-submit-gates:out-of-order:shutdown-quiescent:max-retired' in text(prefix+'-completion'),'Completion lifetime incomplete')
   raw=text(prefix+'-io');need('writes7168:reads2048:ack6144:uncertain-tail1024:short113:EINTR:EAGAIN' in raw and 'retain-after-EINTR0' in raw and 'retain-after-EINTR1' in raw and 'overlap-app8192-file7168' in raw,'IO or close model absent')
   a=json.loads(text(prefix+'-locality'));b=json.loads(text(prefix+'-competing-reader'));summary=locality(a,b);need(summary==json.loads(text(prefix+'-locality-summary'))==obs[prefix+'-locality'],'Locality summary not raw-derived');need('of_touch' in text(prefix+'-touch-assembly'),'No actual retained touch code')
  oracle=json.loads(text(cc+'-oracle'));need(oracle==obs[cc+'-oracle'] and oracle['passed'] and oracle['counts']=={'schedules':19680,'pages':39360,'LRU_prefix_inclusions':42648,'stride_independent':128},'Independent oracle missing');w=oracle['witnesses'];need(w['miss-vs-cost']['FIFO_faults_writebacks']==[9,2] and w['miss-vs-cost']['OPT_faults_writebacks']==[7,3] and w['miss-vs-cost']['cost_optimum']==17,'Cost objective confused');need(len(w['supplied-page-traces'])==8 and all(len(t['steps'])==12 for t in w['supplied-page-traces']),'Missing page transitions');need(w['fairness']['yield']['timeline']!=w['fairness']['yield-reset-mutant']['timeline'],'Yield mutant not discriminated')
  for kind,code,marker in [('fixed',0,'OF_PASS:mutant-witnesses'),('tie',7,'OF_KILLED:tie-selection'),('idle',8,'OF_KILLED:idle-as-wait')]:need(marker in text(cc+'-mutant-'+kind,code),'Mutant survives');text(cc+'-mutant-'+kind+'-build')
  for name in ('core','readiness','completion','io'):flags(cc+'-sanitized-'+name+'-build',['-fsanitize=address,undefined']);need('OF_PASS:' in text(cc+'-sanitized-'+name),'Sanitized mechanism failed')
  for name in ('core','lifecycle','descriptors'):need('ERROR SUMMARY: 0 errors' in text(cc+'-memcheck-'+name),'Memcheck errors')
  for name in ('core','retained','bytes'):flags(cc+'-'+name+'-freestanding',['-std=c17','-ffreestanding','-fno-builtin']);text(cc+'-'+name+'-freestanding')
  need(not text(cc+'-undefined').strip(),'Original core runtime closure open');need('INTERP' not in text(cc+'-guest-headers'),'Guest probe not static');flags(cc+'-guest-build',['-static','-std=c17']);raw=text(cc+'-guest-run');derived=json.loads(json.dumps(guest(raw)));need(derived==json.loads(text(cc+'-guest-observations'))==obs[cc+'-guest'],'Guest counters/claims not raw-derived');need(derived['physical_durability'] is False and derived['host_policy_changed'] is False,'Guest scope inflated')
  work=Path(rows[cc+'-guest-run']['command'][rows[cc+'-guest-run']['command'].index('--work')+1]);record=json.loads((work/'command.json').read_text());command=record['command'];need('-nic' in command and command[command.index('-nic')+1]=='none' and command[command.index('-accel')+1]=='tcg' and command[command.index('-m')+1]=='768M','Guest containment changed');disk=work/'owned-ext4.raw';need(disk.stat().st_size==64*1024*1024 and str(disk) in record['format'],'Unbounded or wrong disk');need(all(str(p).startswith(str(work)) for p in [disk,work/'owned.cpio']),'Guest artifact scope')
 for profile in ('O0','O2'):
  label='arm64-'+profile;flags(label+'-build',['--target=aarch64-linux-gnu','-std=c17','-nostdlib','-static','-'+profile]);need('OF:aarch64:write64:x0-x2:x8:negative-error' in text(label),'Second ABI unexecuted');need('svc' in text(label+'-assembly') and not text(label+'-undefined').strip() and 'INTERP' not in text(label+'-headers'),'Second ABI closure/instructions missing')
 need('OF_CHECK:' in text('stub',1),'Learner stub solved');need(len(v['checks'])==79 and all(c['passed'] is True for c in v['checks']),'Mechanism checks incomplete');need(len(v['open_gates'])==5 and 'seven-day' in v['open_gates'][1] and 'Physical' in v['open_gates'][2] and 'POSIX2024' in v['open_gates'][3] and 'cold-cache' in v['open_gates'][4],'External authority inflated')
 provision=v['private_provision'];need(provision['passed'] and all(sha(Path(p))==h for p,h in provision['sha256'].items()),'Private tool/kernel identity stale');need(provision['source_kernel_unchanged_config']==provision['sha256']['/home/frede/cmastery-b06-tools/debug-kernel/linux-source-7.0.0/.config'],'Original kernel changed')
 resources=json.loads((ROOT/'research/v5/expansion/legacy_os_followthrough/resources.json').read_text())['resources']
 for resource in resources:
  for path,digest in resource.get('external_sha256',{}).items():need(sha(Path(path))==digest,'Inspected external source changed')
 older=(ROOT/'research/v5/expansion/legacy_os_followthrough/inspected/linux-6.6-fair.c').read_text();newer=Path('/home/frede/cmastery-os-follow-tools/linux-source-7.0.0/kernel/sched/fair.c').read_text();need('DELAY_DEQUEUE' not in older and 'RUN_TO_PARITY' in older and 'DELAY_DEQUEUE' in newer and 'protect_slice' in newer,'Selected release comparison changed')
 from research.v5.expansion.legacy_os_followthrough.criterion_notes import NOTES
 need(len(NOTES)==30 and all(selector in rows for _,_,selectors in NOTES.values() for selector in selectors),'Exact criterion selector absent');return True
def main(report):
 from research.v5.expansion.legacy_os_followthrough.author_support import attach,SUPPORT,PROJECTS,public_paths
 from cmastery import assessments
 v=json.loads((BASE/'campaign.json').read_text());validate_campaign(v);checks=[{'name':'source-bound-exact-mechanisms','passed':True}]
 def check(name,value):checks.append({'name':name,'passed':bool(value)})
 controls=[('missing-command',lambda x:x['commands'].pop()),('stale-source',lambda x:x['inspected_paths'].update({'labs/os_followthrough/os.h':'0'*64})),('stale-artifact',lambda x:x['artifact_sha256'].update({next(iter(x['artifact_sha256'])):'0'*64})),('fake-stub-pass',lambda x:next(r for r in x['commands'] if r['label']=='stub').update(exit_code=0)),('wrong-baseline',lambda x:next(r for r in x['commands'] if r['label']=='gcc-O2-core-build')['command'].remove('-std=c17')),('invented-guest-durability',lambda x:x['observations']['gcc-guest'].update(physical_durability=True)),('removed-competitor',lambda x:x['observations']['gcc-O2-locality'].update(competing_reader_bytes=0)),('fake-independent-oracle',lambda x:x['observations']['gcc-oracle']['counts'].update(schedules=1)),('external-completed',lambda x:x.update(open_gates=[]))]
 for name,mutate in controls:
  changed=copy.deepcopy(v);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,AssertionError):rejected=True
  check(name,rejected)
 for pid in PROJECTS:
  path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(pid+'.json');r=json.loads(path.read_text());old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True));criteria=copy.deepcopy(r['criteria'])
  for c in criteria:c.pop('successor_support',None)
  check(pid+'-original-criteria',criteria==old['criteria']);check(pid+'-historical-contracts',all(r[k]==old[k] for k in ('baseline_binding','execution_support','status')));check(pid+'-idempotent',attach(copy.deepcopy(r))==r)
  with tempfile.TemporaryDirectory(prefix='os-follow-export-') as directory:
   work=Path(directory);assessments.export(ROOT,pid,work);check(pid+'-public-export',all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in public_paths()) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*')))
   if pid in ('PR-T49-01','PR-T49-10'):
    for reference in (False,True):
     exe=work/('core-'+str(reference));cmd=[sys.executable,work/'labs/os_followthrough/build.py','--output',exe]
     if reference:cmd+=['--os-source',REF/'os.c','--base-source',ROOT/'assessor/os_observatory/oslab.c']
     build=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60);okay=False
     if build.returncode==0:
      result=subprocess.run([exe],capture_output=True,text=True,timeout=10);okay=result.returncode==(0 if reference else 1) and ('OF_PASS:' if reference else 'OF_CHECK:') in result.stdout+result.stderr
     check(pid+('-explicit-reference' if reference else '-incomplete-stub'),okay)
 support=json.loads((ROOT/SUPPORT).read_text());original=[c for pid in PROJECTS for c in json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(pid+'.json')).read_text())['criteria']];expected={c['id'] for c in original}|{s['id'] for c in original for s in c['variants']};check('60-exact-identities',len(support['updates'])==60 and {r['id'] for r in support['updates']}==expected);check('support-source-pins',all(hashlib.sha256(read(p)).hexdigest()==h for p,h in support['source_sha256'].items()));check('external-authority-separate',all(r['dimensions']['external_review']['state']=='not-run' and r['dimensions']['independent_learner']['state']=='not-assessed' and r['dimensions']['physical_observation']['state'].startswith('not-run') for r in support['updates']));mapping=json.loads((LAB/'criteria.json').read_text())['criteria'];check('exact-task-and-variant-preservation',all(next(r for r in mapping if r['id']==c['id'])['original_task']==c['task'] and next(r for r in mapping if r['id']==c['id'])['changed_conditions']==c['variants'] for c in original))
 result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():sha(p) for p in (Path(__file__).resolve(),BASE/'campaign.json',ROOT/SUPPORT)},'scope':'Exact30 original criteria+30 delayed variants;host/owned-guest/model,author and independent/physical/source authority separate.'};dest=(ROOT/report).resolve();need(dest.is_relative_to(BASE.resolve()),'Report escapes');dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[c['name'] for c in checks if not c['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
