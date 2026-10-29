"""Generate the active contextual route and its lossless objective/migration maps.

The producer reads public canonical sources only. The canonical route source is
`curriculum/v5/routes/contextual-v2.source.json`; its authored episodes are
scheduled contiguously in authored order under the typed prerequisites, and
nodes that no episode places remain visible as `unauthored` project blocks
ordered by readiness. Encounter maps come only from authored encounters.

The predecessor `contextual-v1.json` and its `coverage.json`/`migration.json`
are frozen: this producer never writes them and `--check` verifies their pinned
SHA-256 identities. Historical catalog records are never edited. `--check`
compares expected output without modifying any file.
"""
from __future__ import annotations

import argparse
from collections import defaultdict
from copy import deepcopy
import hashlib
import json

from common import ROOT, load_taxonomy

BASE = 'curriculum/v5/routes/'
SOURCE = BASE + 'contextual-v2.source.json'
ROLES = ('introduction', 'derivation', 'implementation', 'integrated-reuse',
         'changed-assumption', 'independent-transfer')
HARD = {'concept', 'component'}


def read(name):
    return json.loads((ROOT/name).read_text(encoding='utf-8-sig'))


def sha256(name):
    return hashlib.sha256((ROOT/name).read_bytes()).hexdigest()


def output_names(route_id):
    """Successor outputs are named for their route; criterion resolutions are shared."""
    return {'route': route_id + '.json', 'coverage': route_id + '.coverage.json',
            'migration': route_id + '.migration.json', 'resolutions': 'criterion_resolutions.json'}


def project_files(area, suffixes):
    folder = ROOT/area
    return [p.relative_to(ROOT).as_posix() for p in sorted(folder.glob('*'))
            if p.is_file() and p.suffix in suffixes]


def prepare(source):
    """Build route nodes and typed prerequisites; this part has no ordering policy."""
    from native_successors import resolve
    units = read('curriculum/v5/units.json')['units']
    exercises = read('curriculum/v5/exercises.json')['exercises']
    project_entries = {}
    for unit in units:
        project_entries.setdefault(unit['project'], unit)
    artifacts = {r['id']: r for r in read('curriculum/v5/artifacts.json')['artifacts']}
    components = deepcopy(source.get('component_contracts', []))
    for artifact in components:
        if artifact['id'] in artifacts:
            raise ValueError('Active component must not replace historical identity')
        artifact['required_header_sha256'] = sha256(artifact['required_header'])
        artifacts[artifact['id']] = artifact
    domains = {d: c['id'] for c in source['contexts'] for d in c['domains']}
    resolutions = []
    for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-*.json')):
        resolutions += resolve(json.loads(path.read_text(encoding='utf-8-sig')), ROOT, source.get('successor_dispositions'))
    resolution_by_project = defaultdict(list)
    for row in resolutions:
        resolution_by_project[row['project']].append(row)
    supplied_policy = source['supplied_artifacts']
    suffixes = set(supplied_policy['suffixes'])
    nodes = []
    for kind, rows in [('unit', units), ('exercise', exercises)]:
        for value in rows:
            key = value['id']
            project = value['project']
            domain = value.get('domain') or (project.split('-')[1] if project.startswith('PR-T') else 'INTEGRATION')
            context = source['project_context'].get(project, domains.get(domain, 'integration'))
            required = value.get('requires', []) if kind == 'unit' else value.get('prerequisite_exercises', []) + value.get('prerequisite_units', [])
            edges = [dict(kind='concept', target=k, reason=value.get('requires_reasons', {}).get(k, 'Pass the referenced mechanism assessment before independent reuse.')) for k in required]
            policy = source.get('exercise_prerequisite_policy', {})
            if kind == 'exercise' and policy.get('inherit_parent_entry_upstream') and key not in policy.get('portable_core_exceptions', {}):
                parent = project_entries[project]
                for prior in parent.get('requires', []):
                    if prior not in required:
                        edges.append(dict(kind='concept', target=prior, derived_from=parent['id'],
                                          reason='Reuse the parent entry prerequisite: ' + parent.get('requires_reasons', {}).get(prior, 'assessed prior mechanism') + '. This is conceptual preparation for the extension; it does not import a hidden implementation.'))
            imports = value.get('imports', []) if kind == 'unit' else value.get('prerequisite_artifacts', [])
            for aid in imports:
                artifact = artifacts[aid]
                # Preserve inherited import gating: a later unit still carries its actual import.
                edges = [e for e in edges if e['target'] != artifact['producer_unit']]
                edges.append(dict(kind='component', target=artifact['producer_unit'], artifact=aid,
                                  contract_version=artifact['contract_version'], reason=artifact['public_contract']))
            family = value.get('family') if kind == 'unit' else value['lab']['family']
            if family:
                edges.append(dict(kind='environment', target='cc', reason='Build the component with an identified C17 compiler.'))
            contracts = []
            sources = ['curriculum/v5/units.json' if kind == 'unit' else value['brief'], 'curriculum/projects/' + project + '.md']
            assessment = ROOT/'curriculum/v5/expansion/legacy_assessments'/(project + '.json')
            if assessment.exists():
                data = json.loads(assessment.read_text(encoding='utf-8-sig'))
                contracts += data['shared_contracts']
                sources.append(assessment.relative_to(ROOT).as_posix())
            if kind == 'exercise':
                contracts += value['shared_contracts']
            supply = supplied_policy['projects'].get(project, {})
            contracts += supply.get('contracts', [])
            contracts += [name for name in supply.get('optional_contracts', []) if (ROOT/name).is_file()]
            supplied = []
            for area in supply.get('directories', []) + (supply.get('unit_directories', []) if kind == 'unit' else []):
                supplied += project_files(area, suffixes)
            supplied += supply.get('files', [])
            supplied += [a['required_header'] for a in components if a['producer_unit'] == key]
            supplied = list(dict.fromkeys(supplied))
            selected_resolutions = [r for r in resolution_by_project[project] if r['objective'] in value['objectives']]
            for resolution in selected_resolutions:
                contracts += resolution['public_contracts']
            contracts = list(dict.fromkeys(contracts))
            node = dict(id=key, kind=kind, project=project, title=value['title'], context=context,
                        objectives=value['objectives'], prerequisites=edges, contracts=contracts, sources=sources,
                        supplied_artifacts=supplied, criterion_resolutions=selected_resolutions,
                        required=value.get('required', value.get('status') != 'elective'),
                        assessment=dict(kind='existing-scoped-assessment',
                                        tests=value.get('tests', []) if kind == 'unit' else value['lab']['tests'],
                                        limits=value.get('remaining_evidence', 'Reasoning, independence and changed conditions require explicit evidence.')))
            if project in source.get('contract_scopes', {}):
                node['contract_scope'] = source['contract_scopes'][project]
            nodes.append(node)
    byid = {n['id']: n for n in nodes}
    for identifier, edges in source.get('additional_prerequisites', {}).items():
        if identifier not in byid:
            raise ValueError('Additional prerequisite names an unknown node: ' + identifier)
        byid[identifier]['prerequisites'] += deepcopy(edges)
    for group in source.get('exercise_concept_groups', []):
        for identifier in group['exercises']:
            if byid[identifier]['kind'] != 'exercise':
                raise ValueError('Exercise concept group names a baseline unit')
            for prior in group['requires']:
                if not any(edge['target'] == prior and edge['kind'] in HARD for edge in byid[identifier]['prerequisites']):
                    byid[identifier]['prerequisites'].append(dict(kind='concept', target=prior, reason=group['reason'], derived_from=group['source']))
    for entry in nodes:
        gates = {}
        for edge in entry['prerequisites']:
            gates.setdefault((edge['kind'], edge['target'], edge.get('artifact'), edge.get('contract_version')), edge)
        entry['prerequisites'] = list(gates.values())
        for edge in entry['prerequisites']:
            if edge['kind'] in HARD and edge['target'] not in byid:
                raise ValueError('Dangling prerequisite: ' + entry['id'] + ' -> ' + edge['target'])
    for identifier, increment in source.get('increments', {}).items():
        byid[identifier]['active_increment'] = increment
    return dict(nodes=nodes, artifacts=artifacts, components=components, resolutions=resolutions)


def hard_prerequisites(nodes):
    return {n['id']: {e['target'] for e in n['prerequisites'] if e['kind'] in HARD} for n in nodes}


def catalog_key(node):
    return (node['project'], 0 if node['kind'] == 'unit' else 1, node['id'])


def schedule(episodes, nodes):
    """Place episodes contiguously in authored order; unauthored blocks follow readiness.

    An unauthored ancestor that an episode needs is placed, as a project block,
    immediately before that episode. After the last episode the remaining
    unauthored nodes are placed in the order they became ready; ties use the
    subject catalog, and a project block continues while its next node is ready.
    Nothing balances or rotates context labels.
    """
    byid = {n['id']: n for n in nodes}
    hard = hard_prerequisites(nodes)
    owner = {}
    for episode in episodes:
        if not episode.get('construction'):
            raise ValueError('Episode has no construction: ' + episode['id'])
        for key in episode['construction']:
            if key not in byid:
                raise ValueError('Episode ' + episode['id'] + ' names an unknown node: ' + key)
            if key in owner:
                raise ValueError('Node ' + key + ' is constructed by two episodes: ' + owner[key] + ', ' + episode['id'])
            owner[key] = episode['id']
    order, done, placement = [], set(), {}
    dependents = defaultdict(list)
    for key, priors in hard.items():
        for prior in priors:
            dependents[prior].append(key)
    # Readiness time: the route position at which a node's last hard prerequisite was placed.
    ready_since = {key: 0 for key, priors in hard.items() if not priors}

    def place(key, where):
        order.append(key)
        done.add(key)
        placement[key] = where
        for later in dependents[key]:
            if later not in ready_since and hard[later] <= done:
                ready_since[later] = len(order)

    def closure(keys):
        result, stack = set(), list(keys)
        while stack:
            for prior in hard[stack.pop()]:
                if prior not in result:
                    result.add(prior)
                    stack.append(prior)
        return result

    def place_unauthored(required, needed_by):
        pending = set(required)
        current = None
        while pending:
            ready = [k for k in pending if k in ready_since]
            if not ready:
                raise ValueError('Route cycle/dangling prerequisites: ' + str({k: sorted(hard[k]-done) for k in sorted(pending)[:8]}))
            same = [k for k in ready if current is not None and byid[k]['project'] == current]
            choice = min(same or ready, key=lambda k: (0 if same else ready_since[k], catalog_key(byid[k])))
            current = byid[choice]['project']
            pending.remove(choice)
            place(choice, dict(kind='unauthored', block='unauthored:' + current, needed_by=needed_by))

    for episode in episodes:
        construction = episode['construction']
        missing = closure(construction) - done - set(construction)
        foreign = sorted(k for k in missing if k in owner)
        if foreign:
            raise ValueError('Episode ' + episode['id'] + ' requires ' + foreign[0] + ' from later episode ' + owner[foreign[0]])
        for key in sorted(missing):
            inside = sorted(hard[key] & set(construction))
            if inside:
                raise ValueError('Episode ' + episode['id'] + ' needs unauthored ' + key + ', which itself requires ' + inside[0] +
                                 ' from the same episode; author it into the construction or split the episode')
        place_unauthored(missing, episode['id'])
        for step, key in enumerate(construction, 1):
            unmet = sorted(hard[key] - done)
            if unmet:
                raise ValueError('Episode ' + episode['id'] + ' construction violates prerequisites: ' + key + ' requires ' + unmet[0])
            place(key, dict(kind='episode', episode=episode['id'], step=step, steps=len(construction)))
    place_unauthored(set(byid) - done, None)
    return order, placement


def criterion_universe(exercises):
    """Every assessable criterion with its objective, owning node scope and text."""
    universe = {}
    for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-*.json')):
        data = json.loads(path.read_text(encoding='utf-8-sig'))
        for c in data['criteria']:
            text = [c.get('title', ''), c.get('task', ''), c.get('purpose', '')] + list(c.get('reasoning', []))
            text += [str(v) for v in c.get('rubric', {}).values()] + list(c.get('hints', []))
            text += [v.get('task', '') for v in c.get('variants', [])]
            universe[c['id']] = dict(objective=c['objective'], project=data['project'], kind='legacy',
                                     source=path.relative_to(ROOT).as_posix(), text=' '.join(text))
    for x in exercises:
        for row in x['rubric']:
            universe[row['id']] = dict(objective=row['objective'], exercise=x['id'], project=x['project'], kind='exercise',
                                       source=x['brief'], text=' '.join([row['criterion']] + list(row['levels'].values())))
    return universe


def assemble(source, prepared):
    """Order nodes from authored episodes and bind authored encounters."""
    nodes = deepcopy(prepared['nodes'])
    byid = {n['id']: n for n in nodes}
    contexts = {c['id']: c for c in source['contexts']}
    episodes = source.get('episodes', [])
    episode_by_id = {e['id']: e for e in episodes}
    if len(episode_by_id) != len(episodes):
        raise ValueError('Duplicate episode identifier')
    order, placement = schedule(episodes, nodes)
    position = {k: i for i, k in enumerate(order)}
    hard = hard_prerequisites(nodes)
    consumers = defaultdict(list)
    for key in order:
        for prior in hard[key]:
            consumers[prior].append(key)
    encounters_by_node = defaultdict(list)
    for mechanism in source.get('mechanisms', []):
        for encounter in mechanism.get('encounters', []):
            if isinstance(encounter, dict) and encounter.get('node') in byid:
                encounters_by_node[encounter['node']].append((mechanism, encounter))
    for key in order:
        node = byid[key]
        where = placement[key]
        node['placement'] = where
        node['encounters'] = [dict(mechanism=m.get('id'), role=e.get('role'), objective=e.get('objective'), criterion=e.get('criterion'))
                              for m, e in encounters_by_node[key]]
        increment = node.get('active_increment')
        if where['kind'] == 'episode':
            episode = episode_by_id[where['episode']]
            note = episode.get('notes', {}).get(key)
            parts = [episode['title'] + ' (step ' + str(where['step']) + ' of ' + str(where['steps']) + ').']
            if where['step'] == 1:
                parts.append(episode['motivating_limitation'])
            if note:
                parts.append(note)
            elif where['step'] > 1:
                parts.append('This step continues the construction toward: ' + episode['exiting_artifact']['description'])
            for mechanism, encounter in encounters_by_node[key]:
                if encounter.get('criterion') in (None, 'missing') or encounter.get('role') not in ROLES or not encounter.get('deliverable'):
                    continue  # malformed or unbound encounters are reported by the validator, never narrated
                parts.append('As the ' + encounter['role'] + ' encounter for ' + str(mechanism.get('mechanism')) + ': ' + encounter['deliverable'])
            if increment:
                parts.append('This increment: ' + increment['task'])
            node['why_now'] = ' '.join(parts)
            node['why_now_kind'] = 'episode'
            construction = episode['construction']
            if where['step'] < where['steps']:
                following = construction[where['step']]
                node['next_consumer'] = following + ' continues this episode (' + byid[following]['title'] + ').'
                node['next_consumer_kind'] = 'episode'
            else:
                later = [e['id'] for e in episodes[episodes.index(episode)+1:]
                         if any(set(construction) & hard[k] for k in e['construction'])]
                exit_text = 'Exiting artifact: ' + episode['exiting_artifact']['description']
                if later:
                    node['next_consumer'] = exit_text + ' Consumed next by episode ' + ', '.join(later[:3]) + '.'
                    node['next_consumer_kind'] = 'episode'
                else:
                    node['next_consumer'] = exit_text + ' No later authored episode consumes it yet.'
                    node['next_consumer_kind'] = 'unauthored'
        else:
            if increment:
                node['why_now'] = increment['task']
                node['why_now_kind'] = 'increment'
            else:
                need = (' It is placed here because episode ' + where['needed_by'] + ' requires it.') if where['needed_by'] else ''
                node['why_now'] = ('Unauthored project block ' + node['project'] + ': no authored episode places this increment yet.' + need +
                                   ' Context: ' + contexts[node['context']]['problem'] + ' This increment: ' + node['title'] + '.')
                node['why_now_kind'] = 'generic'
            direct = consumers[key]
            node['next_consumer'] = (', '.join(direct[:4]) + ' (derived from prerequisite edges).') if direct else 'No later node depends on it; independent integrated defense in the ' + node['context'] + ' context.'
            node['next_consumer_kind'] = 'derived'
    universe = criterion_universe(read('curriculum/v5/exercises.json')['exercises'])
    mechanisms = []
    for mechanism in source.get('mechanisms', []):
        bound = deepcopy(mechanism)
        roles = {role: [] for role in ROLES}
        bound.setdefault('encounters', [])
        for encounter in bound['encounters']:
            if encounter.get('node') and encounter.get('role') in roles:
                roles[encounter['role']].append(encounter['node'])
            criterion = universe.get(encounter.get('criterion'))
            if criterion:
                encounter['criterion_source'] = criterion['source']
        bound['encounter_map'] = roles
        bound['bound_roles'] = sorted({e['role'] for e in bound['encounters'] if e.get('role') in ROLES and e.get('node') and e.get('criterion') not in (None, 'missing')}, key=ROLES.index)
        bound['missing'] = [dict(role=e.get('role'), node=e.get('node'), gap=e.get('gap', '')) for e in bound['encounters'] if e.get('criterion') in (None, 'missing')]
        mechanisms.append(bound)
    theory = deepcopy(source.get('theory', []))
    for topic in theory:
        for binding in topic.get('bindings', []):
            criterion = universe.get(binding.get('criterion'))
            if criterion:
                binding['criterion_source'] = criterion['source']
    episodes_out = []
    for episode in episodes:
        value = deepcopy(episode)
        span = [position[k] for k in episode['construction']]
        value['span'] = dict(first=min(span), last=max(span))
        episodes_out.append(value)
    blocks = []
    for key in order:
        where = placement[key]
        if where['kind'] != 'unauthored':
            continue
        if blocks and blocks[-1]['id'] == where['block'] and blocks[-1]['last'] == position[key]-1:
            blocks[-1]['nodes'].append(key)
            blocks[-1]['last'] = position[key]
        else:
            blocks.append(dict(id=where['block'], project=byid[key]['project'], status='unauthored', needed_by=where['needed_by'],
                               first=position[key], last=position[key], nodes=[key]))
    paths = []
    for path in source.get('paths', []):
        members = [k for e in path['episodes'] for k in episode_by_id[e]['construction']]
        paths.append(dict(path, units=members))
    stats = dict(episodes=len(episodes), nodes=len(order),
                 nodes_in_episodes=sum(1 for k in order if placement[k]['kind'] == 'episode'),
                 unauthored_nodes=sum(1 for k in order if placement[k]['kind'] == 'unauthored'),
                 unauthored_blocks=len(blocks),
                 why_now=dict((kind, sum(1 for n in nodes if n['why_now_kind'] == kind)) for kind in ('episode', 'increment', 'generic')),
                 encounters=sum(1 for m in mechanisms for e in m['encounters'] if e.get('role') in ROLES and e.get('node') and e.get('criterion') not in (None, 'missing')),
                 missing_bindings=sum(len(m['missing']) for m in mechanisms),
                 mechanisms_fully_bound=sum(1 for m in mechanisms if len(m['bound_roles']) == len(ROLES) and not m['missing']),
                 theory_topics_bound=sum(1 for t in theory if t.get('bindings')))
    names = output_names(source['id'])
    predecessor = source['predecessor']
    coverage = []
    entries_by_objective = defaultdict(list)
    for key in order:
        for objective in byid[key]['objectives']:
            entries_by_objective[objective].append(key)
    legacy_by_objective = defaultdict(list)
    for identifier, value in sorted(universe.items()):
        if value['kind'] == 'legacy':
            legacy_by_objective[value['objective']].append((identifier, value))
    bindings_by_objective = defaultdict(list)
    for m in mechanisms:
        for e in m['encounters']:
            if e.get('node'):
                bindings_by_objective[e.get('objective')].append(dict(mechanism=m.get('id'), role=e.get('role'), node=e['node'], criterion=e.get('criterion')))
    for domain in load_taxonomy():
        for topic in domain['subtopics']:
            for objective in topic['objectives']:
                entries = entries_by_objective[objective['id']]
                if not entries:
                    raise ValueError('Lost objective: ' + objective['id'])
                projects = {byid[k]['project'] for k in entries}
                criteria = [dict(id=c, source=v['source']) for c, v in legacy_by_objective[objective['id']] if v['project'] in projects]
                bindings = bindings_by_objective[objective['id']]
                coverage.append(dict(objective=objective['id'], text=objective['text'], encounters=entries, criteria=criteria,
                                     authored_bindings=bindings, contexts=sorted({byid[k]['context'] for k in entries}),
                                     disposition='retained', limits='Coverage links identify authored obligations, not completed assessments.'))
    old = read(predecessor['route'])
    old_position = {k: i for i, k in enumerate(old['order'])}
    old_nodes = {n['id']: n for n in old['nodes']}
    records = []
    for key in order:
        previous = old_nodes.get(key)
        before = {(e['kind'], e['target'], e.get('artifact')) for e in previous['prerequisites'] if e['kind'] in HARD} if previous else set()
        after = {(e['kind'], e['target'], e.get('artifact')) for e in byid[key]['prerequisites'] if e['kind'] in HARD}
        records.append(dict(id=key, successor=key, disposition='retained' if previous else 'added',
                            prerequisites='unchanged' if before == after else 'changed',
                            previous_position=old_position.get(key), position=position[key], placement=placement[key],
                            acceptance='Retain the original contract and receipt; adoption records a route-migrated event and grants no acceptance.'))
    migration = dict(schema_version=1, from_route=predecessor['id'], to_route=source['id'], records=records,
                     removed=sorted(set(old_nodes) - set(byid)), objective_ids=[c['objective'] for c in coverage],
                     ledger_policy='Do not rewrite old ledger entries. Profiles pinned to ' + predecessor['id'] + ' keep that frozen route until `study route PROFILE --adopt` appends a route-migrated event. New profiles select ' + source['id'] + '.')
    pilots = [dict(id=p['id'], teaching=p['teaching'], units=p['units']) for p in paths]
    route = dict(schema_version=2, id=source['id'], catalog='curriculum/v5', contexts=source['contexts'], nodes=nodes, order=order,
                 coverage=BASE + names['coverage'], migration=BASE + names['migration'],
                 predecessor=dict(id=predecessor['id'], route=predecessor['route']),
                 episodes=episodes_out, blocks=blocks, mechanisms=mechanisms, theory=theory, paths=paths, pilots=pilots,
                 component_contracts=prepared['components'], statistics=stats, policy=source['policy'],
                 semantic_review=dict(record=BASE + 'semantic-review.md', status='authored-review-with-explicit-remaining-work'))
    return {names['resolutions']: dict(schema_version=1, criteria=prepared['resolutions'],
                                       limits='Exact explicit successor links resolved to public teaching; unresolved metadata bindings remain visible.'),
            names['route']: route, names['coverage']: dict(schema_version=1, route=source['id'], objectives=coverage),
            names['migration']: migration}


def generate(source=None):
    source = read(SOURCE) if source is None else source
    return assemble(source, prepare(source))


def frozen_errors(source=None):
    """The predecessor route and its maps must remain byte-for-byte unchanged."""
    source = read(SOURCE) if source is None else source
    errors = []
    for name, expected in source['predecessor']['frozen_sha256'].items():
        if not (ROOT/name).is_file() or sha256(name) != expected:
            errors.append('Frozen predecessor changed: ' + name)
    return errors


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    errors = frozen_errors()
    for name, value in generate().items():
        text = json.dumps(value, ensure_ascii=False, indent=2) + '\n'
        path = ROOT/BASE/name
        if args.check:
            if not path.exists() or path.read_text(encoding='utf-8') != text:
                errors.append('Stale generated route: ' + name)
        else:
            path.write_text(text, encoding='utf-8')
    if errors:
        raise SystemExit('\n'.join(errors))
    print('Active route generation ' + ('verified' if args.check else 'complete'))


if __name__ == '__main__':
    main()
