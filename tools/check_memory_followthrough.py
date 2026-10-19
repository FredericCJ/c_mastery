"""Read-only T45 source/artifact/claim guard and incomplete learner export checks."""
import argparse,copy,hashlib,json,re,subprocess,sys,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];BASE=ROOT/'reviews/v5/expansion/legacy_memory_followthrough';LAB=ROOT/'labs/memory_followthrough';REF=ROOT/'assessor/memory_followthrough';sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
from run_memory_followthrough import sources
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def need(value,message):
 if not value:raise ValueError(message)
def read(name):
 p=(ROOT/name).resolve();need(p.is_relative_to(ROOT.resolve()),'Escaped path');return p.read_bytes()
def validate_campaign(v):
 need(v.get('passed') is True and v.get('error') is None,'Failed campaign');need(v['inspected_paths']=={p.relative_to(ROOT).as_posix():sha(p) for p in sources()},'Stale source closure');art=v['artifact_sha256'];need(art and all(hashlib.sha256(read(p)).hexdigest()==h for p,h in art.items()),'Stale artifact');rows={r['label']:r for r in v['commands']};need(len(rows)==len(v['commands'])==225,'Missing/duplicate commands');obs=v['observations']
 def text(name,code=0):
  row=rows[name];need(row['exit_code']==row['expected_exit_code']==code,'Wrong outcome '+name);need(row['output'] in art,'Unbound output');return read(row['output']).decode()
 def flags(name,wanted):need(all(x in rows[name]['command'] for x in wanted),'Wrong command '+name)
 need('15.2' in text('gcc-version') and '21.1.8' in text('clang-version') and '17.1' in text('gdb-version') and '3.26' in text('valgrind-version'),'Tool mismatch')
 certificates=json.loads(text('certificates'));need(certificates==obs['certificates'] and certificates['passed'] and len(certificates['records'])==10,'Premise certificate missing');records=certificates['records'];need(records['stack-paths']['unknown_indirect_whole_bound'] is None and records['stack-paths']['MSP_shared_peak']==296 and records['stack-paths']['FP_changed_peak']==368,'Unknown stack promoted');need(records['ownership-graph']['strong_reachable']==['C'] and records['ownership-graph']['weak_is_not_owning']==['A','B'],'Weak ownership changed');need(records['fault-observation']['BFAR_without_valid_bit'] is None and records['fault-observation']['missing_heap'] is None,'Unknown target memory invented');need(records['causal-prefixes']['changed_first_bad']==2 and records['causal-prefixes']['changed_symptom_event']==7,'Symptom mistaken for cause')
 markers=['fill1287:move729:scan285','accepted12-refused-all:rejected8-zero-calls','560-gap48-gap16-overflow40','200-cycles:two-borrows-expire','24-staged-cases:five-failure-prefixes','all-six-permutations:one-terminal-result','sync-completion-before-return','nested-leases2','retire-capacity','896:DFS7-vs-BFS64','MW_PASS:core']
 for cc in ('gcc','clang'):
  for profile,expected in [('O0',['-O0','-g','-fno-omit-frame-pointer']),('O2',['-O2','-g','-DNDEBUG','-fomit-frame-pointer'])]:
   prefix=cc+'-'+profile;flags(prefix+'-core-build',['-std=c17','-Werror','-pedantic',*expected,str(REF/'memory.c')]);text(prefix+'-core-build');need(all(m in text(prefix+'-core') for m in markers),'Missing core mechanism');need('800-prefix-cycles:32-80-96:reverse-cleanup:first-cause:child-transfer:epoch-bound64' in text(prefix+'-resources') and 'budget16:denied50:peak16:live0' in text(prefix+'-resources'),'Resource cleanup/retention absent');need('changed24:reserved64:restored8:untouched32' in text(prefix+'-pattern') and 'depth6:leaves64' in text(prefix+'-pattern'),'Stack measurement confused')
   bounded=json.loads(text(prefix+'-bounded'));need(bounded==obs[prefix+'-bounded'] and bounded['passed'] and bounded['parent_limits_unchanged'] and bounded['signal_is_not_a_portable_UB_oracle'],'Bounded stack report fabricated');low,high=bounded['records'];need(low['depth']==1 and low['exit_code']==0 and high['depth']==128 and high['exit_code']<0 and all(r['entered'] and 'stack-limit65536:core0' in r['stdout'] for r in bounded['records']),'Setup/entry not established')
   for specimen in ('provider','work','original'):
    label=prefix+'-'+specimen+'-stack-usage';text(label);flags(label,['-fstack-usage',*expected]);obj=Path(rows[label]['command'][-1]);su=obj.with_suffix('.su');need(su.relative_to(ROOT).as_posix() in art and su.read_text()==obs[prefix+'-'+specimen+'-su'],'Stack records unbound');assembly=text(prefix+'-'+specimen+'-assembly');need(('pf_dynamic' if specimen=='provider' else 'mw_tree' if specimen=='work' else 'recursive_tree') in assembly,'No actual object mechanism')
   for fixed in (0,1):
    for instrument in ('plain','asan','ubsan'):
     name=f'{cc}-{profile}-{instrument}-{fixed}';text(name+'-build');flags(name+'-build',['-std=c17','-'+profile,'-DMW_FIXED='+str(fixed)]);cmd=rows[name+'-build']['command'];need(('-fsanitize=address' in cmd) if instrument=='asan' else ('-fsanitize=undefined' in cmd) if instrument=='ubsan' else not any('-fsanitize=' in x for x in cmd),'Mixed observer profile');modes=('extent','lower','nul','pool','init','free','lifetime','canary') if instrument=='asan' else ('signed',) if instrument=='ubsan' else ('init','leak')
     for mode in modes:
      code=0 if fixed else 89 if instrument=='plain' else 86 if mode not in ('pool','init','canary') else 0;raw=text(name+'-'+mode,code);need('SETUP:allocation8' in raw and 'ENTRY' in raw,'Control not entered')
      if code==86:need(('signed integer overflow' if instrument=='ubsan' else 'heap-use-after-free' if mode=='lifetime' else 'AddressSanitizer') in raw,'No actual detector report')
      elif instrument=='plain':need(('ERROR SUMMARY: 0 errors' if fixed else 'Conditional jump or move depends on uninitialised' if mode=='init' else 'LEAK SUMMARY') in raw,'Wrong Memcheck observation')
      elif mode=='pool':need(('DENIED:logical-extent' if fixed else 'SEMANTIC:logical-extent-violated') in raw,'Logical boundary confused')
      elif mode=='canary':need(('DENIED:before-canary' if fixed else 'CANARY:damage-detected-after-extent-violation') in raw,'Mitigation called prevention')
      else:need('EXIT:bounded-trial' in raw,'Silent trial not completed')
  need('MW_PASS:core' in text(cc+'-core-sanitized'),'No instrumented semantic regression');flags(cc+'-core-sanitized-build',['-fsanitize=address,undefined']);need(not text(cc+'-undefined').strip(),'Freestanding host core not closed');flags(cc+'-memory-object',['-std=c17','-ffreestanding','-fno-builtin']);text(cc+'-memory-object');need('ckd_add' in text(cc+'-ckd-header-read') and 'ckd_mul' in text(cc+'-ckd-header-read'),'Installed optional header not read');flags(cc+'-checked23-build',['-std=c23']);need('draft-scoped:cases98:temporary-preserves:wrapped-result0-on-error:not-C17-baseline' in text(cc+'-checked23'),'Optional comparison substituted baseline')
  for coverage in ('plain','asan'):
   flags(cc+'-coverage-'+coverage+'-build',['-fsanitize=address']);text(cc+'-dependency-'+coverage+'-build');need(('-fsanitize=address' in rows[cc+'-dependency-'+coverage+'-build']['command'])==(coverage=='asan'),'Dependency coverage not changed');raw=text(cc+'-coverage-'+coverage,86 if coverage=='asan' else 0);need('ENTRY' in raw and ('heap-buffer-overflow' if coverage=='asan' else 'EXIT:bounded-trial') in raw,'Coverage control absent')
  need('Conditional jump or move depends on uninitialised' in text(cc+'-memcheck-uninstrumented',89),'Binary definedness control missing')
 need('dynamic,bounded' in obs['gcc-O0-provider-su'] and '\tdynamic' in obs['gcc-O2-provider-su'],'Qualifiers removed')
 for bad in (0,1):need(('bad-caller-reclaims-shared-context-during-successor' if bad else 'fixed-context-held-for-successor:caller-timing-changed') in text('hidden-'+str(bad)),'Changed caller ownership missing')
 need(text('watch-map-before')==text('watch-map-after'),'Legal corruption grew sections');gdb=json.loads(text('watch-gdb').split('MW_GDB:',1)[1].splitlines()[0]);need(gdb==obs['gdb'] and gdb['native_core'] is False and gdb['omitted_memory']=='unknown' and gdb['ASLR']=='preserved','Debugger scope inflated');commands=[r['command'] for r in gdb['records']];need(commands.count('watch *p')==2 and commands.count('delete breakpoints')==3 and commands[-2:]==['delete breakpoints','continue'],'Watch not removed before lifetime end');need(any(r['command']=='print generation' and ' = 2' in r['output'] for r in gdb['records']) and any(r['command']=='print *p' and ' = 82' in r['output'] for r in gdb['records']),'Relocated watch not observed');need('New value = 99' in text('watch-gdb') and 'New value = 82' in text('watch-gdb') and 'exited normally' in text('watch-gdb'),'Watch/run incomplete')
 need(not text('image-undefined').strip(),'Image runtime closure open');sections=text('image-sections');need(re.search(r'\.data\s+00000060\s+20000000\s+00000[0-9a-f]+',sections) and '.retained' in sections,'LMA/VMA not separate');need('copy96:zero400:retained64:two-initializations:not-physical-reset' in text('image-qemu'),'Target startup result inflated')
 for kind in ('ram-overflow','flash-overflow'):
  row=rows['image-'+kind+'-link'];raw=read(row['output']).decode();need(row['exit_code']!=0 and row['expected_exit_code'] is None and ('overflow' in raw or 'will not fit' in raw),'Missing actual link rejection')
 need('MW_CHECK:' in text('stub',1),'Learner stub solved');from research.v5.expansion.legacy_memory_followthrough.criterion_notes import NOTES
 need(len(NOTES)==30 and all(selector in rows for _,_,selectors in NOTES.values() for selector in selectors),'Exact criterion selector absent');need(len(v['checks'])==227 and all(c['passed'] is True for c in v['checks']),'Missing mechanism checks');need(len(v['open_gates'])==4 and 'seven-day' in v['open_gates'][1] and 'Physical' in v['open_gates'][2] and 'draft' in v['open_gates'][3],'External evidence invented');return True
def main(report):
 from research.v5.expansion.legacy_memory_followthrough.author_support import attach,SUPPORT,PROJECTS,public_paths
 from cmastery import assessments
 v=json.loads((BASE/'campaign.json').read_text());validate_campaign(v);checks=[{'name':'source-bound-exact-mechanisms','passed':True}]
 def check(name,value):checks.append({'name':name,'passed':bool(value)})
 controls=[('missing-command',lambda x:x['commands'].pop()),('stale-source',lambda x:x['inspected_paths'].update({'labs/memory_followthrough/memory.h':'0'*64})),('stale-artifact',lambda x:x['artifact_sha256'].update({next(iter(x['artifact_sha256'])):'0'*64})),('fake-stub-pass',lambda x:next(r for r in x['commands'] if r['label']=='stub').update(exit_code=0)),('wrong-language-baseline',lambda x:next(r for r in x['commands'] if r['label']=='gcc-O2-core-build')['command'].remove('-std=c17')),('fake-stack-entry',lambda x:x['observations']['clang-O2-bounded']['records'][1].update(entered=False)),('invented-memory',lambda x:x['observations']['gdb'].update(omitted_memory='captured-all')),('external-completed',lambda x:x.update(open_gates=[]))]
 for name,mutate in controls:
  value=copy.deepcopy(v);mutate(value)
  try:validate_campaign(value);rejected=False
  except (ValueError,KeyError,IndexError,AssertionError):rejected=True
  check(name,rejected)
 for pid in PROJECTS:
  path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(pid+'.json');r=json.loads(path.read_text());old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True));criteria=copy.deepcopy(r['criteria'])
  for c in criteria:c.pop('successor_support',None)
  check(pid+'-original-criteria',criteria==old['criteria']);check(pid+'-historical-contracts',all(r[k]==old[k] for k in ('baseline_binding','execution_support','status')));check(pid+'-idempotent',attach(copy.deepcopy(r))==r)
  with tempfile.TemporaryDirectory(prefix='memory-follow-export-') as directory:
   work=Path(directory);assessments.export(ROOT,pid,work);check(pid+'-public-export',all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in public_paths()) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*')))
   if pid in ('PR-T45-01','PR-T45-10'):
    for ref in (False,True):
     exe=work/('core-'+str(ref));cmd=[sys.executable,work/'labs/memory_followthrough/build.py','--output',exe]
     if ref:cmd+=['--memory-source',REF/'memory.c','--foundation-source',ROOT/'assessor/foundations/foundation.c']
     build=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60);okay=False
     if build.returncode==0:
      run=subprocess.run([exe],capture_output=True,text=True,timeout=10);okay=run.returncode==(0 if ref else 1) and ('MW_PASS:' if ref else 'MW_CHECK:') in run.stdout+run.stderr
     check(pid+('-explicit-reference' if ref else '-incomplete-stub'),okay)
 support=json.loads((ROOT/SUPPORT).read_text());original=[c for pid in PROJECTS for c in json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(pid+'.json')).read_text())['criteria']];expected={c['id'] for c in original}|{v['id'] for c in original for v in c['variants']};check('60-exact-identities',len(support['updates'])==60 and {r['id'] for r in support['updates']}==expected);check('support-source-pins',all(hashlib.sha256(read(p)).hexdigest()==h for p,h in support['source_sha256'].items()));check('external-authority-separate',all(r['dimensions']['external_review']['state']=='not-run' and r['dimensions']['independent_learner']['state']=='not-assessed' and r['dimensions']['physical_observation']['state'].startswith('not-run') for r in support['updates']));mapping=json.loads((LAB/'criteria.json').read_text())['criteria'];check('exact-task-and-variant-preservation',all(next(r for r in mapping if r['id']==c['id'])['original_task']==c['task'] and next(r for r in mapping if r['id']==c['id'])['changed_conditions']==c['variants'] for c in original))
 result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():sha(p) for p in (Path(__file__).resolve(),BASE/'campaign.json',ROOT/SUPPORT)},'scope':'Exact30 original criteria+30 original delayed variants;author execution and independent/physical/source authority separate.'};dest=(ROOT/report).resolve();need(dest.is_relative_to(BASE.resolve()),'Report escapes');dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[c['name'] for c in checks if not c['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
