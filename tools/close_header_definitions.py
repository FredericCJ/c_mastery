"""Attach and bind twelve exact retained header/diagnostic tasks."""
import argparse,copy,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_language_headers';PROJECT='PR-T02-04';CARD='curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json';REPORT='research/v5/expansion/legacy_language_headers/validation.json';SUPPORT='research/v5/expansion/legacy_language_headers/completion_dispositions.json'
from research.v5.expansion.legacy_language_headers.author_support import attach
ARTIFACTS=['labs/language_followthrough/headers/CONTRACTS.md','labs/language_followthrough/headers/tests.c','labs/language_followthrough/headers/probes/experiment.h','assessor/language_followthrough/headers/headers.h','assessor/language_followthrough/headers/headers.c','assessor/language_followthrough/headers/symbols.py','assessor/language_followthrough/headers/ANALYSIS.md']
CASES={'T02.11.a':'definitions','T02.11.b':'inline','T02.11.c':'header','T02.12.a':'classification','T02.12.b':'diagnostics','T02.12.c':'analysis'}
WHY={'definitions':'Distinct internal function/static-object identities across separately compiled consumers and exactly one external definition, plus unsigned/unchanged-output controls.','inline':'Address-taking ELF symbols and actual missing/duplicate/repaired links in C17/GNU17/explicit GNU89-inline at O0/O3; unexecuted call-only optimizer counterexample.','header':'First/opposite/repeated includes, actual fresh installed static-library client, broken transitive dependency and repaired self-contained public header.','classification':'Actual syntax/constraint/prototyped-call sources, warning-only translation, permissive dialect comparison and isolated runtime overread.','diagnostics':'Relevant negative source/category checks and repaired positive controls in two compilers and both installed C/C.utf8 locales.','analysis':'Accepted translation with invalid dynamic bound, declared GNU extension, separate sanitizer observation and actual unknown/constrained/repaired analyzer assumptions.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+CARD],cwd=ROOT,text=True,encoding='utf8'))
def checks(case,delayed):
    if case=='definitions':return ['gcc.debug.state','clang.optimized.state','gcc.debug.mutant.SHARED'] if delayed else ['gcc.debug.definitions','clang.sanitized.definitions','gcc.static.O0.symbols','gcc.static.O0.missing','gcc.static.O0.duplicate','gcc.debug.mutant.HELPER','gcc.debug.mutant.OUTPUT']
    if case=='inline':return [cc+'.'+d+'.'+o+'.'+f+'.'+n for cc in ['gcc','clang'] for d in ['c17','gnu17','gnu89-inline'] for o in ['O0','O3'] for f in ['inline','extern'] for n in ['symbols','link']]+([cc+'.c17.O3.callonly.link' for cc in ['gcc','clang']] if delayed else [])
    if case=='header':return ['gcc.header.masked','gcc.header.missing','gcc.header.first','clang.header.installed.run'] if delayed else [cc+'.header.'+n for cc in ['gcc','clang'] for n in ['first','reverse','installed.compile','installed.run']]
    if case=='classification':return [cc+'.permissive.'+n for cc in ['gcc','clang'] for n in ['syntax','redeclaration','arity','type','runtime']]+['gcc.constraint.warning'] if delayed else ['gcc.diagnostic.C.syntax','gcc.diagnostic.C.redeclaration','gcc.runtime.compile','gcc.runtime.valid','gcc.runtime.invalid']
    if case=='diagnostics':return [cc+'.diagnostic.'+loc+'.'+n for cc in ['gcc','clang'] for loc in (['C','C.utf8'] if delayed else ['C']) for n in ['syntax','redeclaration','arity','type','repair']]
    return ['analyzer.capability','analyzer.unknown','analyzer.known','analyzer.repaired'] if delayed else ['gcc.extension.strict','gcc.extension.gnu','clang.extension.strict','clang.extension.gnu','gcc.runtime.valid','gcc.runtime.invalid','clang.runtime.invalid','gcc.debug.definitions']
def rows(report):
    out=[]
    for c in read(ROOT/CARD)['criteria']:
        v=c['variants'][0];case=CASES[c['objective']]
        for delayed in [False,True]:
            dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:3]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT,'research/v5/expansion/legacy_language_headers/exports.json']),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
            out.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks(case,delayed),case=case,analysis=ARTIFACTS[-1]+'#'+c['objective'].lower().replace('.','-')),closure_rationale=WHY[case],remaining_author_work=[],evidence_limits=report['limits']))
    return out
def integrate():
    d=read(ROOT/CARD);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'];snapshot=HERE/(PROJECT+'.before-headers.json')
    if not snapshot.exists():snapshot.write_bytes((ROOT/CARD).read_bytes())
    public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/language_followthrough/headers').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
    put(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T02',key='header_definitions_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_language_headers/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='All six exact ordinary and unchanged delayed header/diagnostic tasks, actual separately compiled C and isolated compiler/runtime/analyzer counterexamples.'))
    after=attach(copy.deepcopy(d));assert after['criteria']==old['criteria'] and after['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(after))==after;put(ROOT/CARD,after)
    put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],case=CASES[c['objective']],teaching=ARTIFACTS[0],assessment=ARTIFACTS[-1]+'#'+c['objective'].lower().replace('.','-')) for c in d['criteria']],limits=['The canonical card is read directly by ordinary builds; this pure attachment is rerun through close_header_definitions --integrate. No unrelated generator ownership is asserted.','Finite author executions do not supply independent learner delayed attempts.']))
    print('PASS exact six criteria and six delayed tasks preserved; attachment idempotent')
def package():
    import run_header_definitions as run
    reports=[read(HERE/n) for n in ['validation.json','exports.json']];assert reports[0]['source_sha256']==run.pins()
    files={pathlib.Path(__file__),ROOT/'tools/check_header_definitions.py',ROOT/CARD}
    for r in reports:
        assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
        for key in ['source_sha256','outputs_sha256']:
            for n,h in r[key].items():assert sha(ROOT/n)==h,('stale',n)
        files|={ROOT/n for n in r['source_sha256']}
    files|={p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
    assert read(ROOT/CARD)['criteria']==original()['criteria']
    put(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-T02-04 six ordinary and six unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_language_headers/exports.json'],updates=rows(reports[0]),remaining_parents=['Exact final published C17/C23 source access remains open.','External review, independent learner delayed assessment and physical observations remain separate.']))
    print('PASS12 exact header/diagnostic dispositions')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
