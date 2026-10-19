"""Additive maintained attachment and bounded T25 consume/index evidence packet."""
import argparse
import copy
import hashlib
import json
import pathlib
import sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_threads.author_support import attach
from run_consume_index_depth import HERE,PUBLIC,pins
CANON=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T25-03.json'
ATTACH=ROOT/'research/v5/expansion/legacy_thread_consume_index/attachment.json'
SUPPORT='research/v5/expansion/consume_index_depth/completion_dispositions.json'
REPORT='research/v5/expansion/consume_index_depth/validation.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def integrate():
    old=read(CANON);snapshot=HERE/'PR-T25-03.before-index.json'
    if not snapshot.exists():snapshot.write_bytes(CANON.read_bytes())
    row=dict(project='PR-T25-03',key='thread_consume_index_successor',
             supplied_artifacts=[p.relative_to(ROOT).as_posix() for p in sorted(PUBLIC.iterdir()) if p.is_file()],
             shared_contracts=['labs/consume_index_depth/CONTRACTS.md'],
             source_register='research/v5/expansion/legacy_thread_atomics/resources.json',support=SUPPORT,
             criterion_ids=['AX-PR-T25-03-T25.08.c'],
             scope='Exact branch-selected versus value-derived index delayed specimen and finite dependency graph; immutable-data tests and target object audits. Exact final-edition source comparison remains partial.')
    write(ATTACH,row);new=attach(copy.deepcopy(old))
    assert new['criteria']==old['criteria'] and new['baseline_binding']==old['baseline_binding']
    assert attach(copy.deepcopy(new))==new
    write(CANON,new)
def package():
    report=read(HERE/'validation.json');assert report['status']=='pass' and report['source_sha256']==pins()
    guard=ROOT/'reviews/v5/expansion/consume_index_depth/support_validation.json';g=read(guard)
    assert g['status']=='pass' and g['campaign']==sha(HERE/'validation.json')
    canonical=read(CANON);assert attach(copy.deepcopy(canonical))==canonical
    old=read(HERE/'PR-T25-03.before-index.json');assert old['criteria']==canonical['criteria'] and old['baseline_binding']==canonical['baseline_binding']
    exports=ROOT/'research/v5/expansion/legacy_thread_atomics/exports.json';e=read(exports)
    assert e['status']=='pass'
    for path,h in e['source_sha256'].items():assert sha(ROOT/path)==h,('stale export',path)
    artifacts=['labs/consume_index_depth/CONTRACTS.md','assessor/consume_index_depth/ANALYSIS.md','assessor/consume_index_depth/consume_index.c','assessor/consume_index_depth/oracle.py']
    dimensions={k:dict(state=state,evidence=ev) for k,state,ev in [
        ('teaching','partial-exact-reference-unavailable',artifacts[:2]),
        ('implementation','complete',artifacts),('author_validation','partial-exact-reference-unavailable',[REPORT]),
        ('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-applicable-source-analysis',[])]}
    criterion=next(c for c in canonical['criteria'] if c['id']=='AX-PR-T25-03-T25.08.c')
    rows=[]
    for ident in [criterion['id'],criterion['variants'][0]['id'],'LRUN-16']:
        rows.append(dict(id=ident,dimensions=copy.deepcopy(dimensions),artifacts=artifacts,
                         closure_rationale='The previously missing literal branch-selected-index source now exists beside value-derived, killed and acquire comparisons. Six C17 host profiles, three semantic controls,729 independent finite graphs each, four M4 object/link audits and an isolated public export passed. These close the bounded author mechanism, not the exact published C17/C23 comparison.',
                         author_validation=dict(report=REPORT,checks=[x['id'] for x in report['commands']]),
                         remaining_author_work=['Inspect the exact final C17 and published C23 passages; preserve the inspected draft/compiler distinctions until accessible.'],
                         evidence_limits=report['limits']))
    files=[ROOT/path for path in pins()]+[CANON,ATTACH,pathlib.Path(__file__),HERE/'PR-T25-03.before-index.json',HERE/'validation.json',guard,exports]
    write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Three original mixed-source rows with exact delayed mechanism supplied; earlier full atomic support retained separately.',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(set(files))},current_reports=[REPORT,guard.relative_to(ROOT).as_posix(),exports.relative_to(ROOT).as_posix()],updates=rows))
def main():
    p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args()
    integrate() if a.integrate else package();print('PASS additive consume index '+('attachment' if a.integrate else 'three-row packet'))
if __name__=='__main__':main()
