"""Bind the exact three cache criterion/changed-condition pairs."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_cache'
from research.v5.expansion.legacy_architecture_cache.author_support import attach
PROJECT='PR-T37-05';RECEIPT='research/v5/expansion/legacy_architecture_cache/validation.json';SUPPORT='research/v5/expansion/legacy_architecture_cache/completion_dispositions.json'
ARTIFACTS=['labs/architecture_cache/CONTRACTS.md','assessor/architecture_cache/ANALYSIS.md','assessor/architecture_cache/cache.c','assessor/architecture_cache/oracle.py','labs/architecture_cache/tests.c','assessor/architecture_cache/generate.py','assessor/algorithms/algo.c']
RATIONALES={'T37.07.a':'Complete admitted address/valid/tag and target/shadow snapshots, exact retained direct-map producer comparison, actual two-way LRU and independent list/stack-distance certificates.','T37.07.b':'Two equal-work layout pairs, checked exact rational AMAT/stall/Hz arithmetic, total-versus-additional latency counterexample and actual bounded service with overlapping misses, dependencies and4352-tick endpoint.','T37.07.c':'Pre-access three-C classification against equal-capacity LRU, original capacity witness and actual changed FIFO shadow with the same target hits but a different qualified diagnostic category.'}
REMAINING=['PR-T37-06 through PR-T37-08 original and changed mechanisms remain separately required.','External review, independent seven-day learner, physical observation, exact final C17/C23 and new external textbook inspection remain unclaimed.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 patterns={'T37.07.a':['.ordinary','.geometry','.mutation.VALID','.mutation.WAYS','thumb.'],'T37.07.b':['.layouts','.costs','.overlap','.mutation.UNITS','.mutation.ADDITIONAL','.mutation.OVERLAP','thumb.'],'T37.07.c':['.ordinary','.geometry','.mutation.SHADOW','.mutation.FIFO','thumb.']}
 return dict(report=RECEIPT,checks=[n for n in commands if any(p in n for p in patterns[objective])])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def all_rows(r):
 d=read(canon());commands=[c['id'] for c in r['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 return rows
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-cache.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/architecture_cache').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/algorithms/include/algo.h','labs/algorithms/SPEC.md','labs/containers_depth/byte_helpers.c']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T37',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/algorithms/SPEC.md'],source_register='research/v5/expansion/legacy_architecture_cache/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Three exact ordinary/changed cache residency, dimensional costs and reference-policy mechanisms.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive cache attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_architecture_cache.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith(('attempt','root_'))}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-cache.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;rows=all_rows(r);assert len(rows)==6
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Three original T37.07 criterion/V1 pairs',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_architecture_cache/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact cache dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
