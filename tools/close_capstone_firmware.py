"""Bind all twelve original MCU ordinary/delayed tasks to exact current evidence."""
import argparse,copy,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_capstone_firmware'
from research.v5.expansion.legacy_capstone_firmware.author_support import attach
import run_capstone_firmware as runner
PROJECTS=[f'PR-CAP-MCU-{i:02}' for i in range(1,5)];REPORT='research/v5/expansion/legacy_capstone_firmware/validation.json';SUPPORT='research/v5/expansion/legacy_capstone_firmware/completion_dispositions.json';ANALYSIS='assessor/capstone_followthrough/firmware_analysis.md';CONTRACTS=['labs/capstone_followthrough/core/CONTRACTS.md','labs/capstone_followthrough/firmware/CONTRACTS.md','labs/capstone_followthrough/firmware/BOARD_CAPTURE.md']
REMAINING=['Physical F439 execution, deployed-byte verification, electrical timing/reset and board qualification remain required and unperformed.','Independent seven-day delayed learner defense and external acceptance remain unperformed.','Final published C17/C23 text access remains a source gap.','Signature verification and flash programming are outside the explicitly trusted-verdict/RAM-cut model; no physical persistence or cryptographic result is inferred.']
SPECS={
'T20.01.b':(['gcc.O0.reference.0','gcc.O0.representation','gcc.result-default-reject','gcc.result-header','gcc.result-full.undefined'],['gcc.O0.reference.1','clang.asan.reference.1'],'Actual P3/P4/Pd/Cs interfaces, equal host result/plain comparison and explicit target dependency audit; late DMA retains memory until matching drain.'),
'T30.07.b':(['gcc.O0.reference.4','gcc.O0.normal.size','clang.O3.f439.size'],['gcc.O3.reference.4','gcc.O0.normal.symbols','clang.O3.normal.symbols'],'Exact1896/2056 admission plus per-image static/frame/stack ledger; Xf basic versus extended/lazy FP and explicit nesting/ABI assumptions.'),
'T35.01.a':(['gcc.O0.reference.0','gcc.O0.authority.0','clang.asan.reference.2'],['clang.O3.reference.0','gcc.O0.transfer'],'Safe-state transitions, trusted authorization and explicit replay sequence/value policy retained across recovery; CRC is not authority.'),
'T27.10.b':(['gcc.O0.normal.run','gcc.O0.copy.run','gcc.O0.bss.run','gcc.O0.vector.binary','gcc.O0.f439.link'],['gcc.O0.reorder.run','clang.O3.reorder.run'],'Actual poisoned reset, copy/BSS/vector controls and actual imported F439 startup cross-build; section reorder/aligned initializer recomputes ranges.'),
'T28.11.b':(['gcc.O0.reference.0','gcc.O0.reference.1','gcc.O0.normal.run','gcc.O0.f439.symbols','clang.O3.f439.undefined'],['gcc.O0.reference.7','gcc.O0.status.7'],'Host and actual selected struct lr_cpu service sample-path byte equivalence; actual P4/Pd F439 composition cross-built, physical execution pending; original W1C/read-clear RMW repair.'),
'T31.08.b':(['gcc.O0.normal.release','gcc.O0.f439.release'],['gcc.O0.normal.debug.binary','gcc.O0.normal.release','gcc.O0.f439.debug.binary','gcc.O0.f439.release'],'Actual ELF-derived binary and source/map/vector dossier; stale bundle refuses; stripped bytes, debuglink CRC and external symbol hash bind association; physical protocol remains open.'),
'T31.09.b':(['gcc.O0.reference.3','gcc.O0.trust.3'],['clang.asan.reference.3'],'All ordinary cut positions and trusted-verdict/compatibility admission; two-part torn markers and bounded serial wrap/half-range ambiguity.'),
'T31.10.c':(['gcc.O0.reference.2','gcc.O0.watchdog.2','gcc.O0.transfer'],['clang.asan.reference.2'],'Epoch masks1,3,2,3, two recovery attempts, retained first fault and late lease; all crash-record cuts distinguish incomplete capture from no fault.'),
'T35.08.c':(['gcc.O0.reference.5','gcc.O0.normal.run','gcc.O0.mpu.run'],['clang.O3.normal.run','clang.O3.reference.5'],'Live pointer defeats C module isolation; actual unprivileged MPU denial and SVC fixed-mailbox copy/privilege transition; struct lr_cpu permission is not DMA protection.'),
'T30.10.c':(['gcc.O0.reference.6','gcc.O0.normal.release'],['clang.asan.reference.6','gcc.O0.f439.release'],'Actual struct cpf_clock/modulo helper,12 versus65548 ticks and rational16/8MHz conversion; explicit interval/epoch and physical timing limitations.'),
'T33.10.c':(['gcc.O0.normal.release','gcc.O0.f439.release'],['gcc.O0.normal.release'],'Property-specific evidence matrix rejects eight substitutions and invalidates DMA claims on a struct lr_cpu-only emulator; labels are not evidence verification.'),
'T35.12.b':(['gcc.O0.reference.0','gcc.O0.reference.1','gcc.O0.reference.2','gcc.O0.reference.3','gcc.O0.reference.4','gcc.O0.transfer'],['clang.asan.transfer'],'Sealed composed malformed/authority/budget/lifetime/update/recovery review; changed A5 payload and completion order retain irreversible prefix/first fault with named original delay.')}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def card(p):return 'curriculum/v5/expansion/legacy_assessments/'+p+'.json'
def original(p):return json.loads(subprocess.check_output(['git','show','HEAD:'+card(p)],cwd=ROOT,text=True,encoding='utf8'))
def public():
    files={p for tree in [runner.PUBLIC,*[ROOT/n for n in runner.INCLUDES[1:]],ROOT/'labs/errors/include'] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
    files|={ROOT/n for n in ['labs/checkpoint_depth/src/checkpoint_depth.c','labs/checkpoint_depth/src/checkpoint_firmware.c','labs/checkpoint_depth/CONTRACTS.md','labs/errors/src/errors.c','labs/errors/CONTRACTS.md','labs/checkpoint_firmware_followthrough/CONTRACTS.md','fixtures/integration/irq.json','fixtures/legacy_capstones/boundaries.json','fixtures/legacy_capstones/firmware_timing_matrix.json']}
    return sorted(p.relative_to(ROOT).as_posix() for p in files)
def rows(r):
    out=[]
    for p in PROJECTS:
        for c in read(ROOT/card(p))['criteria']:
            normal,changed,why=SPECS[c['objective']];v=c['variants'][0]
            for delayed in [False,True]:
                artifacts=[*CONTRACTS,ANALYSIS,'assessor/capstone_followthrough/firmware/firmware.c','assessor/capstone_followthrough/firmware/target.c','assessor/capstone_followthrough/firmware/f439_adapter.c','labs/capstone_followthrough/firmware/checks.c','labs/capstone_followthrough/firmware/evidence.py','fixtures/legacy_capstones/firmware_timing_matrix.json']
                dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',CONTRACTS+[ANALYSIS]),('implementation','complete',artifacts),('author_validation','passed',[REPORT,'research/v5/expansion/legacy_capstone_firmware/exports.json']),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
                out.append(dict(id=v['id'] if delayed else c['id'],parent_project=p,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORT,checks=changed if delayed else normal,analysis=ANALYSIS+'#'+c['objective'].lower().replace('.','-')),closure_rationale=why,remaining_author_work=[],evidence_limits=r['limits']))
    return out
def integrate():
    for p in PROJECTS:
        d=read(ROOT/card(p));old=original(p);assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'];snap=HERE/(p+'.before-firmware.json')
        if not snap.exists():snap.write_bytes((ROOT/card(p)).read_bytes())
        put(HERE/('attachment_'+p+'.json'),dict(project=p,domain='INTEGRATION',key='capstone_firmware_followthrough',supplied_artifacts=public(),shared_contracts=CONTRACTS,source_register='research/v5/expansion/legacy_capstone_firmware/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Exact twelve ordinary and twelve delayed MCU capstone tasks, native models, actual selected struct lr_cpu/startup/MPU execution, actual P4/Pd F439 cross-build, release and later physical protocol.'))
        a=attach(copy.deepcopy(d));assert a['criteria']==old['criteria'] and a['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(a))==a;put(ROOT/card(p),a)
    put(HERE/'objective_mapping.json',dict(criteria=[dict(id=c['id'],objective=c['objective'],task=c['task'],variant=c['variants'][0],ordinary_checks=SPECS[c['objective']][0],changed_checks=SPECS[c['objective']][1],assessment=ANALYSIS+'#'+c['objective'].lower().replace('.','-')) for p in PROJECTS for c in read(ROOT/card(p))['criteria']],limits=REMAINING));print('PASS4 pure attachments; exact criteria/baselines preserved')
def package():
    r=read(HERE/'validation.json');e=read(HERE/'exports.json');assert r['source_sha256']==runner.pins() and r['external_sha256']==runner.external();files={pathlib.Path(__file__),ROOT/'tools/check_capstone_firmware.py',ROOT/'tools/check_capstone_firmware_exports.py',ROOT/ANALYSIS,*[ROOT/card(p) for p in PROJECTS]}
    for report in [r,e]:
        assert report['status']=='pass' and report['commands'] and all(x['passed'] for x in report['commands'])
        for key in ['source_sha256','outputs_sha256']:
            for n,h in report[key].items():assert sha(ROOT/n)==h,('stale',n)
        files|={ROOT/n for n in report['source_sha256']}
    files|={p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
    put(HERE/'completion_dispositions.json',dict(schema_version=1,scope='All12 ordinary and12 named delayed MCU capstone tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_capstone_firmware/exports.json'],updates=rows(r),remaining_parents=REMAINING));print('PASS24 exact MCU capstone dispositions')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
