from __future__ import annotations
import hashlib
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
META = re.compile(r'\A```json\s*\n(.*?)\n```\s*\n', re.S)
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))
from cmastery.text import normalize_prose, find_glued, strip_bullet  # noqa: E402

def read_json(path):
    return json.loads(Path(path).read_text(encoding='utf-8-sig'))

def dump(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8', newline='\n')

def write(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(value, encoding='utf-8', newline='\n')

def parse(path):
    value = Path(path).read_text(encoding='utf-8-sig')
    match = META.match(value)
    if not match:
        raise ValueError(f'{Path(path).name}: missing fenced JSON contract')
    return json.loads(match[1]), value[match.end():]

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def load_taxonomy():
    taxonomy = read_json(ROOT / 'curriculum/taxonomy.json')['domains']
    for name in ['enhancement_taxonomy.json','performance_debugging_taxonomy.json','v2_taxonomy.json','v3_taxonomy.json','v5_taxonomy.json']:
        extension = ROOT / 'curriculum' / name
        if extension.exists(): taxonomy += read_json(extension)['domains']
    return taxonomy

def verify_research_hashes(gate, folder, addition_glob='*_additional_resources.json'):
    pinned=gate.get('research_input_sha256',{})
    if not pinned: raise ValueError('Research gate has no pinned evidence')
    for addition in (ROOT/folder).glob(addition_glob):
        if addition.relative_to(ROOT).as_posix() not in pinned:
            raise ValueError('Additional source scope has not been reviewed: '+addition.name)
    for path,digest in pinned.items():
        if sha(ROOT/path)!=digest:
            raise ValueError('Research input changed after gate: '+path)

def require_scoped_research_gate(group):
    gate=read_json(ROOT/'reviews/performance_debugging_research_gate.json')
    scoped=gate.get('groups',{}).get(group,{})
    if scoped.get('accepted') is not True: raise ValueError('Scoped source research not accepted: '+group)
    verify_research_hashes(scoped,'research/performance_debugging',group+'*_additional_resources.json')

def require_research_gates():
    if (ROOT/'curriculum/v5/exercises').is_dir():
        from expansion_evidence import require_research
        require_research()
    if not read_json(ROOT / 'reviews/research_gate.json').get('accepted'):
        raise ValueError('Research gate not accepted')
    if (ROOT / 'curriculum/enhancement_taxonomy.json').exists():
        gate = ROOT / 'reviews/enhancement_research_gate.json'
        if not gate.exists() or not read_json(gate).get('accepted'):
            raise ValueError('Enhancement research gate not accepted')
        verify_research_hashes(read_json(gate),'research/enhancement')
    if (ROOT/'curriculum/performance_debugging_taxonomy.json').exists():
        gate=ROOT/'reviews/performance_debugging_research_gate.json'
        if not gate.exists() or read_json(gate).get('accepted') is not True or read_json(gate).get('accepted_domains')!=['T43','T44','T45']:
            raise ValueError('Performance/debugging research gate not accepted')
        verify_research_hashes(read_json(gate),'research/performance_debugging')
    if (ROOT/'curriculum/edition.json').exists():
        gate=ROOT/'reviews/v2/research_gate.json'
        if not gate.exists() or read_json(gate).get('accepted') is not True:
            raise ValueError('Second-edition source research gate not accepted')
        prior=read_json(gate)
        if (ROOT/'curriculum/v3_taxonomy.json').exists():
            # The accepted v2 receipt remains immutable. Its two aggregate
            # registries are preserved byte-for-byte before adding v3 sources.
            prior=dict(prior)
            prior['research_input_sha256']={
                ('references/baseline_v2/'+Path(path).name if path in
                 {'references/resources.json','references/registry_v2.json'} else path):digest
                for path,digest in prior['research_input_sha256'].items()}
        verify_research_hashes(prior,'research/v2','*_resources.json')
    if (ROOT/'curriculum/v3_taxonomy.json').exists():
        gate=ROOT/'reviews/v3/research_gate.json'
        if not gate.exists() or read_json(gate).get('accepted') is not True:
            raise ValueError('Third-edition source research gate not accepted')
        verify_research_hashes(read_json(gate),'research/v3','*resources.json')
    if (ROOT/'curriculum/v5_taxonomy.json').exists():
        from v5_common import require_domain_research, domains_manifest
        data = domains_manifest()
        require_domain_research(data)
        authored = {d['id'] for d in read_json(ROOT/'curriculum/v5_taxonomy.json')['domains']}
        if authored != set(data['domains']):
            raise ValueError('Fifth-edition taxonomy and gated domain recipes differ')

def check_taxonomy_preservation(taxonomy):
    if (ROOT/'curriculum/edition.json').exists():
        from v2_common import check_migration
        return check_migration(taxonomy)
    errors = []
    base = read_json(ROOT / 'curriculum/taxonomy.json')['domains']
    actual_base = [d for d in taxonomy if d['id'] in {x['id'] for x in base}]
    if actual_base != base:
        errors.append('Original 1,185 unique objectives and taxonomy must be preserved exactly')
    domains = [d['id'] for d in taxonomy]
    topics = [t['id'] for d in taxonomy for t in d['subtopics']]
    leaves = [o['id'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']]
    for name, ids in [('domain', domains), ('topic', topics), ('objective', leaves)]:
        if len(ids) != len(set(ids)):
            errors.append('Duplicate taxonomy ' + name + ' IDs')
    for d in taxonomy:
        for t in d['subtopics']:
            if not t['id'].startswith(d['id'] + '.'):
                errors.append('Topic outside owning domain: ' + t['id'])
            if {o['id'] for o in t['objectives']} != {t['id'] + '.' + x for x in 'abc'}:
                errors.append('Topic must have three distinct a/b/c objectives: ' + t['id'])
    manifest = read_json(ROOT / 'reviews/baseline_2026_10_04/artifact_manifest.json')
    if sha(ROOT / 'curriculum/taxonomy.json') != manifest['artifacts']['curriculum/taxonomy.json']:
        errors.append('Original taxonomy file changed from accepted baseline')
    if (ROOT/'curriculum/performance_debugging_taxonomy.json').exists():
        prior=read_json(ROOT/'curriculum/enhancement_taxonomy.json')['domains']
        actual=[d for d in taxonomy if d['id'] in {x['id'] for x in prior}]
        pinned=read_json(ROOT/'reviews/baseline_2026_10_09_architecture_networking/artifact_manifest.json')['artifacts']
        if actual!=prior or sha(ROOT/'curriculum/enhancement_taxonomy.json')!=pinned['curriculum/enhancement_taxonomy.json']:
            errors.append('Prior 198 enhancement objectives and taxonomy must be preserved exactly')
    return errors

def load():
    taxonomy = load_taxonomy()
    modules, projects = {}, {}
    for folder, target in [('modules', modules), ('projects', projects)]:
        for path in sorted((ROOT / f'curriculum/{folder}').glob('*.md')):
            meta, body = parse(path)
            key = meta['id']
            if key in target:
                raise ValueError('Duplicate ID: ' + key)
            target[key] = {'meta': meta, 'body': body, 'path': path}
    resources = read_json(ROOT / 'references/resources.json')['resources']
    return taxonomy, modules, projects, resources

def domain_dependencies(modules,projects=None):
    result={key:set(m['meta']['prerequisites']) | {leaf.split('.')[0] for leaf in m['meta']['prelude_objectives'] if leaf.split('.')[0]!=key}
            for key,m in modules.items()}
    for project in (projects or {}).values():
        meta=project['meta']; owner=meta.get('module')
        if owner in result:
            result[owner]|={leaf.split('.')[0] for leaf in meta.get('prelude_objectives',[]) if isinstance(leaf,str) and leaf.split('.')[0]!=owner}
    return result

def check_program_order(order, modules, projects=None):
    errors=[]
    if len(order)!=len(modules) or set(order)!=set(modules):
        errors.append('Program order must include every module exactly once')
    completed=set()
    for key in order:
        if key not in modules: continue
        missing=domain_dependencies(modules,projects)[key]-completed
        if missing: errors.append(key+': program order precedes foundations '+str(sorted(missing)))
        completed.add(key)
    return errors

def anchor(key):
    return re.sub(r'[^a-z0-9-]', '-', key.lower())

def total_hours(meta):
    return sum(meta['workload'].values())
