"""Check source freshness, retained original criteria and standalone learner export."""
import argparse,copy,hashlib,json,subprocess,sys,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from cmastery import cli
from research.v5.expansion.legacy_posix_followthrough.author_support import attach
from research.v5.expansion.legacy_posix_mapping.author_support import attach as attach_mapping
from research.v5.expansion.legacy_posix_namespace.author_support import attach as attach_namespace
from research.v5.expansion.legacy_posix_io.author_support import attach as attach_io

def fresh(report,expected=168,profiles=6):
 if not report.get('passed') or len(report.get('profiles',[]))!=profiles:raise ValueError('Campaign did not pass all required profiles')
 if len([o for p in report['profiles'] for o in p['outcomes'] if o.get('passed')])!=expected:raise ValueError('Missing executed reference/control outcome')
 for path,digest in report['inspected_paths'].items():
  if hashlib.sha256((ROOT/path).read_bytes()).hexdigest()!=digest:raise ValueError('Stale source '+path)
 for path,digest in report['log_sha256'].items():
  if hashlib.sha256((ROOT/path).read_bytes()).hexdigest()!=digest:raise ValueError('Stale log '+path)

def main(report_path,mapping=False,namespace=False,io=False):
 if sum((mapping,namespace,io))>1:raise ValueError('Select one successor')
 batch='legacy_posix_io' if io else ('legacy_posix_namespace' if namespace else ('legacy_posix_mapping' if mapping else 'legacy_posix_followthrough'))
 subdir='io' if io else ('namespace' if namespace else ('mapping' if mapping else 'followthrough'));expected_outcomes=149 if io else (132 if namespace else (114 if mapping else 168))
 project='PR-T23-02' if namespace or io else ('PR-T23-03' if mapping else 'PR-T23-01');attach_current=attach_io if io else (attach_namespace if namespace else (attach_mapping if mapping else attach))
 campaign_file='io_validation.json' if io else ('namespace_validation.json' if namespace else ('mapping_validation.json' if mapping else 'launch_validation.json'))
 expected_profiles=8 if io else 6
 dest=(ROOT/report_path).resolve();base=ROOT/'reviews/v5/expansion'/batch
 if not dest.is_relative_to(base.resolve()):raise ValueError('Use POSIX followthrough evidence directory')
 canonical='curriculum/v5/expansion/legacy_assessments/'+project+'.json'
 current=json.loads((ROOT/canonical).read_text(encoding='utf-8'))
 old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+canonical],cwd=ROOT,text=True,encoding='utf-8'))
 cleaned=copy.deepcopy(current['criteria'])
 for criterion in cleaned:criterion.pop('successor_support',None)
 checks=[{'name':'original-criteria-unchanged','passed':cleaned==old['criteria']},
         {'name':'original-binding-unchanged','passed':current['baseline_binding']==old['baseline_binding']},
         {'name':'original-execution-observation-retained','passed':all(current['execution_support'].get(k)==v for k,v in old['execution_support'].items())},
         {'name':'renderer-hook-idempotent','passed':attach_current(copy.deepcopy(current))==current}]
 campaign=json.loads((base/campaign_file).read_text(encoding='utf-8'));fresh(campaign,expected_outcomes,expected_profiles)
 checks.append({'name':'all'+str(expected_outcomes)+'-source-bound-outcomes','passed':True})
 for name,alter in [('failed-campaign',lambda d:d.update(passed=False)),('missing-outcome',lambda d:d['profiles'][0]['outcomes'].pop()),('stale-source',lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),('stale-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64}))]:
  negative=copy.deepcopy(campaign);alter(negative)
  try:fresh(negative,expected_outcomes,expected_profiles);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 with tempfile.TemporaryDirectory(prefix='plx-standalone-') as tmp:
  work=Path(tmp)/'learner';cli.instantiate(project,work,family_name='posix_lifecycle_depth')
  expected=ROOT/'labs/posix_lifecycle_depth'/subdir;actual=work/subdir
  copied=all((actual/p.name).read_bytes()==p.read_bytes() for p in expected.iterdir() if p.is_file())
  leaks=[str(p.relative_to(work)) for p in work.rglob('*') if p.is_file() and 'assessor' in p.relative_to(work).parts]
  commands=[['gcc','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',str(actual/'fault_helper.c' if mapping else work/'tests/helper.c'),'-o',str(work/'helper')],
            ['gcc','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(actual),str(actual/'tests.c'),str(actual/('io_stub.c' if io else ('path_stub.c' if namespace else ('map_stub.c' if mapping else 'launch_stub.c')))),*(['-Wl,--wrap=pwrite'] if io else (['-Wl,--wrap=syscall'] if namespace else [])),'-o',str(work/'followthrough-tests')]]
  build=[subprocess.run(c,capture_output=True,text=True,timeout=60) for c in commands]
  args=[str(work/'followthrough-tests'),'stream'] if io else ([str(work/'followthrough-tests'),'confined'] if namespace else [str(work/'followthrough-tests'),'views' if mapping else 'channel',str(work/'helper')])
  execution=subprocess.run(args,cwd=work,capture_output=True,text=True,timeout=15) if all(r.returncode==0 for r in build) else None
  checks.append({'name':'standalone-public-export','passed':copied and not leaks,'all_supplied_files_identical':copied,'assessor_files':leaks})
  checks.append({'name':'standalone-unsolved-stub','passed':execution is not None and execution.returncode==1 and ('PL_CHECK:' if mapping or namespace or io else 'PLX_CHECK:') in execution.stderr,
                 'commands':commands,'build_exits':[r.returncode for r in build],'build_output':[r.stdout+r.stderr for r in build],
                 'execution_exit':None if execution is None else execution.returncode,'output':None if execution is None else execution.stdout+execution.stderr})
 paths=[Path(__file__).resolve(),ROOT/canonical,base/campaign_file,ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion'/batch/'author_support.py',ROOT/'research/v5/expansion/legacy_runtime/author_supplements.py']
 paths += [p for p in (ROOT/'labs/posix_lifecycle_depth'/subdir).iterdir() if p.is_file()]
 value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,
        'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
        'scope':'Author source/export guards; no independent reviewer, learner attempt or physical evidence.'}
 dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'checks':len(checks)}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);group=p.add_mutually_exclusive_group();group.add_argument('--mapping',action='store_true',help='Validate PR-T23-03 mapping successor instead of the launcher successor.');group.add_argument('--namespace',action='store_true',help='Validate PR-T23-02 namespace successor.');group.add_argument('--io',action='store_true',help='Validate PR-T23-02 resumable I/O and actual i386 ABI successor.');args=p.parse_args();raise SystemExit(main(args.report,args.mapping,args.namespace,args.io))
