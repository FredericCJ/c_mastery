"""Bind the exact three memory/bus criterion/changed-condition pairs."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_bus'
from research.v5.expansion.legacy_architecture_bus.author_support import attach
PROJECT='PR-T37-06';RECEIPT='research/v5/expansion/legacy_architecture_bus/validation.json';SUPPORT='research/v5/expansion/legacy_architecture_bus/completion_dispositions.json'
ARTIFACTS=['labs/architecture_bus/CONTRACTS.md','assessor/architecture_bus/ANALYSIS.md','assessor/architecture_bus/bus.c','assessor/architecture_bus/oracle.py','labs/architecture_bus/tests.c','assessor/architecture_bus/linked.py','assessor/peripheral_depth/dma/dma.c','labs/architecture_bytes/placement.c','labs/firmware_lifecycle/targets/f439_service.ld']
RATIONALES={'T37.08.a':'Freshly inspected RM0090 Rev21 text and both bus figures, explicitly scoped CPU/DMA data-port graph, actual F407 missing-SRAM3 variant and independent full-extent/master checks.','T37.08.b':'Actual unchanged typed objects/B06 linker and symbol-derived placement, only CPU-local CCM relocation, original bad-DMA link rejection and actual Pd lease producer with FIFO/generation/READY/finalize evidence.','T37.08.c':'Actual bounded burst/wait model with independently derived recurrent and conservative bounds, bridge expansion separated from inspected USART16/32-bit permission, no physical throughput or WCET claim.'}
REMAINING=['PR-T37-07 and PR-T37-08 original and changed mechanisms remain separately required.','Real board/silicon/clock/errata qualification and initialization, physical observation, independent review/delayed learner and final C17/C23 source evidence remain separate.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 patterns={'T37.08.a':['.topology','.placement','.mutation.CCM','.mutation.F407','thumb.'],'T37.08.b':['ledger.','linked.','.placement','.lease','.mutation.END','.mutation.RELEASE','.mutation.EN_ONLY','thumb.'],'T37.08.c':['.arbitration','.bridge','.mutation.FAIR','.mutation.WAIT','.mutation.BRIDGE','.mutation.WIDTH','thumb.']}
 return dict(report=RECEIPT,checks=[n for n in commands if any(p in n for p in patterns[objective])])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def all_rows(r):
 d=read(canon());commands=[c['id'] for c in r['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 return rows
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-bus.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/architecture_bus').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/peripheral_depth/dma/dma.h','labs/peripheral_depth/dma/dma.c','labs/peripheral_depth/dma/CONTRACTS.md','labs/architecture_bytes/placement.c','labs/architecture_bytes/bytes.h','labs/algorithm_wrappers/wrappers.h','labs/containers_depth/containers.h','labs/containers_depth/byte_helpers.c','labs/firmware_lifecycle/targets/f439_service.ld']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T37',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/peripheral_depth/dma/CONTRACTS.md'],source_register='research/v5/expansion/legacy_architecture_bus/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Three exact ordinary/changed memory topology, actual typed placement/lease and bounded arbitration/bridge mechanisms.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive bus attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_architecture_bus.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith(('attempt','root_'))}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-bus.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;rows=all_rows(r);assert len(rows)==6
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Three original T37.08 criterion/V1 pairs',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_architecture_bus/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact bus dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
