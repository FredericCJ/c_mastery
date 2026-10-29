"""Bind exact original six kernel criteria plus their delayed tasks."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_advanced_kernel'
from research.v5.expansion.legacy_advanced.author_support import attach
PROJECTS=['PR-T26-03'];RECEIPT='research/v5/expansion/legacy_advanced_kernel/validation.json';GUEST='research/v5/expansion/legacy_advanced_kernel/guest_validation.json';SUPPORT='research/v5/expansion/legacy_advanced_kernel/completion_dispositions.json'
ARTIFACTS=['labs/advanced_concurrency_depth/kernel/CONTRACTS.md','assessor/advanced_concurrency_depth/kernel/ANALYSIS.md','assessor/advanced_concurrency_depth/kernel/kernel.c','assessor/advanced_concurrency_depth/kernel/models.py','assessor/advanced_concurrency_depth/kernel/guest/ak_work.c','assessor/kernel_lifecycle/guest/b06_owner.c','assessor/kernel_lifecycle/guest/b06_irq.c','assessor/advanced_concurrency_depth/spsc/spsc.c']
OBJECTIVES=['T26.09.a','T26.09.b','T26.09.c','T26.10.a','T26.10.b','T26.10.c']
RATIONALES={'T26.09.a':'Two actual matching module/kernel builds, exact release/config/image/ABI identity, wrong struct-module ABI rejected without force loading; changed diagnostics audited, not assumed.','T26.09.b':'All560 residual-copy model cases plus actual original T57 cross-protected-page user-copy failure/unchanged payload, copied worker ownership and actual hardirq/threaded contexts.','T26.09.c':'Two actual disposable networkless diskless guest releases with source/image/initramfs/log identities, bounded teardown and original synthetic requests only.','T26.10.a':'Separate explicit ISO publication graph and pinned marked LKMM MP relation fragment with strong/weak controls; separate coherent-DMA and MMIO contract/negative inferences.','T26.10.b':'Exact original ordinary-work fixture replay, timer counterexample and actual external-producer requeue module: gate closure, producer join, synchronous work/timer drain and repeated guest unload.','T26.10.c':'Actual struct as_queue composed with retained copied-input ownership and executable synthetic DMA backend: held request across admission failures, explicit synchronization and quiescence before unmap; physical device evidence remains separate.'}
REMAINING=['All48 original T26 criterion instances now have scoped author packages; current aggregate must validate all six packages before claiming domain author completion.','External review, independent delayed learner, physical device/board and exact final published C17/C23 observations remain separate; no universal kernel/LKMM claim.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T26-03.json'
def selected(d):return [c for c in d['criteria'] if c['objective'] in OBJECTIVES]
def evidence(commands,objective):
 checks=['models']
 if objective=='T26.09.b':checks += [n for n in commands if n.endswith('.copy') and '.learner.' not in n and '.mutation.' not in n]
 if objective in ['T26.10.a','T26.10.c']:checks += [n for n in commands if n.endswith('.backend') and '.learner.' not in n and '.mutation.' not in n]
 e=dict(report=RECEIPT,checks=checks)
 if objective!='T26.10.a':e['additional_reports']=[dict(report=GUEST,checks=['packaged.guest','debug.guest'])]
 return e
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT,GUEST]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 d=read(canon());snapshot=HERE/'PR-T26-03.before-kernel.json'
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/advanced_concurrency_depth/kernel').rglob('*') if p.is_file())+['labs/execution/include/cmx.h','labs/advanced_concurrency_depth/spsc/spsc.h','labs/advanced_concurrency_depth/spsc/learner.c','labs/advanced_concurrency_depth/spsc/CONTRACTS.md','labs/kernel_lifecycle/guest/probe.c','labs/kernel_lifecycle/guest/README.md']
 write(HERE/'attachment_PR-T26-03.json',dict(project='PR-T26-03',domain='T26',key='advanced_kernel_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/advanced_concurrency_depth/spsc/CONTRACTS.md','labs/kernel_lifecycle/guest/README.md'],source_register='research/v5/expansion/legacy_advanced_kernel/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in selected(d)],scope='Original T26.09/.10 ordinary and delayed tasks; precise native/guest/model/device limits, no independent or physical completion.'));after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS original six criteria unchanged; additive kernel attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','guest_validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_advanced_kernel.py',ROOT/'research/v5/expansion/legacy_advanced/author_support.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  for n,h in data.get('external_input_sha256',{}).items():assert sha(pathlib.Path(n))==h,('external',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/'PR-T26-03.before-kernel.json');assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,g,e=reports;commands=[c['id'] for c in r['commands']];rows=[]
 for c in selected(d):
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project='PR-T26-03',objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']+g['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Six exact original criteria and six delayed identities, scoped author implementation/execution only',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,GUEST,'research/v5/expansion/legacy_advanced_kernel/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=6,delayed=6,commands=len(r['commands']),guest_commands=len(g['commands']),export_commands=len(e['commands']))));print('PASS twelve exact kernel-boundary author dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
