"""Versioned actual implementation of twelve Linux capstone pairs."""
import argparse,copy,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_capstone_linux'
from research.v5.expansion.legacy_capstone_linux.author_support import attach
import run_capstone_linux as runner
PROJECTS=[f'PR-CAP-LINUX-{i:02}' for i in range(1,5)];REPORT='research/v5/expansion/legacy_capstone_linux/validation.json';SUPPORT='research/v5/expansion/legacy_capstone_linux/completion_dispositions.json';ANALYSIS='assessor/capstone_followthrough/linux_analysis.md';CONTRACTS=['labs/capstone_followthrough/core/CONTRACTS.md','labs/capstone_followthrough/linux/CONTRACTS.md']
REMAINING=['Independent delayed learner execution/defense and external acceptance remain unperformed.','Physical timing/device observations are separate and unperformed.','Output durability remains an explicitly pending claim in the required evidence matrix; visibility and CRC are not substituted.','The final published C17/C23 text access gap remains.']
# Each pair names specific ordinary and changed-condition mechanisms, not domain-wide test credit.
SPECS={
'T20.01.a':(['gcc.O0.transfer','gcc.O0.core.pressure'],['gcc.O0.modern.slabs','clang.asan.modern.slabs'],'Borrowed-parser counterexample, owned queue and actual two-consumer OwShared slab lifetime with changed storage accounting.'),
'T22.01.b':(['gcc.O0.core.framing','gcc.target.O0.run','clang.target.O3.undefined','gcc.target.O0.leaked-header','gcc.package.static.generated-header-reject'],['gcc.package.static.configure','gcc.package.static.relocated','clang.package.static.relocated'],'Same portable core, poisoned startup, final helper closure and clean adapter-free/exported/header-only consumer.'),
'T35.01.a':(['gcc.O0.core.framing','gcc.O0.linux.file','gcc.O0.core.pressure'],['clang.asan.core.framing','gcc.O0.minimize.authority'],'CRC-valid denial, ordinary output and stalled admission; payload authority is rejected before synthetic effects.'),
'T22.05.b':(['gcc.package.static.consumer','clang.package.static.private-reject'],['gcc.package.shared.relocated','clang.package.shared.dependencies'],'Installed independent68-case consumer, private-symbol rejection and relocated shared runtime policy.'),
'T23.04.a':(['gcc.O0.core.progress','gcc.O0.modern.result'],['gcc.O0.modern.receipts','clang.asan.modern.receipts'],'Exact six-call prefix/result adapter and explicit volatile message deduplication under terminal error; no rollback/durability overclaim.'),
'T24.08.b':(['gcc.O0.linux.fairness','clang.O3.linux.fairness'],['gcc.O0.linux.duplicate','clang.asan.linux.duplicate'],'Real cached fd5/generation reuse, ET software readiness and duplicate-open-file-description registration lifecycle.'),
'T32.07.b':(['gcc.O0.modern.cleanup','clang.asan.modern.cleanup'],['gcc.tsan.modern','clang.tsan.modern'],'Actual five-acquisition reverse cleanup/OwScope and paused worker retention until join/acknowledgement.'),
'T35.06.b':(['gcc.O0.core.pressure','clang.asan.core.pressure'],['gcc.O3.core.pressure'],'Both parser sessions under maximum-frame stall; separate252 backing/metadata/stack and restricted replaceable telemetry eviction.'),
'T21.04.b':(['gcc.O0.core.pressure','clang.asan.core.pressure'],['gcc.asan.core.terminal','clang.asan.core.terminal'],'Exact accepted/delivered/discarded/released conservation; terminal callback may free owner with no later access.'),
'T20.10.c':(['gcc.O0.compose.p5','clang.tsan.compose','gcc.O3.measure','clang.O3.measure'],['gcc.O0.compose.coroutine','clang.asan.compose.coroutine'],'Actual P5 retained chunk leases at capacity1 and allowed actual COAsync alternative with8-byte/synchronous-disarm limits.'),
'T33.10.a':(['evidence','gcc.tsan.race','gcc.O3.measure'],['evidence'],'Property-specific evidence and runtime-change invalidation; original race sensitivity; durability remains explicitly pending.'),
'T35.12.c':(['gcc.O0.core.framing','gcc.O0.core.pressure','gcc.O0.linux.fairness','gcc.O0.core.progress','gcc.O0.minimize.authority','gcc.O0.minimize.retained'],['gcc.O0.transfer','clang.asan.transfer'],'Positive service matrix and actual property-preserving mutation minimizers; changed split/callback/A5 private schedule, not independent learner success.')}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def card(p):return 'curriculum/v5/expansion/legacy_assessments/'+p+'.json'
def original(p):return json.loads(subprocess.check_output(['git','show','HEAD:'+card(p)],cwd=ROOT,text=True,encoding='utf8'))
def public():
    files={p for tree in [runner.PUBLIC/'core',runner.PUBLIC/'linux',*[ROOT/n for n in runner.INCLUDES[2:]]] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
    files|={ROOT/n for n in ['labs/checkpoint_depth/src/checkpoint_depth.c','labs/errors/src/errors.c','labs/ownership/src/ownership.c','labs/coroutines/src/coroutines.c','labs/checkpoint_linux_followthrough/bridge_stub.c','labs/integration/src/integration.c','labs/foundations/src/foundation.c','labs/algorithms/src/algo.c']}
    files|={ROOT/n for n in ['labs/checkpoint_depth/CONTRACTS.md','labs/checkpoint_linux_followthrough/CONTRACTS.md','labs/errors/CONTRACTS.md','labs/ownership/CONTRACTS.md','labs/coroutines/CONTRACTS.md']}
    return sorted(p.relative_to(ROOT).as_posix() for p in files)
def rows(r):
    out=[]
    for p in PROJECTS:
        for c in read(ROOT/card(p))['criteria']:
            normal,changed,why=SPECS[c['objective']];v=c['variants'][0]
            for delayed in [False,True]:
                artifacts=[*CONTRACTS,ANALYSIS,'assessor/capstone_followthrough/core/service.c','assessor/capstone_followthrough/linux/linux.c','assessor/capstone_followthrough/linux/modern.c','labs/capstone_followthrough/linux/composition.c']
                dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',CONTRACTS+[ANALYSIS]),('implementation','complete',artifacts),('author_validation','passed',[REPORT,'research/v5/expansion/legacy_capstone_linux/exports.json']),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
                out.append(dict(id=v['id'] if delayed else c['id'],parent_project=p,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORT,checks=changed if delayed else normal,analysis=ANALYSIS+'#'+c['objective'].lower().replace('.','-')),closure_rationale=why,remaining_author_work=[],evidence_limits=r['limits']))
    return out
def integrate():
    for p in PROJECTS:
        d=read(ROOT/card(p));old=original(p);assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'];snap=HERE/(p+'.before-linux.json')
        if not snap.exists():snap.write_bytes((ROOT/card(p)).read_bytes())
        put(HERE/('attachment_'+p+'.json'),dict(project=p,domain='INTEGRATION',key='capstone_linux_followthrough',supplied_artifacts=public(),shared_contracts=CONTRACTS,source_register='research/v5/expansion/legacy_capstone_linux/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Exact twelve ordinary and twelve delayed Linux capstone tasks, actual native/target-core service, imported modern/P5 adapters and independent installed package tests.'))
        a=attach(copy.deepcopy(d));assert a['criteria']==old['criteria'] and a['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(a))==a;put(ROOT/card(p),a)
    put(HERE/'objective_mapping.json',dict(criteria=[dict(id=c['id'],objective=c['objective'],task=c['task'],variant=c['variants'][0],ordinary_checks=SPECS[c['objective']][0],changed_checks=SPECS[c['objective']][1],assessment=ANALYSIS+'#'+c['objective'].lower().replace('.','-')) for p in PROJECTS for c in read(ROOT/card(p))['criteria']],limits=REMAINING))
    print('PASS4 pure attachments; all exact criteria/baselines preserved')
def package():
    r=read(HERE/'validation.json');e=read(HERE/'exports.json');assert r['source_sha256']==runner.pins() and r['external_sha256']==runner.external();files={pathlib.Path(__file__),ROOT/'tools/check_capstone_linux.py',ROOT/'tools/check_capstone_linux_exports.py',ROOT/ANALYSIS,*[ROOT/card(p) for p in PROJECTS]}
    for report in [r,e]:
        assert report['status']=='pass' and report['commands'] and all(x['passed'] for x in report['commands'])
        for key in ['source_sha256','outputs_sha256']:
            for n,h in report[key].items():assert sha(ROOT/n)==h,('stale',n)
        files|={ROOT/n for n in report['source_sha256']}
    files|={p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
    put(HERE/'completion_dispositions.json',dict(schema_version=1,scope='All12 ordinary and12 named delayed Linux capstone tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_capstone_linux/exports.json'],updates=rows(r),remaining_parents=REMAINING));print('PASS24 exact Linux capstone dispositions')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
