"""Bind eighteen exact lifetime, representation and alignment tasks."""
import argparse,copy,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_object_atlas';PROJECT='PR-T03-01';CARD='curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json';REPORT='research/v5/expansion/legacy_object_atlas/validation.json';SUPPORT='research/v5/expansion/legacy_object_atlas/completion_dispositions.json'
from research.v5.expansion.legacy_object_atlas.author_support import attach
ARTIFACTS=['labs/object_followthrough/atlas/CONTRACTS.md','labs/object_followthrough/atlas/atlas.h','labs/object_followthrough/atlas/tests.c','labs/object_followthrough/atlas/tls.c','labs/object_followthrough/atlas/analysis_only.c','assessor/object_followthrough/atlas/atlas.c','assessor/object_followthrough/atlas/oracle.py','assessor/object_followthrough/atlas/ANALYSIS.md']
CASES={'T03.01.a':'duration','T03.01.b':'replacement','T03.01.c':'escape','T03.02.a':'layout','T03.02.b':'authority','T03.02.c':'equality','T03.03.a':'geometry','T03.03.b':'conversion','T03.03.c':'typed'}
WHY={'duration':'Four explicit object timelines plus actual GCC documented TLS cross-thread access with predicate publication/withdrawal and TSan controls.','replacement':'Controlled allocate-copy-free growth/shrink/failure, truthful borrow invalidation and integer history saved before lifetime termination.','escape':'Caller-owned capacity-preserving repair and actual deep-copy identity/mutation counterexample for aggregate views.','layout':'Live unsigned-character inspection, actual field/padding intervals, deliberately different padding and reordered member layout.','authority':'Exact inspected C11/C23-draft representation distinctions plus actual LP64/ILP32 execution and bounded selected binary64 premises.','equality':'Explicit member/NaN/zero equivalence, padding/byte comparison and independently calculated normalized13-byte encoding.','geometry':'Contained-slice checked geometry, no fabricated pointers, independent3690-case candidate enumeration and actual typed-array stride.','conversion':'Analysis-only invalid conversion plus defined byte-offset decoder and inspected actual optimized lowering.','typed':'Separate actual owner/type/capacity/alignment, selected64-aligned type and refusal of weaker max_align_t guarantee.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+CARD],cwd=ROOT,text=True,encoding='utf8'))
def checks(case,delayed):
    if case=='duration':return ['gcc.tls.debug.run','gcc.tls.tsan.run','gcc.tls.debug.mutant.GLOBAL','gcc.tls.debug.mutant.NOWRITE'] if delayed else ['gcc.debug.lifetime','gcc.tls.debug.run']
    if case=='replacement':return ['gcc.debug.lifetime','clang.sanitized.lifetime','gcc.debug.mutant.FAIL']
    if case=='escape':return ['gcc.debug.escape','clang.sanitized.escape','gcc.debug.mutant.BORROW']
    if case=='layout':return ['gcc.debug.representation','clang.optimized.representation','i386.representation']
    if case=='authority':return ['gcc.debug.representation','i386.elf','i386.representation','i386.equality']
    if case=='equality':return ['gcc.debug.equality','clang.sanitized.equality','gcc.debug.mutant.BYTES','gcc.debug.mutant.ZERO','gcc.debug.oracle','i386.oracle']
    if case=='geometry':return ['gcc.debug.alignment','gcc.debug.oracle','i386.oracle','gcc.debug.mutant.SLICE']
    if case=='conversion':return ['gcc.debug.alignment','clang.optimized.alignment','decode.assembly']
    return ['gcc.debug.alignment','i386.alignment','gcc.debug.mutant.WIDE']
def rows(report):
    out=[]
    for c in read(ROOT/CARD)['criteria']:
        v=c['variants'][0];case=CASES[c['objective']]
        for delayed in [False,True]:
            dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:5]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT,'research/v5/expansion/legacy_object_atlas/exports.json']),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
            out.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks(case,delayed),case=case,analysis=ARTIFACTS[-1]+'#'+c['objective'].lower().replace('.','-')),closure_rationale=WHY[case],remaining_author_work=[],evidence_limits=report['limits']))
    return out
def integrate():
    d=read(ROOT/CARD);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'];snapshot=HERE/(PROJECT+'.before-atlas.json')
    if not snapshot.exists():snapshot.write_bytes((ROOT/CARD).read_bytes())
    public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/object_followthrough/atlas').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]+['labs/support/cm_parse.c','labs/support/cm_parse.h']
    put(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T03',key='object_atlas_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_object_atlas/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='All nine exact ordinary and unchanged delayed lifetime/representation/alignment tasks; bounded source analysis, actual native/i386/TLS and independent geometry evidence.'))
    after=attach(copy.deepcopy(d));assert after['criteria']==old['criteria'] and after['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(after))==after;put(ROOT/CARD,after)
    put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],case=CASES[c['objective']],teaching=ARTIFACTS[0],assessment=ARTIFACTS[-1]+'#'+c['objective'].lower().replace('.','-')) for c in d['criteria']],limits=['Canonical card read directly; pure project attachment reapplication through close_object_atlas --integrate.','No external review or independent learner delayed assessment from author runs.']))
    print('PASS nine original criteria/nine delayed tasks preserved; attachment idempotent')
def package():
    import run_object_atlas as run
    reports=[read(HERE/n) for n in ['validation.json','exports.json']];assert reports[0]['source_sha256']==run.pins();files={pathlib.Path(__file__),ROOT/'tools/check_object_atlas.py',ROOT/CARD}
    for r in reports:
        assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
        for key in ['source_sha256','outputs_sha256']:
            for n,h in r[key].items():assert sha(ROOT/n)==h,('stale',n)
        files|={ROOT/n for n in r['source_sha256']}
    files|={p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
    assert read(ROOT/CARD)['criteria']==original()['criteria']
    put(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-T03-01 nine ordinary and nine unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_object_atlas/exports.json'],updates=rows(reports[0]),remaining_parents=['Other T03 projects remain required and receive separate exact implementation evidence.','Exact published C17/C23, external review, independent learner delayed assessment and physical observations remain open.']))
    print('PASS18 exact object atlas dispositions')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
