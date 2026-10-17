"""Generate additive depth/traceability records without crediting catalog mentions as mastery."""
from __future__ import annotations

from collections import Counter
import re

from common import ROOT, dump, load, read_json
from v5_common import baseline

BASE = ROOT / 'curriculum/v5/expansion'


def project_facet(mapping, facet):
    if mapping is None:
        return []
    key = {'implementation_or_analysis': 'implementation_analysis'}.get(facet, facet)
    return [{'project': mapping['implementation_analysis']['project'],
             'specified_evidence': mapping[key], 'scope': mapping['scope'],
             'remaining': mapping['remaining_required'],
             'rubric': mapping['independent_assessment']['criterion'],
             'source_assignments': mapping['source_topic'],
             'status': 'authored-not-learner-assessed',
             'learner_artifacts': mapping['implementation_analysis'],
             'source': 'research/v5/expansion/b06/coverage.json'}]


def generate():
    taxonomy, modules, projects, _ = load()
    units = read_json(ROOT / 'curriculum/v5/units.json')['units']
    exercise_path = ROOT / 'curriculum/v5/exercises.json'
    exercises = read_json(exercise_path)['exercises'] if exercise_path.is_file() else []
    accepted = baseline()
    legacy_path = BASE / 'legacy_assessment_catalog.json'
    legacy = read_json(legacy_path)['criteria'] if legacy_path.is_file() else []
    project_maps = {}
    project_map_path = ROOT/'research/v5/expansion/b06/coverage.json'
    if project_map_path.is_file():
        project_maps = {r['objective']: r for r in read_json(project_map_path)['objective_map']}
    rows = []
    for domain in taxonomy:
        for topic in domain['subtopics']:
            for objective in topic['objectives']:
                oid = objective['id']
                linked = [e for e in exercises if oid in e['objectives']]
                historical = [u for u in units if oid in u['objectives']]
                project_mapping = project_maps.get(oid)
                criteria = [c for c in legacy if c['objective'] == oid]
                authored = bool(linked or project_mapping or criteria)
                rows.append({
                    'objective': oid, 'text': objective['text'], 'domain': domain['id'], 'topic': topic['id'],
                    'historical_module': f"curriculum/modules/{domain['id']}.md",
                    'historical_units': [u['id'] for u in historical],
                    'historical_named_tests': sorted({test for u in historical for test in u['tests']}),
                    'historical_evidence_limit': 'Catalog links are preserved; their presence is not a fresh content or execution assessment.',
                    'identity_status': 'accepted-baseline' if oid in accepted['objectives'] else 'authored-addition',
                    'expansion_status': 'authored-facets-require-assessment' if authored else 'not-depth-audited',
                    'exercises': [e['id'] for e in linked],
                    'project_unit_assessment': project_mapping,
                    'legacy_assessment_criteria': criteria,
                    'facets': {name: [
                        {'exercise': e['id'], 'specified_evidence': next(c for c in e['objective_coverage'] if c['objective'] == oid)[field],
                         'assessed_claim': next(c for c in e['objective_coverage'] if c['objective'] == oid).get('assessed_claim'),
                         'scope': next(c for c in e['objective_coverage'] if c['objective'] == oid).get('scope', 'Only the named criterion is assessed; the whole leaf is not inferred complete.'),
                         'rubric': [c for c in e['rubric'] if c['objective'] == oid],
                         'source_assignments': e['source_records'], 'status': 'authored-not-learner-assessed'}
                        for e in linked] + [
                            {'criterion': c['id'], 'specified_evidence': c['facet_scope'][name],
                             'scope': c['facet_scope']['remaining'], 'rubric': c['rubric'],
                             'source_assignments': c['source_records'], 'status': c['status'],
                             'learner_artifacts': c['deliverables'], 'source': c['source']}
                            for c in criteria] + project_facet(project_mapping, name)
                        for name, field in [('teaching', 'teaching'), ('implementation_or_analysis', 'implementation'),
                                            ('boundary_adversarial', 'boundary'), ('independent_assessment', 'independent_assessment')]},
                    'learner_mastery': 'not-assessed',
                    'remaining': ('Assess each mapped rubric criterion and delayed variant; no learner result is inferred. '
                                  + (project_mapping.get('remaining_required','') if project_mapping else '')) if authored
                                 else 'Inspect actual teaching, learner work and assessor obligations before choosing a depth expansion.'
                })
    counts = {'domains': len(taxonomy), 'objectives': len(rows), 'units': len(units),
              'units_with_named_tests': sum(bool(u['tests']) for u in units),
              'units_without_named_tests': sum(not u['tests'] for u in units),
              'exercise_sidecars': len(exercises), 'project_unit_exercises': len({r['implementation_analysis']['project'] for r in project_maps.values()}),
              'legacy_assessment_criteria': len(legacy),
              'legacy_assessment_projects': len({c['project'] for c in legacy}),
              'objectives_with_added_exercises': sum(bool(r['exercises'] or r['project_unit_assessment'] or r['legacy_assessment_criteria']) for r in rows)}
    dump(BASE / 'coverage_depth.json', {'schema_version': 1,
        'policy': 'Generated mapping of actual exercise criteria, not a claim that all baseline teaching has been reassessed. '
                  'Analysis work can be adequate without named executable tests. Every facet retains separate evidence.',
        'counts': counts, 'objectives': rows})
    plan = (ROOT / 'planning/modern_techniques_plan.md').read_text(encoding='utf-8')
    technique_rows = []
    authored_dispositions = {}
    for path in sorted((ROOT/'research/v5/modern/coverage').glob('*.json')):
        for record in read_json(path).get('census',[]):
            authored_dispositions.setdefault(record.get('technique'),[]).append(dict(record, register=path.relative_to(ROOT).as_posix()))
    for prefix, count in [('M', 26), ('C', 24), ('P', 26), ('D', 24)]:
        for number in range(1, count + 1):
            key = f'{prefix}{number:02}'
            domains = []
            for block in re.split(r'(?=^### T\d\d\b)', plan, flags=re.M):
                match = re.match(r'### (T\d\d)\b', block)
                if match and re.search(r'\b' + key + r'\b', block):
                    domains.append(match[1])
            technique_rows.append({'technique': key, 'candidate_domains': domains,
                'status': 'authored-disposition-requires-resolution-check' if key in authored_dispositions else 'planned-not-authored',
                'dispositions': authored_dispositions.get(key,[]), 'source_status': 'discovery-lead-not-passage-evidence',
                'next_action': 'Use check_modern_coverage.py to resolve actual rubric, artifact and inspected-source links. A disposition is not learner evidence.'})
    dump(BASE / 'modern_discovery.json', {'schema_version': 1, 'techniques': technique_rows,
        'limits': 'Explicit mentions in the existing plan are candidate ownership only. No modern exercise coverage is inferred. '
                  'The SWE catalog and library still require topic-by-topic inspection; these 100 techniques are not the whole corpus.'})
    domain_rows = []
    for d in taxonomy:
        owned = [r for r in rows if r['domain'] == d['id']]
        domain_rows.append({'domain': d['id'], 'title': d['title'], 'objectives': len(owned),
                           'objectives_with_additions': sum(bool(r['exercises'] or r['project_unit_assessment'] or r['legacy_assessment_criteria']) for r in owned),
                           'status': 'authored-assessment-obligations' if all(r['exercises'] or r['project_unit_assessment'] or r['legacy_assessment_criteria'] for r in owned)
                                     else ('partial-expansion' if any(r['exercises'] or r['legacy_assessment_criteria'] for r in owned) else 'depth-audit-pending')})
    dump(BASE / 'domain_progress.json', {'schema_version': 1, 'domains': domain_rows,
        'planned_domains': [{'domain': f'T{i}', 'status': 'authored' if any(d['id']==f'T{i}' for d in taxonomy) else 'planned-not-authored'} for i in range(57, 77)]})
    lines = ['# Coverage and depth audit — integrated expansion', '',
             'This is a bounded inspection of the active repository, with an explicit remaining-work map. '
             'It does not claim a fresh substantive reading of every source or every existing exercise.', '',
             '| Observation | Count |', '|---|---:|']
    lines += [f'| {k.replace("_", " ")} | {v} |' for k, v in counts.items()]
    lines += ['', 'Named tests measure executable support only. Analysis exercises need inspectable reasoning and assessment, '
              'not an irrelevant execution quota. Accepted T01–T56 identities remain preserved. New-domain status is derived from actual canonical contracts below.', '',
              '## Consequential inspected gaps', '',
              '- PR-T03-03 covers effective type, incompatible lvalues, restrict and provenance, but its fixture contains four mathematical overlap results and no dedicated family mapping. B01 adds mechanism-specific exercises.',
              '- PR-T16-03 names generation handles and wrap policy, but three fixture snapshots do not assess complete reuse and exhaustion histories. B01 adds a finite transition model and retirement policy.',
              '- Existing foundation family mappings explicitly identify their visible tests as partial anchors. The expansion retains those limitations and adds objective-specific criteria.',
              '- Real retained process, VM, storage and kernel laboratories are stronger than their arithmetic fixtures, but do not execute every newly specified failure and transfer task. Scoped audits distinguish these evidence layers.',
              '- Actual communication-source inspection exposed lower-send admission, session receipt, fixed wire-length and descriptor-incarnation gaps. Added original laboratories retain the historical implementations as bounded comparison controls.',
              '- The MCU cooperative stage uses PendSV, the retained fault handler stops the image, and periodic idle does not establish tickless operation. Required supplemental tasks keep these distinct mechanisms explicit.',
              '- The old v5 acceptance records a different source identity from its current pre-expansion publication; its implementing-agent review leaves independent review outstanding.',
              '- Ordinary units are generated inside accepted hour envelopes. The exercise layer avoids reallocating those historical envelopes.', '',
              '## Mapping and limits', '',
              '`coverage_depth.json` includes every existing leaf objective, its preserved unit/test links and any actual expansion criteria. '
              '`domain_progress.json` distinguishes partial deepening from unaudited domains. B06 project/unit exercises retain their existing schema and explicit leaf facets rather than being duplicated as sidecars. `modern_discovery.json` tracks all100 census IDs and their authored dispositions; check_modern_coverage validates the actual links.', '',
              'B01 adds no taxonomy leaves: its demonstrated competencies already have applicable leaf objectives. '
              'New omissions require additive taxonomy records with authority, prerequisites and assessment rationale. T57–T76 domain fragments and research taxonomies record actual additions; uncreated domains remain planned.', '',
              'The canonical gap ledger and remaining-work manifest define what is still open. '
              'Hardware validation, learner independence and research completeness are not inferred from author execution.']
    scoped_audits = sorted(p for p in (ROOT/'research/v5/expansion').glob('*/*.md')
                          if p.name.lower() in {'audit.md', 'completion.md'})
    if scoped_audits:
        lines += ['', '## Scoped source and depth audits', '',
                  'Each audit identifies the actual passages read, mechanism-specific additions and required remaining experiments. '
                  'Its source register and criterion-level mapping are authoritative for that bounded inspection.', '']
        lines += ['- [' + p.parent.name + '](../../../' + p.relative_to(ROOT).as_posix() + ')' for p in scoped_audits]
    (BASE / 'audit.md').write_text('\n'.join(lines) + '\n', encoding='utf-8', newline='\n')
    return counts


if __name__ == '__main__':
    print(generate())
