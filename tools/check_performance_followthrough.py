"""Read-only source/receipt guard with ordinary learner-export checks."""
import argparse,copy,hashlib,json,re,subprocess,sys,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];BASE=ROOT/'reviews/v5/expansion/legacy_performance_followthrough';LAB=ROOT/'labs/performance_followthrough';REF=ROOT/'assessor/performance_followthrough';sys.path[:0]=[str(ROOT),str(LAB),str(ROOT/'tools')]
from run_performance_followthrough import sources
import analyze
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def need(x,message):
 if not x:raise ValueError(message)
def read(p):
 path=(ROOT/p).resolve();need(path.is_relative_to(ROOT.resolve()),'Escaped path');return path.read_bytes()
def validate_campaign(v):
 need(v.get('passed') is True and v.get('error') is None,'Campaign failed');need(v['inspected_paths']=={p.relative_to(ROOT).as_posix():sha(p) for p in sources()},'Stale source closure');art=v['artifact_sha256'];need(art and all(hashlib.sha256(read(p)).hexdigest()==h for p,h in art.items()),'Missing/stale artifact');rows={r['label']:r for r in v['commands']};need(len(rows)==len(v['commands'])==156,'Missing/duplicate command');obs=v['observations']
 def text(name,code=0):
  r=rows[name];need(r['exit_code']==r['expected_exit_code']==code,'Wrong outcome '+name);need(r['output'] in art,'Unbound output');return read(r['output']).decode()
 def flags(name,wanted):need(all(x in rows[name]['command'] for x in wanted),'Wrong command '+name)
 need('15.2' in text('gcc-version') and '21.1.8' in text('clang-version') and '17.1' in text('gdb-version'),'Wrong tools')
 for name in ('gprof-version','kernel','cpu','filesystem','cgroup-membership'):need(bool(text(name).strip()),'Missing environment')
 certificates=json.loads(text('certificates'));need(certificates==obs['certificates'] and certificates['passed'] and len(certificates['records'])==16,'Wrong certificate report');records=certificates['records'];need(records['potential']['quarter_shrink_max_amortized']==3 and records['finite-growth']['observed_equal_through']==1024 and records['samples']['paired_difference_median']==3 and records['cgroup']['gauge']['memory']['value']==-1024,'Wrong distinct model results')
 for cc in ('gcc','clang'):
  for profile,wanted in [('debug',['-O0','-g']),('release',['-O2','-DNDEBUG'])]:
   prefix=cc+'-'+profile;flags(prefix+'-core-build',['-std=c17','-Werror','-pedantic',*wanted,str(REF/'cost.c'),str(REF/'byte_helpers.c')]);text(prefix+'-core-build');core=text(prefix+'-core')
   for marker in ('sort:retained-and-byte-and-binary:1093','cache:LRU-and-FIFO:9331','table:nonemptyPhi2:prefixActual=91:amortized=105:failedAttempts=3:max=33','payload:fallible-deep-copy-bytes0,1,8,37','write:retained-prefix-faults=40:budget4-prefix5:vectored3,2,3','async:retained-cancel-not-complete','interval:borrow1:equal0:reverse-overflow-unchanged','pmu:2400,6000:CPI5/2:zero-running,mixed-group,duplicate-id','hooks:work9/4:child5/5:recursion7/5:interleaved-context9','dwt:capture-only:64cycles4000ns','PF_PASS:all'):need(marker in core,'Missing mechanism '+marker)
  need('V2:E3:Z9:byte-comparisons9:parallel-retained:stream-attempt5-accepted4-prefix' in text(cc+'-graph'),'Wrong graph workload');text(cc+'-graph-build')
  actual=[json.loads(s) for s in text(cc+'-host').splitlines()];need(actual==obs[cc+'-host'] and len(actual)==10,'Wrong host report');named={r['case']:r for r in actual if r['case']!='read-pass'};clock=named['clocks'];need(clock['process_cpu_ns']>0 and all(x>0 for x in clock['worker_cpu_ns']) and clock['wall_minus_process_ns']==clock['elapsed_ns']-clock['process_cpu_ns'],'Clock scopes flattened');need(named['child-clock-scope']['waited_child_cpu_us']>0,'No actual child observation');need(named['write-sync']['accepted']==8192 and named['write-sync']['fsync_status']==0,'Missing synchronization');reads=[r for r in actual if r['case']=='read-pass'];need([r['pass'] for r in reads]==[0,1,2] and all(r['accepted_bytes']==8192 and r['syscalls']>0 and r['device_bytes'] is None for r in reads) and reads[2]['after_owned_other_process'],'Missing repeated read scope');need(named['injected-fsync-error']['status']=='failed' and named['injected-fsync-error']['source']=='local-adapter' and named['injected-fsync-error']['synchronizer_calls']==1 and named['injected-fsync-error']['publication']=='stage-retained' and named['injected-fsync-error']['destination_unchanged'] is True,'Injected error fabricated');need(named['publish']['boundaries']==['write8','file-fsync','rename','directory-fsync'] and named['publish']['power_loss']=='unobserved','Persistence overstated');need(named['sparse-preallocated']['payload']==8 and named['sparse-preallocated']['physical_device_traffic'] is None,'Traffic collapsed');need(named['memory']['payload_live']==[0,1048576,0] and len(named['memory']['rss_pages'])==3 and named['memory']['leak_conclusion']=='not-established','RSS flattened')
  for kind in ('host','inline'):text(cc+'-'+kind+'-build');text(cc+'-'+kind+'-symbols');text(cc+'-'+kind+'-disassembly')
  need(re.search(r'call.*<pf_lower_core>',text(cc+'-host-disassembly')) and not re.search(r'call.*<pf_lower_core>',text(cc+'-inline-disassembly')),'Inlining boundary did not change')
  for kind in ('pairs','inline-pairs'):need(analyze.pairs(text(cc+'-'+kind))==obs[cc+'-'+kind],'Wrong actual paired analysis')
  pmu=[json.loads(s) for s in text(cc+'-pmu').splitlines()];need(pmu==obs[cc+'-pmu'] and {r['event'] for r in pmu}=={'instructions','cycles','branch-misses','cache-misses'} and len(pmu)==4,'Wrong PMU requests')
  for r in pmu:
   if r['status']=='measured':need(r['raw']>=0 and 0<r['running']<=r['enabled'],'Invalid raw event')
   else:need(r['raw'] is None and r['status'] in ('denied','unavailable','never-scheduled','io-error','malformed') and (r['errno']>0 or r['status'] in ('never-scheduled','malformed')) and r['pid']==0 and r['cpu']==-1,'Unavailable became measured zero')
  c=json.loads(text(cc+'-cgroup'));need(c==obs[cc+'-cgroup'] and c['passed'] and c['policy_writes']==c['membership_writes']==0 and not c['same_descriptor_peak_reset'] and c['workload_code']==0 and c['ancestors'],'Cgroup scope changed');need(c['object_ledger_bytes']==[0,1048576,0] and 'shared' in c['scope'],'Object/aggregate scope lost')
  for name in ('cost-object','helper-object','closed-object'):text(cc+'-'+name)
  flags(cc+'-cost-object',['-std=c17','-ffreestanding','-fno-builtin','-U_FORTIFY_SOURCE']);raw=set(text(cc+'-cost-undefined').split())-{'U'};need(raw==({'memcpy'} if cc=='clang' else set()),'Unexpected core runtime');need(not text(cc+'-closed-undefined').strip(),'Unclosed target helper')
  for kind in ('stack','lto'):
   prefix=cc+'-'+kind;text(prefix+'-build');need('CFG_WORK:16:64' in text(prefix),'Missing actual different loop work');sym=analyze.symbols(text(prefix+'-symbols'));need(any(k.startswith('pf_sequential') for k in sym) and any(k.startswith('pf_nested') for k in sym),'Missing independent code sizes');need(analyze.callgraph(text(prefix+'-disassembly'))==obs[prefix+'-callgraph'] and obs[prefix+'-callgraph']['bounded_whole_program_stack'] is None,'Wrong actual callgraph')
   if kind=='lto':flags(prefix+'-build',['-flto'])
  for mode in ('base','hook','drop','pg'):
   prefix=cc+'-'+mode;raw=text(prefix);syms=text(prefix+'-symbols');text(prefix+'-disassembly');text(prefix+'-size');text(prefix+'-link')
   for source in ('main','kernel','helper'):text(prefix+'-'+source+'-compile')
   elapsed,checksum=map(int,re.search(r'PROFILE_RUN:(\d+):(\d+)',raw).groups());a,b,s=map(int,re.search(r'CALLERS:2:(\d+):8:(\d+):(\d+)',raw).groups());o=obs[prefix+'-perturbation'];symbols=analyze.symbols(syms);need(o['elapsed_ns']==elapsed and o['checksum']==checksum==16017 and o['caller_expensive_2_ns']==a and o['caller_cheap_8_ns']==b and o['algo_lower_code_bytes']==symbols['algo_lower']['bytes'] and o['single_trial_not_statistical'],'Wrong matched perturbation ledger')
   if mode in ('hook','drop'):
    flags(prefix+'-main-compile',['-finstrument-functions','-DPF_HOOKS']);flags(prefix+'-kernel-compile',['-finstrument-functions']);text(prefix+'-recorder-compile');need('-finstrument-functions' not in rows[prefix+'-helper-compile']['command'] and '-finstrument-functions' not in rows[prefix+'-recorder-compile']['command'],'Recursive recorder or instrumented helper');h=analyze.hooks(raw,syms);need(h==obs[prefix] and h['complete']==(mode=='hook') and h['uninstrumented_helper_boundary_absent'],'Wrong actual trace reducer')
   if mode=='pg':
    for stage in ('main-compile','kernel-compile','link'):flags(prefix+'-'+stage,['-pg'])
    gp=text(prefix+'-gprof');need(re.search(r'\b1000\s+[^\n]*algo_lower',gp) and obs[prefix+'-gprof']['no_time_samples_accumulated']==('no time accumulated' in gp),'Wrong gprof scope');need(any(p.endswith('/'+prefix+'/gmon.out') for p in art),'Missing normal-exit gmon')
  need(obs[cc+'-hook-perturbation']['algo_lower_code_bytes']!=obs[cc+'-base-perturbation']['algo_lower_code_bytes'],'No observed code perturbation')
 static=json.loads(json.dumps(analyze.lower_cfg(text('trace-disassembly'))));need(static==obs['actual-static-CFG'] and static['blocks']==6 and static['edges']==7 and static['metric']==3,'Wrong connected static CFG');text('trace-build');text('trace-symbols')
 for name in ('empty','duplicate','index4a','index4b'):need(analyze.trace(text('trace-'+name))==obs['trace-'+name],'Wrong instruction trace')
 need(obs['trace-index4a']['result']==obs['trace-index4b']['result']==4 and obs['trace-index4a']['relative_pc_path']!=obs['trace-index4b']['relative_pc_path'] and obs['trace-empty']['instructions']<obs['trace-duplicate']['instructions'],'Trace changed conditions absent')
 for name in ('stability','flush','borrow','wrap'):text(name+'-mutant-build');need('PF_CHECK:' in text(name+'-mutant',1),'Semantic mutant survived')
 text('stub-build');need('PF_CHECK:' in text('stub',1),'Learner stub solved');su=[read(p).decode() for p in art if p.endswith('.su')];need(any('dynamic,bounded' in s for s in su) and any('\tdynamic\n' in s for s in su) and any('\tstatic\n' in s for s in su),'Missing stack qualifiers');need(len(v['checks'])==42 and all(x['passed'] is True for x in v['checks']),'Missing/failed producer checks');need(len(v['open_gates'])==4 and 'unobserved' in v['open_gates'][0] and 'physical' in v['open_gates'][3],'External observation fabricated');return True
def main(report):
 from research.v5.expansion.legacy_performance_followthrough.author_support import attach,SUPPORT,PROJECTS,public_paths
 from cmastery import assessments
 v=json.loads((BASE/'campaign.json').read_text());validate_campaign(v);checks=[{'name':'current-source-bound-mechanisms','passed':True}]
 def check(name,value):checks.append({'name':name,'passed':bool(value)})
 controls=[('missing-command',lambda x:x['commands'].pop()),('stale-source',lambda x:x['inspected_paths'].update({'labs/performance_followthrough/cost.h':'0'*64})),('stale-artifact',lambda x:x['artifact_sha256'].update({next(iter(x['artifact_sha256'])):'0'*64})),('fake-stub-pass',lambda x:next(r for r in x['commands'] if r['label']=='stub').update(exit_code=0)),('wrong-build',lambda x:next(r for r in x['commands'] if r['label']=='clang-release-core-build')['command'].remove('-DNDEBUG')),('changed-trace',lambda x:x['observations']['trace-index4b'].update(result=5)),('fake-cgroup-isolation',lambda x:x['observations']['gcc-cgroup'].update(policy_writes=1)),('invented-external',lambda x:x.update(open_gates=[]))]
 for name,mutate in controls:
  d=copy.deepcopy(v);mutate(d)
  try:validate_campaign(d);reject=False
  except (ValueError,KeyError,IndexError,AssertionError):reject=True
  check(name,reject)
 for pid in PROJECTS:
  path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(pid+'.json');r=json.loads(path.read_text());old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True));criteria=copy.deepcopy(r['criteria'])
  for c in criteria:c.pop('successor_support',None)
  check(pid+'-original-criteria',criteria==old['criteria']);check(pid+'-historical-contracts',all(r[k]==old[k] for k in ('baseline_binding','execution_support','status')));check(pid+'-idempotent',attach(copy.deepcopy(r))==r)
  with tempfile.TemporaryDirectory(prefix='performance-follow-export-') as d:
   work=Path(d);assessments.export(ROOT,pid,work);check(pid+'-public-export',all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in public_paths()) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*')))
   if pid in ('PR-T43-01','PR-T43-12'):
    for ref in (False,True):
     exe=work/('core-'+str(ref));cmd=[sys.executable,work/'labs/performance_followthrough/build.py','--output',exe]
     if ref:cmd+=['--cost-source',REF/'cost.c','--foundation-source',ROOT/'assessor/foundations/foundation.c','--container-source',ROOT/'assessor/containers_depth/sequence.c']
     b=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60);okay=False
     if b.returncode==0:
      e=subprocess.run([exe],capture_output=True,text=True,timeout=10);okay=e.returncode==(0 if ref else 1) and ('PF_PASS:' if ref else 'PF_CHECK:') in e.stdout+e.stderr
     check(pid+('-explicit-reference' if ref else '-incomplete-stub'),okay)
 support=json.loads((ROOT/SUPPORT).read_text());original=[c for pid in PROJECTS for c in json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(pid+'.json')).read_text())['criteria']];expected={c['id'] for c in original}|{x['id'] for c in original for x in c['variants']};check('84-exact-identities',len(support['updates'])==84 and {r['id'] for r in support['updates']}==expected);check('support-pins',all(hashlib.sha256(read(p)).hexdigest()==h for p,h in support['source_sha256'].items()));check('external-authority-separate',all(r['dimensions']['external_review']['state']=='not-run' and r['dimensions']['independent_learner']['state']=='not-assessed' and r['dimensions']['physical_observation']['state'].startswith('not-run') for r in support['updates']))
 result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():sha(p) for p in (Path(__file__).resolve(),BASE/'campaign.json',ROOT/SUPPORT)},'scope':'Author source/receipt closure and unsolved learner exports;external dimensions remain separate.'};dest=(ROOT/report).resolve();need(dest.is_relative_to(BASE.resolve()),'Report escapes');dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[c['name'] for c in checks if not c['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
