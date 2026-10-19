"""Attach executed B01 variants to their original parent obligations."""
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[4]
BASE=ROOT/'research/v5/expansion/followthrough'
RECEIPT='reviews/v5/expansion/followthrough/foundation_transfer_attempt6.json'
PACKAGE='assessor/foundation_depth/transfer/'
PARENTS={'X-T02-001':'semantic', 'X-T04-001':'layout', 'X-T03-001':'cursor',
         'X-T03-002':'codec', 'X-T03-003':'lifecycle', 'X-T03-004':'movement',
         'X-T03-005':'registry', 'X-T35-001':'frame'}

def dump(path, data):
    path.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

def main():
    report=json.loads((ROOT/RECEIPT).read_text())
    assert report['passed'] and report['scope_source_stable']
    children=[]
    dispositions=[]
    for exercise, case in PARENTS.items():
        p=ROOT/'curriculum/v5/exercises'/f'{exercise}.md'
        text=p.read_text(encoding='utf-8')
        record=json.loads(text.split('```json\n',1)[1].split('\n```',1)[0])
        ident=exercise+'-DELAYED'
        row={'id':ident, 'project':record['project'], 'domain':record['domain'],
             'objectives':record['objectives'], 'exercise':exercise,
             'criteria':[x['id'] for x in record['rubric']], 'minimum_days':7,
             'requirement':'Implement and validate the separately packaged changed-condition assignment, then assess a sealed independent learner attempt after at least seven days.',
             'prerequisites':record['prerequisite_exercises']+record['prerequisite_artifacts']+[exercise],
             'artifacts':[PACKAGE+'ASSIGNMENTS.md',PACKAGE+'transfer.h',PACKAGE+'learner.c',PACKAGE+'reference.c',PACKAGE+'tests.c',PACKAGE+'ANALYSIS.md'],
             'completion_criteria':'Meet every linked objective rubric; passing the author reference only establishes the recorded bounded case. Keep actual learner performance pending.',
             'author_observations':{p['compiler']+'-'+p['profile']:p['observations'][case] for p in report['profiles']},
             'scope':'The '+case+' case and its stated parent variants, not all objectives in the domain.',
             'status':'author-support-executed-independent-attempt-pending'}
        children.append(row)
        dimensions={
            'teaching':{'state':'complete','evidence':[str(p.relative_to(ROOT)).replace('\\','/'),PACKAGE+'ASSIGNMENTS.md',PACKAGE+'ANALYSIS.md']},
            'implementation':{'state':'complete','evidence':[PACKAGE+'reference.c',PACKAGE+'learner.c',PACKAGE+'tests.c']},
            'author_validation':{'state':'passed','evidence':[RECEIPT,'reviews/v5/expansion/followthrough/foundation_transfer_check.json']},
            'external_review':{'state':'not-run','evidence':[]},
            'independent_learner':{'state':'not-assessed','evidence':[]},
            'physical_observation':{'state':'not-applicable-to-hosted-transfer-contract','evidence':[PACKAGE+'ASSIGNMENTS.md']}}
        dispositions.append({'id':ident,'dimensions':dimensions,
            'closure_rationale':'Concrete withheld assignment/reference and case-specific executable checks now exist. The semantic case also requires separately assessed reasoning; finite passing executions do not prove it.',
            'required_environment':'C17 GCC/Clang Linux x86-64; ASan/UBSan; Cortex-M4 cross-object/image audit only; C23 comparison is separately labeled.',
            'validation_method':'tools/run_foundation_transfer.py then tools/check_foundation_transfer_v5.py; inspect named '+case+' observations and controls. Release the delayed package only after the original learner attempt is sealed and the interval elapsed.',
            'external_blockers':['Actual unaided learner attempt with elapsed seven-day interval; non-author technical review.']})
        marker='\n## Follow-through assessor support\n'
        analysis=ROOT/record['assessor_analysis']
        old=analysis.read_text(encoding='utf-8').split(marker)[0]
        analysis.write_text(old+marker+'\nThe separately packaged ['+ident+'](../transfer/ASSIGNMENTS.md) and '
            '[worked analysis](../transfer/ANALYSIS.md) implement the parent changed-condition requirements. '
            'Use the `'+case+'` case in `tools/run_foundation_transfer.py`; '
            'the [source-bound author receipt](../../../'+RECEIPT+') records the actual scope. '
            'Keep reference, tests and reasoning solutions withheld. Author execution does not satisfy the independent learner rubric.\n',encoding='utf-8')
        # The brief is canonical Markdown. It exposes the existence and release
        # protocol of the variant, never the withheld solution content.
        if marker not in text:
            p.write_text(text+marker+'\nThe existing `'+ident+'` now has a separate assessor release package. '
                'Submit the changed-condition implementation, ownership/range argument, independent expected results and resource audit under the same objective rubric. '
                'The assessor releases its concrete interface and inputs after the prerequisite attempt is sealed and at least seven days have elapsed. '
                'Author execution of that package is evidence about the harness, not an assessment of a learner.\n',encoding='utf-8')
    groups=[
        ('G-B01-NESTED-CURSOR','X-T03-001','cursor','Enumerate admitted/rejected nested cursor states; owner-relative subviews and a tagged consumer must preserve cursor/output on failure.'),
        ('G-B01-EXTENT-CONTROLS','X-T04-001','layout','Reject multiplication, zero-width offset, and early output-write defects; compare staged C17 manual/builtin and C23 checked arithmetic with identical output contracts.'),
        ('G-B01-LIFECYCLE-MATRIX','X-T03-003','lifecycle','Execute all3125 five-operation strings over the declared five-action alphabet, plus distinct partial initializer failure/rollback and successful destructor traces.'),
        ('G-B01-FLAG-AUTHORITY','X-T35-001','frame','Reject authority taken from each syntactically valid wire flag; distinguish trusted QUERY/UPDATE policy and cancellation authority transitions.'),
        ('G-B01-REPRESENTATION-COST','X-T03-004','movement','Validate strided overlap, gap preservation, compaction epochs, in-place transformation, and equivalent-contract disjoint transformation measurements; record raw ticks without a universal speedup claim.')]
    for ident,exercise,case,requirement in groups:
        source=next(x for x in children if x['exercise']==exercise)
        children.append({'id':ident,'exercise':exercise,'objectives':source['objectives'],
            'requirement':requirement,'artifacts':[PACKAGE+'tests.c',PACKAGE+'comparisons.c',RECEIPT],
            'status':'bounded-author-campaign-passed',
            'limits':'Finite enumerated bounds are recorded in the source and receipt; not exhaustive over all C implementations or arbitrary operation lengths.'})
        d=next(x for x in dispositions if x['id']==exercise+'-DELAYED')
        dispositions.append({**d,'id':ident,'closure_rationale':requirement+' Actual named bounded observations are in the linked receipt.'})
    dump(BASE/'foundation_gap_ledger.json',{'schema_version':1,
        'policy':'Additive split of the original mixed obligation; the original historical claim remains unchanged.',
        'gaps':[{'id':'G-B01-EXPERIMENT-SCOPE','origin':'curriculum/v5/expansion/gaps.json',
            'status':'author-support-executed-external-assessment-pending',
            'remaining':children,'limits':['No independent learner performance, non-author review, physical execution or proof of absolute mastery.']} ]})
    parent_dims=json.loads(json.dumps(dispositions[0]['dimensions']))
    parent_dims['teaching']['evidence']=[PACKAGE+'ASSIGNMENTS.md',PACKAGE+'SEMANTIC_CASES.md',PACKAGE+'ANALYSIS.md']
    dispositions.append({'id':'G-B01-EXPERIMENT-SCOPE','dimensions':parent_dims,
        'child_obligations':[r['id'] for r in children],
        'closure_rationale':'Every originally named author-side experiment category has concrete support and source-bound bounded execution. The mixed parent retains independent/delayed learner and non-author review as pending dimensions; no whole-domain mastery closure.'})
    dump(BASE/'foundation_dispositions.json',{'schema_version':1,'updates':dispositions})
    p=ROOT/'curriculum/v5/expansion/implementation_updates.json'
    data=json.loads(p.read_text())
    ids={r['id'] for r in dispositions}
    data['updates']=[r for r in data['updates'] if r['id'] not in ids]+dispositions
    dump(p,data)

if __name__=='__main__': main()
