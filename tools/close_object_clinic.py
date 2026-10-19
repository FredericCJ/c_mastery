"""Bind three unchanged original and three delayed memory-clinic tasks."""
import argparse,copy,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_object_clinic';PROJECT='PR-T03-05';CARD='curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json';REPORT='research/v5/expansion/legacy_object_clinic/validation.json';SUPPORT='research/v5/expansion/legacy_object_clinic/completion_dispositions.json'
from research.v5.expansion.legacy_object_clinic.author_support import attach
ARTIFACTS=['labs/object_followthrough/clinic/CONTRACTS.md','labs/object_followthrough/clinic/clinic.h','labs/object_followthrough/clinic/shared.h','labs/object_followthrough/clinic/tests.c','labs/object_followthrough/clinic/shared_tests.c','labs/object_followthrough/clinic/history.c','labs/object_followthrough/clinic/defects.c','assessor/object_followthrough/clinic/clinic.c','assessor/object_followthrough/clinic/shared.c','assessor/object_followthrough/clinic/oracle.py','assessor/object_followthrough/clinic/observe.py','assessor/object_followthrough/clinic/ANALYSIS.md']
CASES={'T03.14.a':'repairs','T03.14.b':'containers','T03.14.c':'falsification'}
WHY={'repairs':'Four actual original local diagnostic targets, defined lifetime/copy repairs and a real shared-reference concurrent borrower with separate count/access synchronization.','containers':'Actual whole-object-based offsetof derivation, foreign same-typed member rejection, preserved const and the particular offset-zero initial-member conversion.','falsification':'Actual allocator quarantine versus physical free, two compiler/optimization diagnostic profiles and a serial schedule that fails to expose the otherwise detected payload data race.'}
REMAINING=['Exact published C17/C23 source access remains open.','Author campaign cannot supply independent review, independent delayed learner evidence or physical observation.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+CARD],cwd=ROOT,text=True,encoding='utf8'))
def checks(case,delayed):
    return {'repairs':['gcc.debug.unique','gcc.debug.history.oracle','gcc.defects.O0.observe','clang.defects.O3.observe','gcc.sanitized.shared.late','clang.tsan.shared.parallel','gcc.debug.shared.mutant.RETAIN'],'containers':['gcc.debug.containers','clang.sanitized.containers','gcc.debug.mutant.MEMBER'],'falsification':['gcc.debug.unique','gcc.debug.memcheck.unique','gcc.debug.mutant.OVERLAP','gcc.defects.O0.observe','gcc.defects.O3.observe','clang.defects.O0.observe','clang.defects.O3.observe','gcc.tsan.race.serial','gcc.tsan.race.parallel','clang.tsan.race.serial','clang.tsan.race.parallel']}[case]
def rows(report):
    out=[]
    for c in read(ROOT/CARD)['criteria']:
        v=c['variants'][0];case=CASES[c['objective']]
        for delayed in [False,True]:
            dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:7]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT,'research/v5/expansion/legacy_object_clinic/exports.json']),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
            out.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks(case,delayed),case=case,analysis=ARTIFACTS[-1]+'#'+c['objective'].lower().replace('.','-'),diagnostic_scope='Original UB/race reports are sensitivity evidence, never undefined-value or exploitability oracles.'),closure_rationale=WHY[case],remaining_author_work=[],evidence_limits=report['limits']))
    return out
def integrate():
    d=read(ROOT/CARD);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'];snapshot=HERE/(PROJECT+'.before-clinic.json')
    if not snapshot.exists():snapshot.write_bytes((ROOT/CARD).read_bytes())
    public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/object_followthrough/clinic').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]+['labs/foundation_depth/include/depth.h','labs/foundation_depth/src/depth.c','labs/foundation_depth/CONTRACTS.md']
    put(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T03',key='object_clinic_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/foundation_depth/CONTRACTS.md'],source_register='research/v5/expansion/legacy_object_clinic/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='All three unchanged memory-defect, member-recovery and falsification criteria and delayed tasks, including actual pthread borrower and isolated diagnostic children.'))
    after=attach(copy.deepcopy(d));assert after['criteria']==old['criteria'] and after['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(after))==after;put(ROOT/CARD,after)
    put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],case=CASES[c['objective']],teaching=ARTIFACTS[0],assessment=ARTIFACTS[-1]+'#'+c['objective'].lower().replace('.','-')) for c in d['criteria']],limits=['Canonical card is consumed directly; close_object_clinic --integrate reapplies the pure additive attachment.','Actual fd_move producer is imported unchanged; learner exports its unsolved scaffold, not the private implementation.','Seven-day independent learner assessment remains separate from author runs.']));print('PASS3 unchanged original criteria and delayed tasks; idempotent attachment')
def package():
    import run_object_clinic as run
    reports=[read(HERE/n) for n in ['validation.json','exports.json']];assert reports[0]['source_sha256']==run.pins();files={pathlib.Path(__file__),ROOT/'tools/check_object_clinic.py',ROOT/CARD}
    for r in reports:
        assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
        for key in ['source_sha256','outputs_sha256']:
            for n,h in r[key].items():assert sha(ROOT/n)==h,('stale',n)
        files|={ROOT/n for n in r['source_sha256']}
    files|={p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
    assert read(ROOT/CARD)['criteria']==original()['criteria']
    put(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-T03-05 three original and three unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_object_clinic/exports.json'],updates=rows(reports[0]),remaining_parents=REMAINING));print('PASS6 exact memory-clinic dispositions')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
