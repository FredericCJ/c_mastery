"""Render required legacy criteria; never read assessor solution bodies."""
from common import ROOT, anchor, read_json


from cmastery.assessments import learner_brief
from publish_contracts_v5 import reference


def prepare(source, merged):
    import publish_v4 as publisher
    from publish import write, link_page
    base = ROOT / 'curriculum/v5/expansion/legacy_assessments'
    paths = sorted(base.glob('PR-*.json'))
    if not paths:
        return [], [], merged
    shared = '(legacy-assessment-shared-contract)=\n' + (base / 'shared-contract.md').read_text(encoding='utf-8')
    write(source / 'learning/legacy-assessments/shared-contract.md', shared)
    sections, rows, names = [shared], [], []
    for path in paths:
        data = read_json(path)
        project, domain = data['project'], data['domain']
        name = 'learning/legacy-assessments/' + project
        names.append(project)
        page_anchor = 'legacy-assessment-' + anchor(project)
        body = learner_brief(data)
        for contract_name in data['shared_contracts']:
            body = body.replace('- ' + contract_name + '\n', '- ' + reference(contract_name) + '\n')
        value = '(' + page_anchor + ')=\n' + link_page(body)
        write(source / (name + '.md'), value)
        sections.append(value)
        rows.append(['{ref}`' + page_anchor + '`', str(len(data['criteria'])), data['status']['content'],
                     data['status']['learner_assessment']])
        addition = '\n\n## Required objective assessments\n\n' + '{ref}`' + page_anchor + '`: '
        addition += str(len(data['criteria'])) + ' explicit criteria with separate boundary and delayed-transfer obligations.\n'
        domains = ([domain] if domain != 'INTEGRATION' else
                   sorted({criterion['objective'].split('.')[0] for criterion in data['criteria']}))
        for folder, identifier in [('projects', project), *[('modules', item) for item in domains]]:
            page = source / folder / (identifier + '.md')
            write(page, page.read_text(encoding='utf-8') + addition)
        marker = '<a id="' + anchor(project) + '"></a>'
        merged = merged.replace(marker, marker + '\n' + publisher.plain_roles(addition), 1)
    index = '(legacy-assessments)=\n# Required assessments of retained projects\n\n'
    index += 'These criteria reuse the original implementations and bind inspected source passages. '
    index += 'A structural validation or author campaign is distinct from an actual learner assessment. '
    index += 'Required work remains uncapped by historical hours.\n\n'
    index += publisher.table(['Parent project', 'Criteria', 'Content', 'Learner evidence'], rows)
    index += '\nRead {ref}`legacy-assessment-shared-contract` before collecting evidence.\n\n'
    sections.insert(0, index)
    index += '```{toctree}\n:maxdepth: 1\n\nshared-contract\n' + '\n'.join(names) + '\n```\n'
    write(source / 'learning/legacy-assessments/index.md', index)
    return ['learning/legacy-assessments/index'], sections, merged
