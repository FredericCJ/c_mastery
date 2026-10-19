"""Bind all eighteen exact retained nested-array/cursor/conversion tasks."""
import argparse,copy,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_object_cursor';PROJECT='PR-T03-02';CARD='curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json';REPORT='research/v5/expansion/legacy_object_cursor/validation.json';SUPPORT='research/v5/expansion/legacy_object_cursor/completion_dispositions.json'
from research.v5.expansion.legacy_object_cursor.author_support import attach
ARTIFACTS=['labs/object_followthrough/cursor/CONTRACTS.md','labs/object_followthrough/cursor/cursor.h','labs/object_followthrough/cursor/tests.c','labs/object_followthrough/cursor/probe.c','labs/object_followthrough/cursor/analysis_only.md','labs/object_followthrough/cursor/loader.h','labs/object_followthrough/cursor/loader_tests.c','labs/object_followthrough/cursor/plugin.c','labs/object_followthrough/cursor/target.c','assessor/object_followthrough/cursor/cursor.c','assessor/object_followthrough/cursor/loader.c','assessor/object_followthrough/cursor/function_lookup.c','assessor/object_followthrough/cursor/oracle.py','assessor/foundation_depth/depth.c','labs/foundation_depth/CONTRACTS.md','assessor/object_followthrough/cursor/ANALYSIS.md']
CASES={'T03.04.a':'rows','T03.04.b':'bounds','T03.04.c':'accessor','T03.05.a':'forward','T03.05.b':'reverse','T03.05.c':'order','T03.06.a':'nullable','T03.06.b':'history','T03.06.c':'loader'}
WHY={'rows':'Actual nested int rows and separately allocated int-pointer table, separate coordinate bounds and allocation cleanup.','bounds':'Named containing-row arithmetic proof and whole-outer-object character inspection; invalid flattening never executed.','accessor':'Actual callbacks for two representations and ragged-row refusal, with required compile-only type mismatch.','forward':'Direct composition of existing fd_take/fd_subview, unchanged failure state, null empty and explicitly permitted parent/output aliasing.','reverse':'Integer admission before pointer formation, valid empty/end distinctions and actual reverse begin boundary.','order':'Same-array representable subtraction and independent logical order across fragmented lengths with safe metadata-overflow refusal.','nullable':'Null-constant constraint comparison and explicit owner-relative four-byte nullable index; no raw pointer serialization.','history':'Actual object/void and optional uintptr roundtrips, distinct allocated replacement and pre-release integer history only.','loader':'Actual original Linux modules, explicit function-address boundary, typed-table ABI/version checks, counted borrows and close-failure retained ownership.'}
REMAINING=['Other T03 projects remain required and receive separate exact implementation evidence.','Exact published C17/C23 and direct OpenGroup pages remain source gaps; external review, independent learner and physical evidence are not supplied by these runs.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+CARD],cwd=ROOT,text=True,encoding='utf8'))
def checks(case,delayed):
    return {'rows':['gcc.debug.allocated' if delayed else 'gcc.debug.matrix','clang.sanitized.matrix','gcc.debug.mutant.ROW'],'bounds':['gcc.debug.matrix','i386.matrix'],'accessor':['gcc.debug.matrix','gcc.debug.allocated','gcc.negative.table','clang.negative.table'],'forward':['gcc.debug.cursors','gcc.debug.oracle','i386.oracle','gcc.debug.mutant.FD_CURSOR'],'reverse':['gcc.debug.cursors','clang.sanitized.oracle','gcc.debug.mutant.REVERSE'],'order':['gcc.debug.order','i386.order','gcc.debug.mutant.FRAGMENT'],'nullable':['gcc.debug.conversions','i386.conversions','gcc.negative.zero','clang.negative.zero','gcc.debug.mutant.NULL'],'history':['gcc.debug.conversions','clang.sanitized.conversions','i386.conversions'],'loader':['gcc.debug.loader.reference','clang.sanitized.loader.reference','gcc.debug.loader.failure','clang.sanitized.loader.failure','gcc.debug.loader.CLOSE','gcc.debug.loader.RETURN','gcc.negative.function','clang.negative.function']}[case]
def rows(report):
    out=[]
    for c in read(ROOT/CARD)['criteria']:
        v=c['variants'][0];case=CASES[c['objective']]
        for delayed in [False,True]:
            dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:9]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT,'research/v5/expansion/legacy_object_cursor/exports.json']),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
            out.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks(case,delayed),case=case,analysis=ARTIFACTS[-1]+'#'+c['objective'].lower().replace('.','-')),closure_rationale=WHY[case],remaining_author_work=[],evidence_limits=report['limits']))
    return out
def integrate():
    d=read(ROOT/CARD);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'];snapshot=HERE/(PROJECT+'.before-cursor.json')
    if not snapshot.exists():snapshot.write_bytes((ROOT/CARD).read_bytes())
    public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/object_followthrough/cursor').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]+['labs/foundation_depth/include/depth.h','labs/foundation_depth/src/depth.c','labs/foundation_depth/CONTRACTS.md']
    put(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T03',key='object_cursor_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/foundation_depth/CONTRACTS.md'],source_register='research/v5/expansion/legacy_object_cursor/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='All nine exact ordinary and unchanged delayed nested-array/cursor/conversion tasks; strict core and separately declared actual Linux loader comparison.'))
    after=attach(copy.deepcopy(d));assert after['criteria']==old['criteria'] and after['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(after))==after;put(ROOT/CARD,after)
    put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],case=CASES[c['objective']],teaching=ARTIFACTS[0],assessment=ARTIFACTS[-1]+'#'+c['objective'].lower().replace('.','-')) for c in d['criteria']],limits=['Canonical card is consumed directly; close_object_cursor --integrate reapplies the pure project attachment.','Actual foundation source imported; no reverse dependency on T69 or silent profile C loader permission.','No independent learner or external review from author runs.']))
    print('PASS nine original criteria and delayed tasks unchanged; attachment idempotent')
def package():
    import run_object_cursor as run
    reports=[read(HERE/n) for n in ['validation.json','exports.json']];assert reports[0]['source_sha256']==run.pins();files={pathlib.Path(__file__),ROOT/'tools/check_object_cursor.py',ROOT/CARD}
    for r in reports:
        assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
        for key in ['source_sha256','outputs_sha256']:
            for n,h in r[key].items():assert sha(ROOT/n)==h,('stale',n)
        files|={ROOT/n for n in r['source_sha256']}
    files|={p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
    assert read(ROOT/CARD)['criteria']==original()['criteria']
    put(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-T03-02 nine ordinary and nine unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_object_cursor/exports.json'],updates=rows(reports[0]),remaining_parents=REMAINING));print('PASS18 exact cursor dispositions')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
