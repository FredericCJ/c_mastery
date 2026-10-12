"""Author the fifth-edition catalog: the accepted v4 units replayed verbatim, then manifest-declared domains.

The generator is the v4 authoring function bound to RULES_V5. Nothing about a
retained unit, recall set or artifact contract may change: check_v5 compares
every digest against curriculum/v5/baseline.json.
"""
from copy import deepcopy
from author_v4 import RULES_V4, author
from v5_common import V5, baseline, domains_manifest, manifest


def rules_v5():
    data = domains_manifest()
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
    return rules


def main():
    author(rules_v5(), V5, baseline())


if __name__ == '__main__':
    main()
