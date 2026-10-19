"""Preserve and bind all six exact P2 entry criterion/delayed task identities."""
import argparse,copy,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_checkpoint_entry';PROJECT='PR-P2-01';CARD='curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json';REPORT='research/v5/expansion/legacy_checkpoint_entry/validation.json';SUPPORT='research/v5/expansion/legacy_checkpoint_entry/completion_dispositions.json'
from research.v5.expansion.legacy_checkpoint_entry.author_support import attach
ARTIFACTS=['labs/checkpoint_followthrough/entry/CONTRACTS.md','labs/checkpoint_followthrough/entry/entry.h','labs/checkpoint_followthrough/entry/check_image.py','labs/checkpoint_followthrough/entry/elf_audit.py','assessor/checkpoint_followthrough/entry/image.c','assessor/checkpoint_depth/targets/entry.S','assessor/checkpoint_depth/targets/byte_helpers.c','assessor/checkpoint_depth/checkpoint_writer.c','labs/checkpoint_depth/tests/helper_probe.c','assessor/checkpoint_followthrough/entry_analysis.md']
CASES={'T09.01.b':'elf','T10.01.c':'entry','T11.01.c':'closure'}
WHY={'elf':'Actual relocatable object/final image, zero/initialized sentinels, strict PT_LOAD and separate fixed/PIE data-and-entry load-bias observations.','entry':'Actual Linux-created argc/argv/env/auxv consumer invokes the unchanged portable writer, changing output with the selected delayed input rather than an ignored placeholder.','closure':'Actual GCC/Clang O0/O3 object and final strict links,65536-byte aggregate helper, missing-provider refusal and fully resolved forbidden-provider/unhandled-relocation controls.'}
REMAINING=['PR-P2-02 and all P3 exact tasks remain required under separate packages.','Published final ISO, external review, independent delayed learner and physical evidence remain separate.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def original():
    d=json.loads(subprocess.check_output(['git','show','HEAD:'+CARD],cwd=ROOT,text=True,encoding='utf8'))
    # The already-authored49-criterion successor added only this exact pointer
    # to each original criterion. Preserve it and every original field.
    for c in d['criteria']:c['successor_support']=dict(path='research/v5/expansion/legacy_completion/checkpoint_support.json',id=c['id'])
    return d
def checks(case):
    return {'elf':['gcc.O0.fixed.image.object','gcc.O0.fixed.audit','gcc.O0.fixed.execute','gcc.O0.pie.execute','clang.O3.pie.execute','gcc.O3.pie.relocation.audit','gcc.O0.fixed.bad-entry.audit','gcc.O0.fixed.bad-extent.audit'],'entry':['gcc.O0.fixed.assembly','gcc.O0.fixed.execute','clang.O3.pie.assembly','clang.O3.pie.execute'],'closure':['gcc.O0.fixed.aggregate.object','gcc.O0.fixed.missing-helper','clang.O3.fixed.missing-helper','gcc.O3.fixed.forbidden.audit','clang.O0.pie.audit','clang.O3.pie.audit']}[case]
def rows(report):
    result=[]
    for c in read(ROOT/CARD)['criteria']:
        v=c['variants'][0];case=CASES[c['objective']]
        for delayed in [False,True]:
            dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:4]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT,'research/v5/expansion/legacy_checkpoint_entry/exports.json']),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
            result.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks(case),case=case,analysis=ARTIFACTS[-1]+'#'+c['objective'].lower().replace('.','-')),closure_rationale=WHY[case],remaining_author_work=[],evidence_limits=report['limits']))
    return result
def integrate():
    d=read(ROOT/CARD);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'];snapshot=HERE/(PROJECT+'.before-entry.json')
    if not snapshot.exists():snapshot.write_bytes((ROOT/CARD).read_bytes())
    public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/checkpoint_followthrough/entry').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]+['labs/checkpoint_depth/include/checkpoint_writer.h','labs/checkpoint_depth/src/checkpoint_writer.c','labs/checkpoint_depth/tests/helper_probe.c','labs/checkpoint_depth/tests/resolved_forbidden.c']
    put(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='INTEGRATION',key='checkpoint_entry_followthrough',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_checkpoint_entry/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='All three exact original and delayed entry/ELF/helper criteria, including actual changed-input semantics.'))
    after=attach(copy.deepcopy(d));assert after['criteria']==old['criteria'] and after['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(after))==after;put(ROOT/CARD,after)
    put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],case=CASES[c['objective']],teaching=ARTIFACTS[0],assessment=ARTIFACTS[-1]+'#'+c['objective'].lower().replace('.','-')) for c in d['criteria']],limits=['Actual x86-64 ABI boundary and real changed arguments, not another architecture.','Inherited source/current receipts remain historical and are not rewritten.','Pure attachment preserves baseline and full original criterion objects.']));print('PASS3 exact original/delayed criteria preserved; attachment idempotent')
def package():
    import run_checkpoint_entry_followthrough as run
    reports=[read(HERE/n) for n in ['validation.json','exports.json']];assert reports[0]['source_sha256']==run.pins();files={pathlib.Path(__file__),ROOT/'tools/check_checkpoint_entry_followthrough.py',ROOT/CARD,ROOT/'research/v5/expansion/legacy_checkpoints/author_supplements.py',ROOT/ARTIFACTS[-1]}
    for r in reports:
        assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
        for key in ['source_sha256','outputs_sha256']:
            for n,h in r[key].items():assert sha(ROOT/n)==h,('stale',n)
        files|={ROOT/n for n in r['source_sha256']}
    files|={p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
    put(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-P2-01 all three ordinary and three delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_checkpoint_entry/exports.json'],updates=rows(reports[0]),remaining_parents=REMAINING));print('PASS6 exact P2 entry dispositions')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
