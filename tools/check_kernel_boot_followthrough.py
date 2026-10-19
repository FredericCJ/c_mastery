"""Source-bound read-only guard for six exact T48.01 identities."""
import argparse,copy,hashlib,json,re,struct,subprocess,sys,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
from run_kernel_boot_followthrough import sources
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def need(v,message):
 if not v:raise ValueError(message)
def validate_campaign(v):
 need(v['passed'] is True and v['error'] is None,'Failed campaign');need(v['inspected_paths']=={p.relative_to(ROOT).as_posix():sha(p) for p in sources()},'Stale sources');need(all(sha(ROOT/p)==h for p,h in v['artifact_sha256'].items()),'Stale artifacts');rows={x['label']:x for x in v['commands']};need(len(rows)==len(v['commands'])==299,'Missing commands');need(len(v['checks'])==149 and all(x['passed'] for x in v['checks']),'Missing mechanism checks');need(len(v['images'])==26,'Missing actual images')
 def raw(label,code=0):
  r=rows[label];need(code is None or r['returncode']==code,'Wrong exit '+label);p=Path(r['output']);need(p.is_relative_to(ROOT) and p.relative_to(ROOT).as_posix() in v['artifact_sha256'],'Unbound output');return p.read_text(errors='replace')
 need('15.2' in raw('gcc-version') and '21.1.8' in raw('clang-version') and '10.2.1' in raw('qemu-system-x86_64-version'),'Tool identity changed')
 for cc in ('gcc','clang'):
  for opt in ('O0','O2'):
   for kind in ('correct','relocated','reordered','omit-info','misaligned','omit-far'):
    label=f'{cc}-{opt}-{kind}';image=next(x for x in v['images'] if x['label']==label);folder=Path(rows[label+'-link']['command'][rows[label+'-link']['command'].index('-o')+1]).parent;elf=folder/'kernel.elf';need(sha(elf)==image['elf_sha256'] and sha(folder/'boot.iso')==image['iso_sha256'],'Image identity');blob=elf.read_bytes();offset=blob.find(struct.pack('<I',0xe85250d6));need(offset==image['header_file_offset'] and 0<=offset<32768 and offset%8==0 and sum(struct.unpack_from('<IIII',blob,offset))%2**32==0,'Header location/checksum');need(not raw(label+'-undefined').strip(),'Runtime dependency');raw(label+'-elf');raw(label+'-disassembly')
    for part in ('boot','entry','probe'):
     cmd=rows[label+'-'+part]['command'];need(all(x in cmd for x in ('-std=c17','-'+opt,'-ffreestanding','-fno-builtin','-mno-red-zone','-mgeneral-regs-only')),'Missing target flags');raw(label+'-'+part)
    text=raw(label+'-guest',35 if kind in ('omit-info','misaligned') else None if kind=='omit-far' else 33)
    if kind in ('correct','relocated','reordered'):
     need('KF:PASS:2048_PAGES_4GIB_SUPERVISOR' in text and 'KF:PASS:BOOT_ONLY_C17' in text,'No actual boot success');rsp=int(re.search(r'entry_rsp=([0-9a-f]+)',text)[1],16);need(rsp%16==8,'Wrong C entry alignment')
    elif kind=='omit-info':need('info=0000000000000000' in text and 'KF:FAIL:BOOT_ARGS' in text,'Missing saved-argument refusal')
    elif kind=='misaligned':need('KF:FAIL:STACK_ALIGNMENT' in text,'Missing alignment refusal')
    else:need('KF:PASS:BOOT_ONLY_C17' not in text,'Invalid mode passed')
    if kind=='relocated':need(image['symbols']['_start']>=0x600000 and image['symbols']['kernel_end']<0x100000000 and image['symbols']['boot_stack_top']>=0x600000,'Relocation obligation lost')
    if opt=='O2' and kind in ('correct','omit-info','omit-far'):
     state=raw(label+'-gdb',None);need(rows[label+'-gdb']['returncode']==0 or (rows[label+'-gdb']['returncode']==1 and 'Terminated via GDBstub' in state),'Unexplained GDB error');m=re.search(r'KF_GDB_START eax=36d76289 ebx=([0-9a-f]+)',state);need(m and int(m[1],16)!=0,'No GRUB argument evidence');need(('KF_GDB_AFTER cs=10 ' in state and 'CS32' in state) if kind=='omit-far' else ('KF_GDB_AFTER cs=8 ' in state and 'CS64' in state),'Wrong far-transfer evidence')
 need('KF:FAIL:STACK_ALIGNMENT' in raw('gcc-O2-baseline-guest',35),'Historical defect unobserved');need('KF:PASS' not in raw('gcc-O2-stub-guest','timeout'),'Learner stub completed');need(v['open_gates']==['Independent external review not run.','Minimum-seven-day independent learner assessment not observed.','Physical processor and current full errata observations not run.'],'External authority inflated')
 for r in json.loads((ROOT/'research/v5/expansion/legacy_kernel_boot/resources.json').read_text())['resources']:
  need(all(sha(Path(p))==h for p,h in r['external_sha256'].items()),'Inspected primary source changed')
 return True
def main(report):
 from research.v5.expansion.legacy_kernel_boot.author_support import attach,SUPPORT,CAMPAIGN,public_paths
 from cmastery import assessments
 v=json.loads((ROOT/CAMPAIGN).read_text());validate_campaign(v);checks=[{'name':'exact-source-bound-boot-mechanisms','passed':True}]
 def check(name,value):checks.append({'name':name,'passed':bool(value)})
 for name,mutate in [('stale-source',lambda x:x['inspected_paths'].update({'tools/run_kernel_boot_followthrough.py':'0'*64})),('missing-command',lambda x:x['commands'].pop()),('fake-stub-pass',lambda x:next(r for r in x['commands'] if r['label']=='gcc-O2-stub-guest').update(returncode=0)),('wrong-image',lambda x:x['images'][0].update(elf_sha256='0'*64)),('missing-relocation',lambda x:next(r for r in x['images'] if r['label']=='gcc-O2-relocated')['symbols'].update(_start=0x101000)),('external-completed',lambda x:x.update(open_gates=[]))]:
  changed=copy.deepcopy(v);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,AssertionError):rejected=True
  check(name,rejected)
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T48-01.json';r=json.loads(path.read_text());old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True));criteria=copy.deepcopy(r['criteria'])
 for c in criteria:c.pop('successor_support',None)
 check('original-criteria',criteria==old['criteria']);check('historical-contracts',all(r[k]==old[k] for k in ('baseline_binding','execution_support','status')));check('idempotent-attachment',attach(copy.deepcopy(r))==r)
 support=json.loads((ROOT/SUPPORT).read_text());expected={c['id'] for c in r['criteria']}|{v['id'] for c in r['criteria'] for v in c['variants']};check('six-exact-identities',len(support['updates'])==6 and {x['id'] for x in support['updates']}==expected);check('support-source-pins',all(sha(ROOT/p)==h for p,h in support['source_sha256'].items()));mapping=json.loads((ROOT/'labs/kernel_followthrough/boot/criteria.json').read_text())['criteria'];check('original-task-and-variants',all(next(x for x in mapping if x['id']==c['id'])['original_task']==c['task'] and next(x for x in mapping if x['id']==c['id'])['changed_conditions']==c['variants'] for c in r['criteria']));check('independent-gates-unclaimed',all(x['dimensions']['external_review']['state']=='not-run' and x['dimensions']['independent_learner']['state']=='not-assessed' and x['dimensions']['physical_observation']['state'].startswith('not-run') for x in support['updates']))
 with tempfile.TemporaryDirectory(prefix='kernel-boot-export-') as directory:
  work=Path(directory);assessments.export(ROOT,'PR-T48-01',work);check('public-export',all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in public_paths()) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*')))
  for cc in ('gcc','clang'):
   for ref in (False,True):
    out=work/(cc+str(ref));cmd=[sys.executable,work/'labs/kernel_followthrough/boot/build.py','--cc',cc,'--output',out]
    if ref:cmd+=['--assessor-root',ROOT/'assessor/kernel_followthrough/boot']
    p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30);check(cc+('-explicit-reference' if ref else '-incomplete-learner-build'),p.returncode==0 and (out/'kernel.elf').exists())
 result={'passed':all(x['passed'] for x in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():sha(p) for p in (Path(__file__),ROOT/CAMPAIGN,ROOT/SUPPORT)},'scope':'Six exactT48.01 IDs;boot-only author evidence;independent delayed and physical gates remain unobserved.'};dest=(ROOT/report).resolve();need(dest.is_relative_to(ROOT/'reviews/v5/expansion/legacy_kernel_followthrough'),'Report escapes');dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[x['name'] for x in checks if not x['passed']]}));return int(not result['passed'])
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
