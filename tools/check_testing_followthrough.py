"""Exact source, mechanism, mutation, export and authority guard for T32."""
import argparse,copy,hashlib,json,re,subprocess,sys,tempfile
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from c_source import find_unique
from run_testing_followthrough import LAB,REF,BASE,PROFILES,sources,sha
@lru_cache(maxsize=None)
def read(path):
 p=(ROOT/path).resolve()
 if not p.is_relative_to(ROOT.resolve()) or not p.is_file():raise ValueError('Nonlocal/missing evidence '+str(path))
 return p.read_bytes()
def validate_campaign(v):
 def need(x,msg):
  if not x:raise ValueError(msg)
 need(v.get('passed') is True and v.get('source_stable') is True and 'error' not in v,'Unpassed campaign')
 need(v.get('inspected_paths')=={p.relative_to(ROOT).as_posix():sha(p) for p in sources()},'Stale/incomplete source closure')
 artifacts=v.get('artifact_sha256',{});need(bool(artifacts),'Missing artifact closure')
 for p,h in artifacts.items():need(hashlib.sha256(read(p)).hexdigest()==h,'Changed artifact '+p)
 rows={r['label']:r for r in v['commands']};need(len(rows)==len(v['commands']),'Duplicate commands')
 need(len(rows)==404 and len(v['checks'])==135,'Incomplete command/check matrix')
 def text(label):
  r=rows[label];need(r['log'] in artifacts,'Unbound log');return read(r['log']).decode('utf-8')
 def result(label,code,marker):
  need(rows[label]['exit_code']==code,'Wrong exit '+label);need(marker in text(label),'Wrong diagnostic '+label)
 for label,r in rows.items():
  need(r['log'] in artifacts,'Missing raw command log')
  if r['expected_exit'] is not None:need(r['exit_code']==r['expected_exit'],'Failed declared command')
  if r['required_marker']:need(r['required_marker'] in text(label),'Missing declared marker')
  need(r['elapsed_seconds']>=0,'Invalid elapsed time')
  if label.endswith('-build') or label.endswith('-object'):need(r['exit_code']==0,'Compile failure is not semantic evidence')
 for profile,cc,flags in PROFILES:
  for kind,count,prefix in [('transaction',3,'TX'),('coverage',4,'CV')]:
   for variant in [0,*range(1,count+1),'stub']:
    name=profile+'-'+kind+'-'+str(variant);r=rows[name];source=(LAB if variant=='stub' else REF)/(kind+'.c');cmd=rows[name+'-build']['command']
    expected=[cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-D'+prefix+'_MUTANT='+str(0 if variant=='stub' else variant),'-I'+str(LAB),str(source),str(LAB/(kind+'_tests.c')),'-o',r['command'][0]]
    need(cmd==expected and len(r['command'])==1,'Wrong native source/flags '+name);result(name,0 if variant==0 else 1,prefix+('_PASS:' if variant==0 else '_CHECK:'))
    need(not any(s in text(name) for s in ('AddressSanitizer','runtime error:','Segmentation fault')),'Unrelated native failure')
    if kind=='coverage' and variant in (0,1,3,4):result(name+'-weak',0,'CV_WEAK_PASS:')
  result(profile+'-ipc',0,'IPC_PASS:extent,token,sequence,count,owned-child-reaped')
 for cc in ('gcc','clang'):
  for kind in ('transaction','coverage'):need(not text(cc+'-'+kind+'-undefined').strip(),'Undeclared core dependency')
  for repaired in (0,1):
   for mode in (0,1,2):
    result(f'{cc}-plain-diag-{repaired}-{mode}',0 if repaired else 89,'ERROR SUMMARY: 0 errors' if repaired else ['Invalid write','Invalid read','Conditional jump or move depends on uninitialised'][mode])
    result(f'{cc}-asan-diag-{repaired}-{mode}',86 if not repaired and mode<2 else 0,['heap-buffer-overflow','heap-use-after-free'][mode] if not repaired and mode<2 else 'DIAG_TERMINATED')
   result(f'{cc}-ubsan-diag-{repaired}-3',0 if repaired else 86,'DIAG_TERMINATED' if repaired else 'signed integer overflow')
   result(f'{cc}-tsan-{repaired}',0 if repaired else 86,'RACE_COMPLETED:20000' if repaired else 'WARNING: ThreadSanitizer: data race')
   for instrument,flag in [('asan','-fsanitize=address'),('ubsan','-fsanitize=undefined'),('tsan','-fsanitize=thread')]:
    label=f'{cc}-{instrument}-'+(str(repaired) if instrument=='tsan' else 'diag-'+str(repaired))+'-build';cmd=rows[label]['command'];need(flag in cmd and not any('-fsanitize=' in a and a!=flag for a in cmd),'Combined/wrong diagnostic profile')
   need(rows[f'{cc}-plain-diag-{repaired}-0']['command'][:4]==['valgrind','--tool=memcheck','--track-origins=yes','--leak-check=full'],'Wrong Memcheck route')
  result(cc+'-packet-runtime',0,'PACKET_PASS:len0,1,2,9,10,11,count4,unchanged-failure')
  for level in ('O0','O3'):
   for repair in (0,1):
    name=f'{cc}-debug-{level}-{repair}';result(name,0 if repair else 1,'DEBUG_VALUE:4660' if repair else 'DEBUG_VALUE:13330');result(name+'-minimal',0 if repair else 1,'DEBUG_VALUE:1:EXPECTED:1' if repair else 'DEBUG_VALUE:256:EXPECTED:1');result(name+'-equal-control',0,'DEBUG_VALUE:0:EXPECTED:0')
    for deletion in ('delete-first','delete-second','delete-both'):result(name+'-'+deletion,0,'DEBUG_REJECT:extent')
   out=text(f'{cc}-debug-{level}-0-gdb');need('n = 2' in out and 'live_owner = 1' in out and '0x12' in out and '0x34' in out and 'rdi' in out and 'rsi' in out,'Missing debugger discrimination');need('set disable-randomization off' in rows[f'{cc}-debug-{level}-0-gdb']['command'],'Debugger must preserve ASLR')
  need('<optimized out>' in text(cc+'-debug-O3-0-gdb'),'Missing optimized-local observation')
  for release in (0,1):
   for repair in (0,1):
    name=f'{cc}-releasecase-{release}-{repair}';marker='RELEASE_CHECK:' if release and not repair else 'RELEASE_PASS:';result(name,1 if release and not repair else 0,marker);result(name+'-gdb',0,marker)
    if release:result(name+'-asan',1 if not repair else 0,marker)
 for case,marker in [(0,'0 alarms generated'),(1,'out of bounds read'),(2,'accessing out of bounds index. assert n < 10;'),(3,'0 alarms generated')]:result('eva-'+str(case),0,marker)
 need("assertion 'Eva,index_bound' got final status invalid" in text('eva-2'),'Callback counterexample lost invalid status')
 need('-DANALYSIS_CASE=1' in ' '.join(rows['eva-1']['command']) and '-DANALYSIS_CASE=2' in ' '.join(rows['eva-2']['command']),'Wrong static contexts')
 need('NON TERMINATING FUNCTION' in text('eva-1'),'Invalid execution pruning must remain visible')
 need('-fanalyzer' in rows['warnings-packet']['command'],'Missing compiler analyzer probe')
 suppression=json.loads((REF/'suppression.json').read_text());need(suppression['source_sha256']==sha(ROOT/suppression['source']) and suppression['disposition'].startswith('Reject global suppression') and suppression['owner']=='T32 packet component maintainer','Invalid warning disposition')
 asource=(ROOT/suppression['source']).read_text();alarm_line=asource.count('\n',0,find_unique(asource,suppression['alarm_anchor'])[0])+1
 need('packet_analysis.c:%d: Warning:'%alarm_line in text('eva-1') and suppression['mechanism_key']=='unchecked/payload-read/count-extent','Suppression warning not matched by mechanism')
 # Same weak branch coverage, different state effects; full decision condition outcome counts.
 coverage=v['coverage'];need([r['mutant'] for r in coverage]==[0,4],'Missing coverage counterexample')
 weak=[read(r['weak']).decode() for r in coverage];branch=lambda s:re.findall(r'^branch.*$',s,re.M)
 need(branch(weak[0])==branch(weak[1]),'Weak branch paths differ')
 strong=read(coverage[0]['strong']).decode();need('condition outcomes covered 4/4' in strong and 'condition outcomes covered 6/6' in strong and 'condition  0 not covered (true)' in strong,'Wrong coverage or missing defensive exclusion')
 for i in (0,4):result('condition-weak-'+str(i),0,'CV_WEAK_PASS:');result('condition-strong-'+str(i),0 if i==0 else 1,'CV_PASS:' if i==0 else 'CV_CHECK:')
 result('prime-path-limit',0,'paths exceeding limit, giving up path coverage');need('-fpath-coverage-limit=1' in rows['prime-path-limit']['command'],'No actual path limit probe')
 for repair in (0,1):
  for schedule in ('aAbB','abAB','abBA','baAB','baBA','bBaA'):result('schedule-'+str(repair)+'-'+schedule,0,'SCHEDULE none 2' if repair or schedule in ('aAbB','bBaA') else 'SCHEDULE lost-update 1')
 for machine in ('mps2-an385','lm3s6965evb'):
  result(machine,0,'GUEST_PASS:BE_SUM=48641:SVC=11');need(rows[machine]['command'][2]==machine and '-kernel' in rows[machine]['command'],'Wrong named machine')
 need(len(v['overhead'])==6,'Missing workload builds')
 expected=sum((i*17+r)%65521 for r in range(512) for i in range(4096))
 for measurement in v['overhead']:
  name=measurement['compiler']+'-workload-'+measurement['profile'];need(len(measurement['samples'])==3 and measurement['binary'] in artifacts,'Missing size/resource artifacts')
  for k,sample in enumerate(measurement['samples']):
   out=text(name+'-'+str(k));m=re.search(r'WORKLOAD:(\d+):ns=(\d+):resolution_ns=(\d+)',out);rss=re.search(r'TIME_RSS:([\d.]+):(\d+)',out)
   need(m is not None and rss is not None and int(m[1])==expected,'Wrong workload relation');need(sample=={'inner_ns':int(m[2]),'resolution_ns':int(m[3]),'outer_seconds':float(rss[1]),'peak_rss_kib':int(rss[2])},'Invented overhead observation')
  need('text' in text(name+'-size') and 'bss' in text(name+'-size'),'Missing code/data size')
 result('independent-models',0,'MODELS_PASS:');model=json.loads(text('independent-models').splitlines()[0]);need(model['equivalence']=={'4':True,'5':True} and model['wire_sum']==48641 and model['wrong_native_little_sum']==65981,'Wrong independent relation')
 need(v['replay']['environment']=={'length':2,'live_owner':1,'wire_order':'big-endian'},'Lost replay premises')
 for r in v['replay']['cases']:need(read(r['path'])==bytes.fromhex(r['hex']) and hashlib.sha256(read(r['path'])).hexdigest()==r['sha256'],'Changed saved input')
 need(len(v['open_gates'])==2 and 'unobserved' in v['open_gates'][0] and 'independent review' in v['open_gates'][1],'Invented external completion')
 need(all(r['passed'] is True for r in v['checks']),'Failed source checks')
 return True
def main(report):
 from research.v5.expansion.legacy_testing_followthrough.author_support import attach,SUPPORT,PROJECTS,public_paths
 from cmastery import assessments
 dest=(ROOT/report).resolve();assert dest.is_relative_to(BASE.resolve());campaign=BASE/'campaign.json';v=json.loads(campaign.read_text());validate_campaign(v);checks=[{'name':'exact-current-mechanism-and-artifact-closure','passed':True}]
 def check(name,result):checks.append({'name':name,'passed':bool(result)})
 controls=[('missing-command',lambda x:x['commands'].pop()),('wrong-source',lambda x:x['inspected_paths'].update({'labs/testing_followthrough/transaction.h':'0'*64})),('wrong-artifact',lambda x:x['artifact_sha256'].update({next(iter(x['artifact_sha256'])):'0'*64})),('mutant-as-pass',lambda x:next(r for r in x['commands'] if r['label']=='gcc-debug-transaction-1').update(exit_code=0)),('wrong-sanitizer',lambda x:next(r for r in x['commands'] if r['label']=='gcc-asan-diag-0-build')['command'].remove('-fsanitize=address')),('startup-as-race',lambda x:next(r for r in x['commands'] if r['label']=='clang-tsan-0').update(exit_code=139)),('wrong-emulator',lambda x:next(r for r in x['commands'] if r['label']=='mps2-an385')['command'].__setitem__(2,'invented-board')),('invented-overhead',lambda x:x['overhead'][0]['samples'][0].update(peak_rss_kib=0)),('lost-input',lambda x:x['replay']['cases'][0].update(hex='ffff')),('invented-physical',lambda x:x.update(open_gates=[]))]
 for name,mutate in controls:
  d=copy.deepcopy(v);mutate(d)
  try:validate_campaign(d);rejected=False
  except (ValueError,KeyError,IndexError):rejected=True
  check(name,rejected)
 for pid in PROJECTS:
  path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(pid+'.json');r=json.loads(path.read_text());old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True));original=copy.deepcopy(r['criteria'])
  for c in original:c.pop('successor_support',None)
  check(pid+'-original-criteria',original==old['criteria']);check(pid+'-baseline',all(r[k]==old[k] for k in ('baseline_binding','execution_support','status')));check(pid+'-idempotent',attach(copy.deepcopy(r))==r)
  with tempfile.TemporaryDirectory(prefix='testing-followthrough-export-') as d:
   work=Path(d);assessments.export(ROOT,pid,work);check(pid+'-public-export',all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in public_paths()) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*')))
   if pid in ('PR-T32-04','PR-T32-06'):
    kind='transaction' if pid.endswith('04') else 'coverage';prefix='TX' if kind=='transaction' else 'CV'
    for ref in (False,True):
     exe=work/('check-'+str(ref));source=REF/(kind+'.c') if ref else work/'labs/testing_followthrough'/(kind+'.c');cmd=[sys.executable,work/'labs/testing_followthrough/build.py','--kind',kind,'--source',source,'--output',exe];b=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60);okay=False
     if b.returncode==0:
      e=subprocess.run([exe],capture_output=True,text=True,timeout=30);okay=e.returncode==(0 if ref else 1) and prefix+('_PASS:' if ref else '_CHECK:') in e.stdout+e.stderr
     check(pid+('-explicit-reference' if ref else '-incomplete-stub'),okay)
 support=json.loads((ROOT/SUPPORT).read_text());expected={c['id'] for pid in PROJECTS for c in json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(pid+'.json')).read_text())['criteria']};expected|={x+'-DELAYED' for x in list(expected)}
 check('48-exact-original-identities',len(support['updates'])==48 and {r['id'] for r in support['updates']}==expected);check('support-source-pins',all(hashlib.sha256(read(p)).hexdigest()==h for p,h in support['source_sha256'].items()));check('authority-separate',all(r['dimensions']['independent_learner']['state']=='not-assessed' and r['dimensions']['external_review']['state']=='not-run' and r['dimensions']['physical_observation']['state'].startswith('not-run') for r in support['updates']))
 result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():sha(p) for p in (Path(__file__).resolve(),campaign,ROOT/SUPPORT)},'scope':'Source-bound author mechanism checks and learner exports; independent reviewer, delayed learner and physical captures remain separate.'};dest.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
