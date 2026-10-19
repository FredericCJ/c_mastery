"""Preserve and bind eight exact P2 writer ordinary/delayed identities."""
import argparse,copy,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_checkpoint_writer';PROJECT='PR-P2-02';CARD='curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json';REPORT='research/v5/expansion/legacy_checkpoint_writer/validation.json';SUPPORT='research/v5/expansion/legacy_checkpoint_writer/completion_dispositions.json'
from research.v5.expansion.legacy_checkpoint_writer.author_support import attach
ARTIFACTS=['labs/checkpoint_followthrough/writer/CONTRACTS.md','labs/checkpoint_followthrough/writer/writer.h','labs/checkpoint_followthrough/writer/tests.c','labs/checkpoint_followthrough/writer/progress.c','assessor/checkpoint_followthrough/writer/writer.c','assessor/checkpoint_followthrough/writer/target.c','assessor/integration/integration.c','assessor/checkpoint_depth/checkpoint_writer.c','assessor/checkpoint_depth/transfer/adapters.c','assessor/errors/errors.c','assessor/checkpoint_followthrough/writer_analysis.md']
WHY={'T10.07.b':'Exact remaining/offset traces, bounded zero/retry turns and three actual nonblocking readiness cycles per raw/libc backend.','T23.04.c':'Actual named InTransfer queue ownership, explicit terminal recovery, observed65/66/65 duplicate mutant and copied-span suffix retention.','T23.04.b':'Separate raw/script/libc numeric translation, actual raw/libc selected OS observations, scripted EINTR and strict/hosted dependency separation.','T21.02.b':'Bounded stable record with first/cleanup causes, logger-independent core and actual er_transfer composition under equal traces with measured sizes/static branches/callbacks/copies.'}
REMAINING=['All P3 tasks remain required under their separate exact-task packet.','Final published ISO source, external review, independent delayed learner and physical observations remain separate.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def original():
    d=json.loads(subprocess.check_output(['git','show','HEAD:'+CARD],cwd=ROOT,text=True,encoding='utf8'))
    for c in d['criteria']:c['successor_support']=dict(path='research/v5/expansion/legacy_completion/checkpoint_support.json',id=c['id'])
    return d
def checks(objective,delayed):
    tags={'T10.07.b':['gcc.O0.progress','clang.O3.progress','gcc.O0.mutant.short','gcc.O0.mutant.retry'],'T23.04.c':['gcc.O0.byte','gcc.O0.span','clang.O3.strict.run','gcc.O0.mutant.requeue','gcc.O0.mutant.suffix'],'T23.04.b':['gcc.O0.progress','clang.O3.progress','gcc.O0.strict.nm'],'T21.02.b':['gcc.O0.progress','gcc.O0.log','clang.O3.shapes','gcc.O0.mutant.cause','gcc.O0.mutant.product']}
    return tags[objective]
def rows(r):
    out=[]
    for c in read(ROOT/CARD)['criteria']:
        v=c['variants'][0]
        for delayed in [False,True]:
            dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:4]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT,'research/v5/expansion/legacy_checkpoint_writer/exports.json']),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
            out.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks(c['objective'],delayed),analysis=ARTIFACTS[-1]+'#'+c['objective'].lower().replace('.','-')),closure_rationale=WHY[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
    return out
def integrate():
    d=read(ROOT/CARD);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'];snapshot=HERE/(PROJECT+'.before-writer.json')
    if not snapshot.exists():snapshot.write_bytes((ROOT/CARD).read_bytes())
    public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/checkpoint_followthrough/writer').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
    for directory in ['labs/integration/include','labs/foundations/include','labs/algorithms/include']:public += [p.relative_to(ROOT).as_posix() for p in sorted((ROOT/directory).rglob('*.h'))]
    public+=['labs/checkpoint_depth/include/checkpoint_writer.h','labs/checkpoint_depth/src/checkpoint_writer.c','labs/checkpoint_depth/transfer/adapters.h','labs/checkpoint_depth/transfer/adapters_stub.c','labs/errors/include/errors.h','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h']
    put(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='INTEGRATION',key='checkpoint_writer_followthrough',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/checkpoint_depth/transfer/CONTRACTS.md'],source_register='research/v5/expansion/legacy_checkpoint_writer/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='All four original and delayed writer tasks, including actual named InTransfer composition.'))
    after=attach(copy.deepcopy(d));assert after['criteria']==old['criteria'] and after['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(after))==after;put(ROOT/CARD,after)
    put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching=ARTIFACTS[0],checks=checks(c['objective'],False),assessment=ARTIFACTS[-1]+'#'+c['objective'].lower().replace('.','-')) for c in d['criteria']],limits=['Original criteria and historical bindings remain byte-equivalent objects.','Current implementation does not confer a learner or independent-review result.']));print('PASS4 exact original/delayed criteria preserved; idempotent attachment')
def package():
    import run_checkpoint_writer_followthrough as run
    reports=[read(HERE/n) for n in ['validation.json','exports.json']];assert reports[0]['source_sha256']==run.pins();files={pathlib.Path(__file__),ROOT/'tools/check_checkpoint_writer_followthrough.py',ROOT/CARD,ROOT/'research/v5/expansion/legacy_checkpoints/author_supplements.py',ROOT/ARTIFACTS[-1]}
    for r in reports:
        assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
        for key in ['source_sha256','outputs_sha256']:
            for n,h in r[key].items():assert sha(ROOT/n)==h,('stale',n)
        files|={ROOT/n for n in r['source_sha256']}
    files|={p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
    put(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-P2-02 all four ordinary and four delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_checkpoint_writer/exports.json'],updates=rows(reports[0]),remaining_parents=REMAINING));print('PASS8 exact P2 writer dispositions')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
