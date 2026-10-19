"""Canonical additive integration and exact bounded T25.01 evidence envelope."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
HERE=ROOT/'research/v5/expansion/legacy_thread_lifecycle';CANON=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T25-01.json'
from research.v5.expansion.legacy_threads.author_support import attach
OBJECTIVES=['T25.01.a','T25.01.b','T25.01.c']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def integrate():
 before=json.loads(CANON.read_text(encoding='utf8'));snapshot=HERE/'PR-T25-01.before-lifecycle.json'
 if not snapshot.exists():snapshot.write_bytes(CANON.read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/thread_depth/lifecycle').rglob('*') if p.is_file())
 write(HERE/'attachment.json',dict(project=before['project'],key='thread_lifecycle_successor',supplied_artifacts=public,shared_contracts=['labs/thread_depth/lifecycle/CONTRACTS.md'],source_register='research/v5/expansion/legacy_thread_lifecycle/resources.json',support='research/v5/expansion/legacy_thread_lifecycle/completion_dispositions.json',criterion_ids=[c['id'] for c in before['criteria'] if c['objective'] in OBJECTIVES],scope='T25.01 actual retained-heap, detached cancellation and interface probes; exact published-edition delayed comparison remains partial.'))
 after=attach(copy.deepcopy(before));assert before['criteria']==after['criteria'] and before['baseline_binding']==after['baseline_binding'];assert attach(copy.deepcopy(after))==after
 write(CANON,after);print('PASS additive T25.01 attachment; original criteria unchanged')
def package():
 d=json.loads(CANON.read_text(encoding='utf8'));before=json.loads((HERE/'PR-T25-01.before-lifecycle.json').read_text(encoding='utf8'));assert before['criteria']==d['criteria'] and before['baseline_binding']==d['baseline_binding'];assert attach(copy.deepcopy(d))==d
 report=json.loads((HERE/'validation.json').read_text(encoding='utf8'));exports=json.loads((HERE/'exports.json').read_text(encoding='utf8'))
 files={CANON,pathlib.Path(__file__),ROOT/'tools/check_thread_lifecycle.py',ROOT/'research/v5/expansion/legacy_threads/author_support.py'}
 files|={p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','attempt1_validation.json','attempt2_validation.json']}
 for r in [report,exports]:
  assert r['status']=='pass' and r['commands'] and all(x['passed'] for x in r['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for p,h in r[key].items():assert sha(ROOT/p)==h,('stale',p)
  files|={ROOT/p for p in r['source_sha256']}
 command_ids=[x['id'] for x in report['commands']];rows=[];receipt='research/v5/expansion/legacy_thread_lifecycle/validation.json'
 analyses=['labs/thread_depth/lifecycle/CONTRACTS.md','assessor/thread_depth/lifecycle/ANALYSIS.md']
 for c in d['criteria']:
  if c['objective'] not in OBJECTIVES:continue
  v=c['variants'][0]
  for delayed in [False,True]:
   partial=delayed and c['objective'].endswith('.c');state='partial' if partial else 'complete'
   checks=[x for x in command_ids if (x.endswith('.pool') or x.endswith('.baseline'))] if c['objective'].endswith('.a') else [x for x in command_ids if x.endswith('.detached')] if c['objective'].endswith('.b') else [x for x in command_ids if '.c17.' in x or '.c23.' in x]
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching',state,analyses[:1]),('implementation',state,analyses),('author_validation','partial' if partial else 'passed',[receipt]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-applicable-to-declared-host-contract',[])]}
   remaining=['Inspect exact published later-edition optionality text before closing the original delayed comparison; draft and compiler evidence do not substitute.'] if partial else []
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=d['project'],objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=analyses,author_validation=dict(report=receipt,checks=checks),closure_rationale={'T25.01.a':'Actual original pl_pool rerun and retained heap successor with every preconsumption join-error ordinal, no repeated join or premature disposal.','T25.01.b':'Actual copied detached job with normal/canceled creator, independent monitor owner and last-reference quiescence.','T25.01.c':'Clean separate C/POSIX type/link/status probes with named wrong-signature diagnostics; inspected drafts remain distinct from missing final text.'}[c['objective']],remaining_author_work=remaining,evidence_limits=report['limits']))
 result=dict(schema_version=1,scope='Only three T25.01 criteria and their exact delayed variants; source-limited delayed .c remains partial',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[receipt,'research/v5/expansion/legacy_thread_lifecycle/exports.json'],updates=rows,remaining_parents=['LRUN-14 still needs T25.02 sharing and exact published comparison; no whole-parent closure.'],summary=dict(criteria=3,delayed_complete=2,delayed_source_partial=1,commands=len(report['commands']),export_commands=len(exports['commands'])))
 write(HERE/'completion_dispositions.json',result);print(result['summary'])
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
