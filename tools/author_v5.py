"""Author the fifth-edition catalog: the accepted v4 units replayed verbatim, then manifest-declared domains.

The generator is the v4 authoring function bound to RULES_V5. Nothing about a
retained unit, recall set or artifact contract may change: check_v5 compares
every digest against curriculum/v5/baseline.json.
"""
from copy import deepcopy
from author_v4 import RULES_V4, author
from v5_common import V5, baseline, domains_manifest, manifest, require_domain_research
import vocabulary
from common import read_json


def vocabulary_sets():
    """One recall set per domain glossary that declares its three prompts; facets are the domain's terms."""
    entries, documents = vocabulary.load()
    sets = []
    for owner, document in sorted(documents.items()):
        recall = document.get('recall')
        if owner == 'core' or not recall:
            continue
        terms = sorted((e for e in entries if e['owner'] == owner), key=lambda e: e['term'].casefold())
        if not terms:
            continue
        key = 'V-' + owner
        facets = [key + '-' + e['id'].split('-', 2)[-1] if e['id'].startswith('V-') else key + '-' + e['id'] for e in terms]
        checklist = [e['term'] + ': ' + e['definition'].split('. ')[0] for e in terms]
        kinds = [('define', 'mechanism'), ('distinguish', 'trace-or-small-implementation'), ('apply', 'changed-condition')]
        variants = [{'id': key + '-' + name, 'kind': kind, 'objectives': facets, 'prompt': recall[name], 'checklist': checklist}
                    for name, kind in kinds]
        sets.append({'id': key, 'kind': 'vocabulary', 'title': owner + ' vocabulary', 'objectives': facets,
                     'objective_text': {f: e['term'] for f, e in zip(facets, terms)}, 'projects': [], 'variants': variants,
                     'resource_ids': sorted({a['resource_id'] for e in terms for a in e.get('authority', [])}),
                     'elective': False,
                     'grading': 'Score only the terms actually defined, distinguished or applied; a consulted glossary is assisted practice.'})
    return sets


def rules_v5():
    data = domains_manifest()
    require_domain_research(data)
    rules = deepcopy(RULES_V4)
    rules['phases'] = list(RULES_V4['phases']) + [(p['title'], p['gate']) for p in data.get('phases', [])]
    phase_domains = {key: list(value) for key, value in RULES_V4['phase_domains'].items()}
    for key, domain in sorted(data.get('domains', {}).items()):
        phase_domains.setdefault(domain['phase'], []).append(int(key[1:]))
        if domain.get('strand'):
            rules['strands'][key] = domain['strand']
        if domain.get('imports'):
            rules['imports'][key] = domain['imports']
        for art in domain.get('artifacts', []):
            rules['artifacts'].append((art['id'], art['producer'], art['contract']))
            rules['critical'][art['id']] = list(art['critical_objectives'])
        for pid, decision in domain.get('decisions', {}).items():
            rules['decisions'][pid] = (decision['question'], list(decision['alternatives']),
                                       decision['discriminator'], decision['reversal_trigger'])
    rules['phase_domains'] = phase_domains
    base = baseline()
    rules['order_prefix'] = list(base['order'])
    total = manifest()['expected']['total']
    rules['budget'] = {'catalog_hours': total['catalog_hours'], 'core_hours': total['core_hours'],
                       'core_after_diagnostic_hours': total['core_after_diagnostic_hours']}
    rules['consolidation_sets'] = (str(total['recall_sets']) +
                                   ' activated topic sets; five short reviews at a planned five minutes each')
    rules['migration_note'] = ('retained identity and budget; the fifth-edition catalog appends new domains '
                               'without changing accepted units')
    rules['authored_domains'] = sorted(data.get('domains', {}))
    rules['authored_recall'] = {}
    for key in rules['authored_domains']:
        path = V5 / 'recall' / (key + '.json')
        if not path.is_file():
            raise ValueError('New domain needs authored recall variants: ' + key)
        for row in read_json(path)['sets']:
            if not row['id'].startswith(key + '.') or row['id'] in rules['authored_recall']:
                raise ValueError('Conflicting authored recall ownership: ' + row['id'])
            rules['authored_recall'][row['id']] = row
    rules['vocabulary_sets'] = vocabulary_sets()
    return rules


def main():
    author(rules_v5(), V5, baseline())
    if (V5 / 'exercises').is_dir():
        from expansion_evidence import require_research
        require_research()
        from author_exercises_v5 import main as author_exercises
        author_exercises()
        from check_legacy_assessments import check as legacy_assessments
        legacy_report = legacy_assessments()
        if legacy_report['failures']:
            raise ValueError('Legacy assessment contracts: ' + str(legacy_report['failures']))
        from audit_expansion_v5 import generate
        generate()
        from reconcile_expansion_progress import generate as progress
        progress()


if __name__ == '__main__':
    main()
