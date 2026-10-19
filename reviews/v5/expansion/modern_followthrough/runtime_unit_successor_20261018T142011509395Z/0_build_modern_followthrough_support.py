"""Prepare exact additive support; never generate catalogs or merge inventory."""
import copy
import hashlib
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from cmastery.exercises import parse_brief

BASE = 'research/v5/expansion/modern_followthrough/'
OLD_RUNTIME = 'reviews/v5/expansion/modern_completion/object_runtime_current_campaign.json'
NEW_RUNTIME = 'reviews/v5/expansion/modern_followthrough/object_runtime_current_01.json'
FREE = 'reviews/v5/expansion/modern_followthrough/free_01.json'
FINAL = 'reviews/v5/expansion/modern_followthrough/final_02.json'
DIMENSIONS = ('teaching', 'implementation', 'author_validation', 'external_review',
              'independent_learner', 'physical_observation')
COMMON_LIMITS = ['Original finite author support only; external review and independent delayed learner performance remain unassessed.',
                 'Source inspection and native/LLVM execution are separate from physical target observations or exact published ISO edition access.',
                 'Each original criterion and changed condition retains its own evidence; another anchor does not discharge it.']


def read(path):
    return json.loads((ROOT/path).read_text(encoding='utf8'))


def sha(path):
    return hashlib.sha256((ROOT/path).read_bytes()).hexdigest()


def write(path, value):
    p = ROOT/path
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(json.dumps(value, ensure_ascii=False, indent=2)+'\n', encoding='utf8', newline='\n')


def current(paths):
    return sorted(set(NEW_RUNTIME if p == OLD_RUNTIME else p for p in paths))


def dimensions(teaching, implementation, validation):
    return dict(teaching=dict(state='complete-retained-and-exact-companion-teaching', evidence=current(teaching)),
                implementation=dict(state='implemented-exact-bounded-mechanisms', evidence=current(implementation)),
                author_validation=dict(state='passed-source-bound-exact-author-support', evidence=current(validation)),
                external_review=dict(state='not-run', evidence=[]),
                independent_learner=dict(state='not-assessed', evidence=[]),
                physical_observation=dict(state='not-claimed-separate-target-obligation', evidence=['labs/modern_followthrough/README.md']))


def variant_text(body):
    match = re.search(r'^## Variants\s*\n(.*?)(?=^## |\Z)', body, re.M|re.S)
    if not match:
        raise ValueError('Missing original variants')
    return match[1].strip()


def aggregate(rows):
    return {key: dict(state=rows[0]['dimensions'][key]['state'],
                      evidence=sorted({p for r in rows for p in r['dimensions'][key]['evidence']}))
            for key in DIMENSIONS}


def make():
    rows, bindings, reports, inputs = [], [], set(), set()
    previous = {}
    readiness = {}
    for family in ('dispatch', 'dod', 'object_runtime'):
        path = 'research/v5/modern/'+family+'_objective_readiness.json'
        data = read(path); inputs.add(path)
        for row in data['criteria']:
            readiness[row['id']] = (path, row)
            inputs.update(row['pins'])
        path = 'research/v5/modern/'+family+'_completion_dispositions.json'
        inputs.add(path)
        previous.update({r['id']: r for r in read(path)['updates']})
    paths = sorted(p for p in (ROOT/'curriculum/v5/exercises').glob('X-*.md')
                   if p.stem.startswith(('X-T69-', 'X-T70-', 'X-T74-')) or p.stem == 'X-T72-010')
    for path in paths:
        meta, body = parse_brief(path)
        origin = path.relative_to(ROOT).as_posix(); inputs.add(origin)
        children = []
        for criterion in meta['rubric']:
            ident = criterion['id']
            if ident in readiness:
                ready_path, support = readiness[ident]
                # One historical readiness row has a second UTF-8 mojibake pass.
                # Preserve it and require the exact single recorded difference;
                # copy the canonical original object, never normalize arbitrary text.
                binding_text = support['criterion']
                if ident == 'X-T70-007-R3':
                    binding_text = binding_text.replace('0Ã¢â‚¬â€œ31', '0â€“31')
                if binding_text != criterion['criterion'] or support['objective'] != criterion['objective']:
                    raise ValueError('Criterion changed: '+ident)
                teach = support['teaching']
                evidence = support['implementation_and_observation']
                validation = [p for p in evidence if p.endswith('.json') and 'passed' in read(p)]
                implementation = [p for p in evidence if p not in validation]
                scope, limits = support['scope'], support.get('limits', [])
            else:
                assert meta['id'] == 'X-T72-010'
                ready_path = 'research/v5/modern/higher_order_completion_dispositions.json'
                teach = [origin, 'labs/higher_order/CONTRACTS.md']
                implementation = ['assessor/higher_order/free.c', 'labs/higher_order/include/higher_order.h',
                                  'labs/higher_order/tests/tests.c', 'tools/verify_higher_order.py']
                validation = ['reviews/v5/expansion/modern_completion/higher_order_campaign.json',
                              'reviews/v5/expansion/modern_completion/higher_order_costs.json']
                scope = {'R1':'Finite Pure/Emit/Choose DAG, strictly descending edges and borrowed arena roots.',
                         'R2':'Transactional terminal substitution, path interpreter and memoized checked summary.',
                         'R3':'Original independent traces/laws/cuts plus exact full-arena bind and callback rollback controls.',
                         'R4':'Shared 64-node/2^63-path transfer, INT64_MIN/zero/positive overflow and allocation-law counterexample.'}[ident.rsplit('-',1)[1]]
                limits = ['Arbitrary callback side effects are not rolled back. Laws compare admitted meanings, not allocation traces.']
            if meta['id'] in ('X-T72-010','X-T69-012'):
                kind, probe_report = ('free', FREE) if meta['id']=='X-T72-010' else ('final', FINAL)
                teach = teach + ['labs/modern_followthrough/'+kind+'/CONTRACTS.md']
                implementation += ['labs/modern_followthrough/'+kind+'/probe.c',
                                   'assessor/modern_followthrough/'+kind+'/oracle.py',
                                   'assessor/modern_followthrough/'+kind+'/ANALYSIS.md']
                validation += [probe_report]
            private_notes = [p for p in teach if p.startswith('assessor/')]
            dim = dimensions([p for p in teach if not p.startswith('assessor/')], implementation+private_notes,
                             validation+[ready_path])
            row = dict(id=ident, original_source=origin, original_criterion=copy.deepcopy(criterion),
                       closure_rationale=scope, dimensions=dim, evidence_limits=limits+COMMON_LIMITS,
                       external_blockers=['Independent reviewer and learner observations remain separate.'])
            rows.append(row); children.append(row); reports.update(current(validation)); inputs.add(ready_path)
        variant_ids = meta.get('delayed_assessment', meta.get('delayed_transfer'))['variant_ids']
        assert len(variant_ids) == 1 and variant_ids[0].startswith(meta['id'])
        variant = variant_ids[0]
        if variant in previous:
            prior = previous[variant]
            dim = copy.deepcopy(prior['dimensions'])
            for key in DIMENSIONS: dim[key]['evidence'] = current(dim[key]['evidence'])
            dim['physical_observation'] = dimensions([],[],[])['physical_observation']
            rationale = prior['closure_rationale']
            limits = prior.get('evidence_limits', [])
        else:
            assert meta['id'] in ('X-T69-012','X-T72-010')
            dim = aggregate(children)
            rationale = ('Exact rendering/evaluation failed builders, retained callback effects, and lambda/application capacity publication.'
                         if meta['id']=='X-T69-012' else
                         'Exact full-arena identity bind failure, all copied-node cuts and 2^63-path signed/unsigned summary boundaries.')
            limits = []
        delayed = dict(id=variant, original_source=origin, original_variants=variant_text(body),
                       minimum_days=meta.get('delayed_assessment',meta.get('delayed_transfer'))['minimum_days'],
                       dimensions=dim, closure_rationale=rationale, evidence_limits=limits+COMMON_LIMITS)
        rows.append(delayed)
        rows.append(dict(id=meta['id'], original_source=origin, required_criteria=[r['id'] for r in children],
                         required_variants=[variant], dimensions=aggregate(children+[delayed]),
                         closure_rationale='Exact constituent rubric and changed-condition evidence; no family-wide acceptance inference.',
                         evidence_limits=COMMON_LIMITS))
        bindings.append(dict(exercise=meta['id'], source=origin, prerequisites=meta['prerequisite_exercises'],
                             criteria=copy.deepcopy(meta['rubric']), variants=variant_text(body),
                             teaching=sorted({p for r in children for p in r['dimensions']['teaching']['evidence']})))
    # Unnamed modern register rows are aliases of already implemented exact work.
    aliases = []
    origin = 'research/v5/modern/errors_ownership_remaining.json'; inputs.add(origin)
    for index, original in enumerate(read(origin)['rows']):
        ident = 'IW-'+hashlib.sha256((origin+'\n'+json.dumps(original,sort_keys=True,ensure_ascii=False)).encode()).hexdigest()[:16]
        readiness_path = original['evidence'][0]; inputs.add(readiness_path)
        data = read(readiness_path); inputs.update(data['input_sha256'])
        teaching = [p for r in data['criteria'] for p in r['teaching'] if not p.startswith('assessor/')]
        implementation = [p for r in data['criteria'] for p in r['implementation']]
        validation = list(data['current_reports']); reports.update(validation)
        rows.append(dict(id=ident, original_source=origin, original_pointer='/rows/'+str(index),
                         original_register_row=copy.deepcopy(original), exact_original_ids=original['original_ids'],
                         dimensions=dimensions(teaching, implementation, validation+[readiness_path]),
                         closure_rationale=original['author_obligation']+' Exact original rubric and delayed evidence reused after source/report/toolchain checks.',
                         evidence_limits=data['limits']+COMMON_LIMITS))
        aliases.append(dict(id=ident, exercise=original['exercise'], readiness=readiness_path,
                            original_ids=original['original_ids']))
    for row in rows:
        for dim in row['dimensions'].values(): inputs.update(dim['evidence'])
    # Preserve old envelopes as inputs but validate the explicit new runtime report.
    for report in sorted(reports):
        data = read(report)
        inputs.add(report)
        for key in ('inspected_paths','input_sha256'):
            if isinstance(data.get(key),dict): inputs.update(data[key])
    write('labs/modern_followthrough/criteria.json', dict(schema_version=1, roots=bindings,
          limits='Original criterion text and learner-facing teaching index only; no scores or assessor answers.'))
    write(BASE+'evidence_map.json', dict(schema_version=1, roots=bindings, register_aliases=aliases,
          report_successors={OLD_RUNTIME:NEW_RUNTIME}, current_reports=sorted(reports)))
    inputs.update(['labs/modern_followthrough/criteria.json','labs/modern_followthrough/README.md',
                   BASE+'evidence_map.json',BASE+'audit_notes.json',
                   'tools/build_modern_followthrough_support.py','tools/check_modern_followthrough.py',
                   'tools/check_object_runtime_followthrough.py','cmastery/exercises.py'])
    result = dict(schema_version=1, required=True, updates=sorted(rows,key=lambda r:r['id']),
                  scope='34 original roots,143 rubric criteria,34 delayed variants and20 exact unnamed modern register aliases. Two concrete missing delayed probes added; retained exact mechanisms and original historical envelopes preserved.',
                  source_sha256={p:sha(p) for p in sorted(inputs)}, current_reports=sorted(reports),
                  report_successors={OLD_RUNTIME:NEW_RUNTIME}, remaining_author_work=[],
                  remaining_external_work=COMMON_LIMITS,
                  source_records=[dict(id='R-MODERN-FOLLOWTHROUGH-LOCAL',path=BASE+'audit_notes.json',
                                       passages=['Actual original criteria, source-bound readiness and exact new probe findings; no newly inspected external edition claim.'])])
    write(BASE+'support.json',result)
    print(json.dumps({'updates':len(rows),'source_pins':len(inputs),'reports':len(reports)}))


if __name__=='__main__':
    make()
