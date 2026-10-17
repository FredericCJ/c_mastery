"""Publish mandatory exercise supplements without editing accepted teaching bytes."""
from __future__ import annotations

from common import ROOT, anchor, read_json
from publish_contracts_v5 import reference, registered, target
from publish_links import links as source_links


def prepare(source, merged):
    import publish_v4 as publisher
    from publish import write, link_page
    from cmastery.exercises import learner_brief
    path = ROOT / 'curriculum/v5/exercises.json'
    if not path.is_file():
        return [], [], merged
    data = read_json(path)
    contracts = registered()
    by_id = {e['id']: e for e in data['exercises']}
    rows, sections, docs = [], [], []
    for key in data['order']:
        e = by_id[key]
        body = source_links(learner_brief(e), contracts=contracts, root=ROOT,
                            current='curriculum/v5/exercises/' + key + '.md',
                            source=source, page='learning/exercises/' + key + '.md')
        body += '\n\n## Required shared contracts\n\n' + '\n'.join(
            '- ' + reference(name) for name in e['shared_contracts']) + '\n'
        value = f'({anchor(key)})=\n' + link_page(body)
        write(source / 'learning/exercises' / (key + '.md'), value)
        docs.append('learning/exercises/' + key)
        sections.append(value)
        rows.append(['{ref}`' + anchor(key) + '`', e['title'], '{ref}`' + anchor(e['project']) + '`',
                     ', '.join(e['families']), ', '.join(e['prerequisite_exercises']) or 'Entry check'])
        addition = '\n\n## Required exercise expansion\n\n' + '{ref}`' + anchor(key) + '`: ' + e['title'] + \
                   '. Additional work; accepted unit hours remain historical.\n'
        for folder, identifier in [('projects', e['project']), ('modules', e['domain'])]:
            page = source / folder / (identifier + '.md')
            text = page.read_text(encoding='utf-8')
            write(page, text + addition)
        marker = '<a id="' + anchor(e['project']) + '"></a>'
        merged = merged.replace(marker, marker + '\n' + publisher.plain_roles(addition), 1)
    contract_docs = []
    for name in contracts:
        identifier, page = target(name)
        if page is None:
            continue
        shared = (ROOT/name).read_text(encoding='utf-8')
        shared = '(' + identifier + ')=\n' + link_page(source_links(
            shared, contracts=contracts, current=name, root=ROOT,
            source=source, page='learning/exercises/' + page + '.md'))
        write(source/'learning/exercises'/(page + '.md'), shared)
        sections.insert(0, shared)
        docs.append('learning/exercises/' + page)
        contract_docs.append(page)
    audit = '(exercise-depth-audit)=\n' + (ROOT / 'curriculum/v5/expansion/audit.md').read_text(encoding='utf-8')
    gaps = read_json(ROOT / 'curriculum/v5/expansion/gaps.json')
    audit += '\n## Open evidence and research gaps\n\n' + publisher.table(
        ['Gap', 'Scope', 'State', 'Required evidence'],
        [[g['id'], g['scope'], g['status'], g['resolution']] for g in gaps['gaps']])
    remaining = read_json(ROOT / 'curriculum/v5/expansion/remaining_work.json')
    audit += '\n## Continuation across the curriculum\n\n' + publisher.table(
        ['Batch', 'Scope', 'State'], [[b['id'], b['scope'], b['status']] for b in remaining['batches']])
    audit = source_links(audit, contracts=contracts, root=ROOT,
                         current='curriculum/v5/expansion/audit.md', source=source,
                         page='learning/exercises/audit.md')
    write(source / 'learning/exercises/audit.md', audit)
    sections.append(audit)
    docs.append('learning/exercises/audit')
    index = '(v5-exercises)=\n# Required exercise expansion\n\n'
    index += f'{len(data["order"])} additional exercises deepen existing objective contracts. They do not replace accepted '
    index += 'units or consume their historical hour envelopes. Follow the actual prerequisites; effort is uncalibrated and uncapped. '
    index += 'Public tests, author execution, independent assessment and physical evidence are distinct.\n\n'
    index += publisher.table(['Exercise', 'Mechanism', 'Parent project', 'Families', 'Prior exercises'], rows)
    index += '\nInitialize with `python -m cmastery init PROJECT DEST --exercise ID`, then '
    index += '`python -m cmastery check DEST --exercise ID`. Use `study sheet --exercise ID` for the evidence card.\n\n'
    index += '```{toctree}\n:maxdepth: 1\n\n' + '\n'.join(contract_docs) + '\naudit\n' + '\n'.join(data['order']) + '\n```\n'
    write(source / 'learning/exercises/index.md', index)
    sections.insert(0, index.split('```{toctree}')[0])
    # Only the index is included in the parent toctree; it owns the other pages.
    return ['learning/exercises/index'], sections, merged
