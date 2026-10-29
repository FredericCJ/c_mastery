"""Public route integrity and rejection regressions; never claims pedagogy passed."""
from __future__ import annotations
from copy import deepcopy
from contextlib import redirect_stdout
import hashlib
import io
import json
import os
import re
from pathlib import Path
import tempfile
import subprocess
import shutil
import sys
import unittest
from unittest.mock import patch
from common import ROOT, load_taxonomy
from cmastery import cli, learning, route
from generate_native_route import (generate, prepare, assemble, read, frozen_errors, criterion_universe,
                                   BASE, SOURCE, ROLES, HARD)
from native_teaching_revision import qualifies

THEORY_TOPICS = ('loop-invariants', 'termination', 'recurrences', 'amortized-analysis', 'greedy-exchange',
                 'dp-recurrence-design', 'graph-correctness', 'probabilistic-assumptions')


def errors(value):
    failures=[]
    entries={n['id']:n for n in value['nodes']}
    if len(entries)!=len(value['nodes']):failures.append('duplicate entry')
    if len(value['order'])!=len(set(value['order'])) or set(value['order'])!=set(entries):failures.append('unreachable or duplicated work')
    position={key:i for i,key in enumerate(value['order'])}
    contexts={c['id'] for c in value['contexts']}
    catalog=learning.catalog()
    for entry in value['nodes']:
        if entry['context'] not in contexts:failures.append('unknown context')
        for edge in entry['prerequisites']:
            if edge['kind'] not in {'concept','component','environment','preparation'}:failures.append('unknown prerequisite type')
            if edge['kind'] in {'concept','component'}:
                if edge['target'] not in entries:failures.append('dangling prerequisite')
                elif position.get(edge['target'],1e9)>=position.get(entry['id'],-1):failures.append('hard cycle or consumer before producer')
            if edge['kind']=='component':
                art=catalog['artifacts'].get(edge['artifact'])
                if art is None or edge['target']!=art['producer_unit'] or str(edge['contract_version'])!=str(art['contract_version']):failures.append('component contract mismatch')
        for name in entry['sources']+entry['contracts']:
            try:
                if not route.public_path(ROOT,name).is_file():failures.append('missing public input')
            except ValueError:failures.append('unsafe public input')
    return failures


REJECTION_MIN_WORDS = 5


def normalized(text):
    return ' '.join(str(text).lower().split())


def words(text):
    return re.findall(r"[a-z0-9]+(?:[-'/.][a-z0-9]+)*", normalized(text))


def names_rejection(rejects, criterion_text):
    """The rejected answer must be a whole sentence of the criterion text, or a
    word-aligned span of at least REJECTION_MIN_WORDS words; a bare substring
    such as 'a' or a fragment cut inside a word does not name an answer."""
    wanted = words(rejects)
    if not wanted:
        return False
    sentences = [words(part) for part in re.split(r'(?<=[.;:!?])\s+', normalized(criterion_text))]
    if wanted in sentences:
        return True
    if len(wanted) < REJECTION_MIN_WORDS:
        return False
    text = words(criterion_text)
    return any(text[i:i + len(wanted)] == wanted for i in range(len(text) - len(wanted) + 1))


def binding_errors(value, criteria=None, artifacts=None):
    """Reject inconsistent authored episodes, encounter bindings and theory criteria.

    A criterion is attached to an exercise only through its own rubric, and to
    a unit when it is a legacy criterion of the unit's project or (for an
    executable unit whose project has no legacy assessment) the rubric of an
    exercise of the same project. Structural agreement is not evidence that learners mastered
    anything; it only rejects bindings that cannot be true.
    """
    failures = []
    def fail(message, where):
        failures.append(message + ': ' + where)
    criteria = criterion_universe(read('curriculum/v5/exercises.json')['exercises']) if criteria is None else criteria
    artifacts = learning.catalog()['artifacts'] if artifacts is None else artifacts
    legacy_projects = {c['project'] for c in criteria.values() if c['kind'] == 'legacy'}
    nodes = {n['id']: n for n in value['nodes']}
    position = {key: i for i, key in enumerate(value['order'])}
    hard = {k: {e['target'] for e in n['prerequisites'] if e['kind'] in HARD} for k, n in nodes.items()}
    edges = {k: {e['artifact'] for e in n['prerequisites'] if e['kind'] == 'component'} for k, n in nodes.items()}
    memo = {}
    def ancestors(key):
        if key not in memo:
            result, stack = set(), [key]
            while stack:
                for prior in hard.get(stack.pop(), ()):
                    if prior not in result:
                        result.add(prior)
                        stack.append(prior)
            memo[key] = result
        return memo[key]
    def produced_before(component, limit):
        artifact = artifacts.get(component)
        return artifact is not None and position.get(artifact['producer_unit'], 1e9) < limit
    def attached(criterion, node):
        if node['kind'] == 'exercise':
            return criterion['kind'] == 'exercise' and criterion['exercise'] == node['id']
        if criterion['kind'] == 'legacy':
            return criterion['project'] == node['project']
        return criterion['project'] == node['project'] and node['project'] not in legacy_projects
    def check_criterion(identifier, objective, node, where):
        criterion = criteria.get(identifier)
        if criterion is None:
            fail('criterion does not exist', where)
            return None
        if criterion['objective'] != objective:
            fail('criterion objective differs', where)
        if not attached(criterion, node):
            fail('criterion not attached to node', where)
        return criterion
    owner = {}
    for episode in value.get('episodes', []):
        where = episode.get('id', '?')
        for field in ('title', 'motivating_limitation', 'transition_reason'):
            if not str(episode.get(field, '')).strip():
                fail('episode missing field ' + field, where)
        if not str(episode.get('entering_state', {}).get('summary', '')).strip() or not str(episode.get('exiting_artifact', {}).get('description', '')).strip():
            fail('episode missing entering state or exiting artifact', where)
        construction = episode.get('construction', [])
        if not construction or any(key not in nodes for key in construction):
            fail('episode names unknown node', where)
            continue
        for key in construction:
            if key in owner:
                fail('node constructed by two episodes', key)
            owner[key] = where
        spots = [position[key] for key in construction]
        if spots != list(range(spots[0], spots[0] + len(spots))):
            fail('episode not contiguous in authored order', where)
        first = min(spots)
        for key in construction:
            for prior in hard[key]:
                if position.get(prior, 1e9) >= position[key]:
                    fail('episode construction violates prerequisites', key + ' requires ' + prior)
            placement = nodes[key].get('placement', {})
            if placement.get('episode') != where or nodes[key].get('why_now_kind') != 'episode':
                fail('episode placement or why_now disagrees with construction', key)
        for component in episode.get('produces', []):
            artifact = artifacts.get(component)
            if artifact is None or artifact['producer_unit'] not in construction:
                fail('episode produces a component it does not construct', where + ' ' + component)
        for component in episode.get('consumes', []):
            if not any(component in edges[key] for key in construction) or not produced_before(component, first):
                fail('episode consumes a component without a prior producer edge', where + ' ' + component)
        for component in episode.get('entering_state', {}).get('components', []):
            if not produced_before(component, first):
                fail('episode entering component not yet produced', where + ' ' + component)
        for concept in episode.get('entering_state', {}).get('concepts', []):
            if position.get(concept, 1e9) >= first:
                fail('episode entering concept not yet placed', where + ' ' + concept)
    for key, node in nodes.items():
        kind = node.get('placement', {}).get('kind')
        if key not in owner and (kind == 'episode' or node.get('why_now_kind') == 'episode'):
            fail('why_now claims an episode that does not construct the node', key)
        if kind == 'unauthored' and node.get('why_now_kind') not in {'generic', 'increment'}:
            fail('unauthored node without generic or increment why_now', key)
    for mechanism in value.get('mechanisms', []):
        name = mechanism.get('id', '?')
        form = mechanism.get('forms', {})
        if not all(str(form.get(k, '')).strip() for k in ('model', 'scalable', 'consumer', 'changes')):
            fail('mechanism forms incomplete', name)
        encounters = mechanism.get('encounters', [])
        for role in ROLES:
            if not any(e.get('role') == role for e in encounters):
                fail('mechanism role absent', name + ' ' + role)
        seen = {}
        bound = []
        for encounter in encounters:
            role, key = encounter.get('role'), encounter.get('node')
            where = name + ' ' + str(role) + ' ' + str(key)
            if role not in ROLES:
                fail('unknown encounter role', where)
                continue
            if not str(encounter.get('deliverable', '')).strip():
                fail('encounter without deliverable', where)
            if 'criterion' not in encounter:
                fail('encounter without criterion', where)
            missing = encounter.get('criterion') in (None, 'missing')
            if missing and not str(encounter.get('gap', '')).strip():
                fail('missing binding without gap', where)
            if key is None:
                if not missing:
                    fail('bound encounter without node', where)
                continue
            if key not in nodes:
                fail('encounter names unknown node', where)
                continue
            if key in seen:
                fail('overlapping roles within a mechanism', where + ' also ' + seen[key])
            seen[key] = role
            node = nodes[key]
            if encounter.get('objective') not in node['objectives']:
                fail('encounter objective not on node', where)
            if role == 'changed-assumption' and not str(encounter.get('assumption_change', '')).strip():
                fail('changed assumption without assumption change', where)
            for component in encounter.get('produces', []):
                artifact = artifacts.get(component)
                if artifact is None or artifact['producer_unit'] != key:
                    fail('encounter produces disagrees with component contract', where + ' ' + component)
            for component in encounter.get('consumes', []):
                if component not in edges[key]:
                    fail('encounter consumes disagrees with component contract', where + ' ' + component)
            for prior in encounter.get('prerequisites', []):
                if position.get(prior, 1e9) >= position[key]:
                    fail('encounter prerequisite not earlier', where + ' ' + prior)
            if not missing:
                check_criterion(encounter['criterion'], encounter.get('objective'), node, where)
                bound.append(encounter)
        ordered = sorted(bound, key=lambda e: position[e['node']])
        for earlier, later in zip(ordered, ordered[1:]):
            if ROLES.index(later['role']) < ROLES.index(earlier['role']):
                fail('misordered roles within a mechanism', name + ' ' + earlier['node'] + ' ' + earlier['role'] + ' before ' + later['node'] + ' ' + later['role'])
        constructions = {e['node'] for e in bound if e['role'] in {'implementation', 'integrated-reuse'}}
        for component in mechanism.get('produces', []):
            artifact = artifacts.get(component)
            if artifact is None or artifact['producer_unit'] not in {e.get('node') for e in encounters}:
                fail('mechanism produces a component none of its encounters constructs', name + ' ' + component)
        for encounter in encounters:
            key = encounter.get('node')
            if encounter.get('role') != 'independent-transfer' or key not in nodes:
                continue
            if not (ancestors(key) & constructions) and not (edges[key] & set(mechanism.get('produces', []))):
                fail('transfer does not consume the mechanism', name + ' ' + key)
    episode_ids = {e.get('id') for e in value.get('episodes', [])}
    for path in value.get('paths', []):
        if not path.get('episodes') or any(e not in episode_ids for e in path['episodes']):
            fail('path names an unknown episode', str(path.get('id')))
    topics = {t.get('topic'): t for t in value.get('theory', [])}
    for topic in THEORY_TOPICS:
        if not topics.get(topic, {}).get('bindings'):
            fail('theory topic unbound', topic)
    for topic in value.get('theory', []):
        if not str(topic.get('reasoning', '')).strip():
            fail('theory topic without reasoning', str(topic.get('topic')))
        for binding in topic.get('bindings', []):
            where = str(topic.get('topic')) + ' ' + str(binding.get('node')) + ' ' + str(binding.get('criterion'))
            node = nodes.get(binding.get('node'))
            if node is None:
                fail('theory binding names unknown node', where)
                continue
            if binding.get('objective') not in node['objectives']:
                fail('encounter objective not on node', where)
            criterion = check_criterion(binding.get('criterion'), binding.get('objective'), node, where)
            if criterion is not None and not names_rejection(binding.get('rejects', ''), criterion['text']):
                fail('theory rejection not named by criterion', where)
    return failures


class AuthoredBindingRejection(unittest.TestCase):
    """Deliberately broken sources must be rejected by the generator or validator."""

    @classmethod
    def setUpClass(cls):
        cls.source=read(SOURCE)
        cls.prepared=prepare(cls.source)
        cls.criteria=criterion_universe(read('curriculum/v5/exercises.json')['exercises'])
        cls.artifacts=learning.catalog()['artifacts']

    def rejected(self, mutate, expected):
        source=deepcopy(self.source)
        mutate(source)
        route_value=assemble(source,self.prepared)['contextual-v2.json']
        failures=binding_errors(route_value,self.criteria,self.artifacts)
        self.assertTrue(any(f.startswith(expected) for f in failures),failures[:5])

    def mechanism(self, source, name):
        return next(m for m in source['mechanisms'] if m['id']==name)

    def encounter(self, source, name, role):
        return next(e for e in self.mechanism(source,name)['encounters'] if e['role']==role)

    def test_valid_source_has_no_binding_errors(self):
        self.assertEqual(binding_errors(assemble(self.source,self.prepared)['contextual-v2.json'],self.criteria,self.artifacts),[])

    def test_overlapping_roles_reject(self):
        def mutate(source):
            extra=deepcopy(self.encounter(source,'arrays-ordered-search','derivation'));extra['role']='implementation'
            self.mechanism(source,'arrays-ordered-search')['encounters'].append(extra)
        self.rejected(mutate,'overlapping roles within a mechanism')

    def test_misordered_roles_reject(self):
        def mutate(source):
            first=self.encounter(source,'arrays-ordered-search','introduction')
            last=self.encounter(source,'arrays-ordered-search','implementation')
            first['role'],last['role']=last['role'],first['role']
        self.rejected(mutate,'misordered roles within a mechanism')

    def test_objective_not_on_node_rejects(self):
        self.rejected(lambda s:self.encounter(s,'rings-incremental-parsing','derivation').update(objective='T14.06.c'),
                      'criterion objective differs')
        self.rejected(lambda s:self.encounter(s,'rings-incremental-parsing','derivation').update(objective='T15.01.a'),
                      'encounter objective not on node')

    def test_unknown_or_mismatched_criterion_rejects(self):
        self.rejected(lambda s:self.encounter(s,'graphs-worklists','implementation').update(criterion='AX-PR-T15-05-T15.99.z'),
                      'criterion does not exist')
        self.rejected(lambda s:self.encounter(s,'graphs-worklists','implementation').update(criterion='AX-PR-T15-05-T15.08.a'),
                      'criterion objective differs')
        self.rejected(lambda s:self.encounter(s,'graphs-worklists','implementation').update(criterion='AX-PR-T14-05-T14.08.b',objective='T15.08.b'),
                      'criterion objective differs')
        # A parent-project legacy criterion with the right objective is not an exercise's own rubric.
        self.rejected(lambda s:self.encounter(s,'stable-ordering','implementation').update(criterion='AX-PR-T15-01-T15.02.b'),
                      'criterion not attached to node')

    def test_missing_binding_requires_gap(self):
        self.rejected(lambda s:self.encounter(s,'dp-greedy','implementation').pop('gap'),'missing binding without gap')

    def test_changed_assumption_requires_assumption_change(self):
        self.rejected(lambda s:self.encounter(s,'deadline-collections','changed-assumption').pop('assumption_change'),
                      'changed assumption without assumption change')

    def test_transfer_must_consume_the_mechanism(self):
        self.rejected(lambda s:self.encounter(s,'hashing-symbol-identity','independent-transfer').update(
            node='X-T76-010',objective='T14.07.b',criterion='X-T76-010-R1'),'transfer does not consume the mechanism')

    def test_every_role_must_be_bound_or_explicitly_missing(self):
        def mutate(source):
            mechanism=self.mechanism(source,'numerical-kernels')
            mechanism['encounters']=[e for e in mechanism['encounters'] if e['role']!='independent-transfer']
        self.rejected(mutate,'mechanism role absent')

    def test_episode_construction_violating_prerequisites_rejects(self):
        source=deepcopy(self.source)
        episode=next(e for e in source['episodes'] if e['id']=='EP-byte-ring')
        episode['construction'].reverse()
        with self.assertRaisesRegex(ValueError,'construction violates prerequisites'):
            assemble(source,self.prepared)
        source=deepcopy(self.source)
        source['episodes']=[e for e in source['episodes'] if e['id']!='EP-sequence-representations']+[e for e in source['episodes'] if e['id']=='EP-sequence-representations']
        with self.assertRaisesRegex(ValueError,'from later episode'):
            assemble(source,self.prepared)
        value=assemble(self.source,self.prepared)['contextual-v2.json']
        order=value['order'];a=order.index('U-T14-01-03');b=order.index('U-T14-01-04')
        order[a],order[b]=order[b],order[a]
        failures=binding_errors(value,self.criteria,self.artifacts)
        self.assertTrue(any(f.startswith('episode construction violates prerequisites') for f in failures))

    def test_components_must_agree_with_contracts(self):
        self.rejected(lambda s:next(e for e in s['episodes'] if e['id']=='EP-byte-ring')['produces'].append('memory.pool'),
                      'episode produces a component it does not construct')
        self.rejected(lambda s:next(e for e in s['episodes'] if e['id']=='EP-byte-ring')['consumes'].append('sync.spsc'),
                      'episode consumes a component without a prior producer edge')
        self.rejected(lambda s:next(e for e in s['episodes'] if e['id']=='EP-checked-extents')['entering_state']['components'].append('containers.ring'),
                      'episode entering component not yet produced')
        self.rejected(lambda s:self.encounter(s,'rings-incremental-parsing','implementation')['produces'].append('sync.spsc'),
                      'encounter produces disagrees with component contract')
        self.rejected(lambda s:self.encounter(s,'rings-incremental-parsing','derivation')['consumes'].append('sync.spsc'),
                      'encounter consumes disagrees with component contract')

    def test_theory_rejection_must_be_named_by_its_criterion(self):
        self.rejected(lambda s:s['theory'][0]['bindings'][0].update(rejects='An invariant only needs to hold at loop exit.'),
                      'theory rejection not named by criterion')
        self.rejected(lambda s:s['theory'].pop(),'theory topic unbound')

    def test_theory_rejection_must_be_a_sentence_or_long_word_aligned_span(self):
        def set_rejects(text):
            def mutate(source):
                topic=next(t for t in source['theory'] if t['topic']=='graph-correctness')
                topic['bindings'][0]['rejects']=text
            return mutate
        for text in ['x','a','Dijkstra is correct','ijkstra is correct whenever no negative cycle','']:
            with self.subTest(rejects=text):
                self.rejected(set_rejects(text),'theory rejection not named by criterion')
        self.assertTrue(names_rejection('Dijkstra is correct whenever no negative cycle exists.',
                                        'Purpose. Dijkstra is correct whenever no negative cycle exists. Other text.'))
        self.assertTrue(names_rejection('treats odd n as equal halves','States T(n) but treats odd n as equal halves (for example)'))
        self.assertFalse(names_rejection('odd n as equal','States T(n) but treats odd n as equal halves'))

    def test_malformed_encounters_are_reported_not_crashes(self):
        self.rejected(lambda s:self.encounter(s,'graphs-worklists','derivation').update(role='bogus'),'unknown encounter role')
        self.rejected(lambda s:self.encounter(s,'graphs-worklists','implementation').pop('criterion'),'encounter without criterion')
        self.rejected(lambda s:self.encounter(s,'graphs-worklists','implementation').pop('deliverable'),'encounter without deliverable')
        self.rejected(lambda s:self.mechanism(s,'graphs-worklists')['encounters'].append({'role':'derivation','node':'U-T15-05-03'}),
                      'encounter without criterion')

    def test_why_now_cannot_claim_an_unauthored_episode(self):
        value=assemble(self.source,self.prepared)['contextual-v2.json']
        entry=next(n for n in value['nodes'] if n['placement']['kind']=='unauthored')
        entry['why_now_kind']='episode'
        failures=binding_errors(value,self.criteria,self.artifacts)
        self.assertTrue(any(f.startswith('why_now claims an episode') for f in failures))


class RouteIntegrity(unittest.TestCase):
    """Checks hold whether edition.json selects contextual-v1 or contextual-v2.

    v2 content is loaded by identifier, so it is verified before and after the
    active-route switch; behaviour that depends on the switch uses the active id.
    """
    @classmethod
    def setUpClass(cls):
        cls.active=route.load()
        cls.v2=route.load(route_id='contextual-v2')
        cls.v1=route.load(route_id='contextual-v1')

    def test_active_route_is_a_known_route(self):
        self.assertIn(self.active['id'],{'contextual-v1','contextual-v2'})
        self.assertEqual(self.v2['id'],'contextual-v2')
        self.assertEqual(self.v1['id'],'contextual-v1')

    def test_canonical_generated_outputs_and_all_objectives(self):
        for name,value in generate().items():
            actual=json.loads((ROOT/BASE/name).read_text())
            self.assertEqual(route.digest(actual),route.digest(value),"Stale generated output: "+name)
        self.assertEqual(errors(self.v2),[])
        self.assertEqual(errors(self.active),[])
        wanted={o['id'] for d in load_taxonomy() for t in d['subtopics'] for o in t['objectives']}
        coverage=json.loads((ROOT/self.v2['coverage']).read_text())['objectives']
        self.assertEqual(wanted,{r['objective'] for r in coverage})
        self.assertTrue(all(r['encounters'] for r in coverage))
        migration=json.loads((ROOT/self.v2['migration']).read_text())
        self.assertEqual({n['id'] for n in self.v2['nodes']},{n['id'] for n in migration['records']})
        self.assertEqual(wanted,set(migration['objective_ids']))
        self.assertEqual(migration['from_route'],'contextual-v1')

    def test_predecessor_route_is_frozen_and_node_identities_are_unchanged(self):
        self.assertEqual(frozen_errors(),[])
        self.assertEqual({n['id'] for n in self.v1['nodes']},{n['id'] for n in self.v2['nodes']})
        migration=json.loads((ROOT/self.v2['migration']).read_text())
        self.assertEqual(migration['removed'],[])
        self.assertTrue(all(r['disposition']=='retained' and r['prerequisites']=='unchanged' for r in migration['records']))
        def gates(value):
            return {n['id']:{(e['kind'],e['target'],e.get('artifact')) for e in n['prerequisites'] if e['kind'] in HARD} for n in value['nodes']}
        self.assertEqual(gates(self.v1),gates(self.v2))
        source=read(SOURCE)
        changed=deepcopy(source);changed['predecessor']['frozen_sha256'][BASE+'contextual-v1.json']='0'*64
        self.assertIn('Frozen predecessor changed: '+BASE+'contextual-v1.json',frozen_errors(changed))

    def test_readme_counts_match_generated_statistics(self):
        stats=self.v2['statistics'];text=' '.join((ROOT/BASE/'README.md').read_text().split())
        expected=[f"{stats['episodes']} episodes place {stats['nodes_in_episodes']} of the {stats['nodes']:,}",
                  f"{stats['encounters']} encounters are bound and {stats['missing_bindings']} remain",
                  f"{stats['mechanisms_fully_bound']} of 13 mechanisms"]
        for phrase in expected:self.assertIn(phrase,text)

    def test_each_route_matches_its_schema_version(self):
        for value,name in [(self.v1,'schema.json'),(self.v2,'schema-v2.json')]:
            schema=json.loads((ROOT/BASE/name).read_text())
            version=schema['properties']['schema_version']
            self.assertIn(value['schema_version'],version.get('enum',[version.get('const')]),name)
            self.assertFalse(set(schema['required'])-set(value),name)
            required=schema['properties']['nodes']['items']['required']
            self.assertTrue(all(set(required)<=set(n) for n in value['nodes']),name)
        v1_schema=json.loads((ROOT/BASE/'schema.json').read_text())
        self.assertEqual(v1_schema['properties']['schema_version'],{'const':1})

    def test_v1_cards_inline_only_frozen_strand_pages(self):
        """Every page a v1 card inlines is pinned, so v1 card identity cannot drift
        through route teaching; v2 teaches from separate pages."""
        frozen=read(SOURCE)['predecessor']['frozen_sha256']
        v1_pages={c['teaching'] for c in self.v1['contexts']}
        v2_pages={c['teaching'] for c in self.v2['contexts']}
        self.assertTrue(v1_pages<=set(frozen),sorted(v1_pages-set(frozen)))
        self.assertFalse(v1_pages&v2_pages)
        for name in v1_pages:
            self.assertEqual(hashlib.sha256((ROOT/name).read_bytes()).hexdigest(),frozen[name],name)
        card=route.render_node('U-T14-01-03',route_id='contextual-v1')
        self.assertIn((ROOT/BASE/'streaming.md').read_text().splitlines()[0][2:],card)
        self.assertNotIn('In `contextual-v2`',card)
        source=read(SOURCE)
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);page=root/BASE/'streaming.md';page.parent.mkdir(parents=True)
            page.write_bytes((ROOT/BASE/'streaming.md').read_bytes()+b'\nEdited.\n')
            with patch('generate_native_route.ROOT',root):
                self.assertIn('Frozen predecessor changed: '+BASE+'streaming.md',frozen_errors(source))

    def test_why_now_comes_from_authored_text_or_is_flagged_generic(self):
        for entry in self.v2['nodes']:
            kind=entry['placement']['kind']
            if kind=='episode':
                episode=next(e for e in self.v2['episodes'] if e['id']==entry['placement']['episode'])
                self.assertEqual(entry['why_now_kind'],'episode')
                self.assertTrue(entry['why_now'].startswith(episode['title']))
                if entry['placement']['step']==1:self.assertIn(episode['motivating_limitation'],entry['why_now'])
                else:self.assertTrue(entry['id'] in episode['notes'] or episode['exiting_artifact']['description'] in entry['why_now'])
            else:
                self.assertIn(entry['why_now_kind'],{'generic','increment'})
                if entry['why_now_kind']=='generic':self.assertIn('Unauthored project block',entry['why_now'])
        stats=self.v2['statistics']
        self.assertEqual(stats['why_now']['episode'],stats['nodes_in_episodes'])
        # Authored order: no least-recently-used context rotation remains.
        context={n['id']:n['context'] for n in self.v2['nodes']}
        switches=sum(1 for a,b in zip(self.v2['order'],self.v2['order'][1:]) if context[a]!=context[b])
        self.assertLess(switches,len(self.v2['order'])//2)

    def test_authored_bindings_are_consistent(self):
        self.assertEqual(binding_errors(self.v2),[])
        stats=self.v2['statistics']
        self.assertEqual(stats['episodes'],len(self.v2['episodes']))
        self.assertEqual(len(self.v2['mechanisms']),13)
        self.assertEqual(stats['theory_topics_bound'],len(THEORY_TOPICS))
        for path in self.v2['paths']:
            self.assertTrue(path['episodes'] and path['units'])

    def test_cycles_dangling_order_lost_work_reject(self):
        bad=deepcopy(self.active);bad['nodes'][0]['prerequisites'].append(dict(kind='concept',target='MISSING',reason='negative fixture'))
        self.assertIn('dangling prerequisite',errors(bad))
        bad=deepcopy(self.active);key=bad['nodes'][0]['id'];bad['nodes'][0]['prerequisites'].append(dict(kind='concept',target=key,reason='negative fixture'))
        self.assertIn('hard cycle or consumer before producer',errors(bad))
        bad=deepcopy(self.active);bad['order'].pop()
        self.assertIn('unreachable or duplicated work',errors(bad))

    def test_component_exists_is_not_assessed_or_exported(self):
        entry={'prerequisites':[dict(kind='component',target='producer',artifact='bytes',contract_version=2)]}
        self.assertEqual(route.blockers(entry,set(),{'bytes'}),['assessment:producer'])
        self.assertEqual(route.blockers(entry,{'producer'},set()),['export:bytes@2'])
        self.assertEqual(route.blockers(entry,{'producer'},{'bytes'}),[])
        self.assertTrue(route.blockers({'prerequisites':[dict(kind='environment',target='unavailable-tool')]},set()))
        self.assertEqual(route.blockers({'prerequisites':[dict(kind='preparation',target='optional')]},set()),[])

    def test_pilot_order_has_real_producers_before_representation_changes(self):
        position={key:i for i,key in enumerate(self.active['order'])}
        sequence=['LAB-HS-03','U-T14-05-01','U-T14-05-04','U-T24-05-02','LAB-RZ-05','LAB-RZ-06','U-T24-05-03']
        self.assertEqual(sorted(sequence,key=position.get),sequence)
        for producer in ['U-T15-05-04','U-T14-01-04','U-T14-05-04']:
            self.assertLess(position[producer],position['U-T36-07-01'])
        self.assertLess(position['U-T55-01-03'],position['U-T43-09-03'])

    def test_foundation_follows_c_bounds(self):
        pos={k:i for i,k in enumerate(self.active['order'])}
        for prior in ['U-T01-03-03','U-T03-02-03','U-T04-01-04']:
            self.assertLess(pos[prior],pos['U-T01-01-01'])

    def test_successor_semantic_prerequisites_are_actual_ancestors(self):
        nodes={n['id']:n for n in self.active['nodes']}
        def ancestors(key):
            result=set()
            def visit(current):
                for edge in nodes[current]['prerequisites']:
                    if edge['kind'] in {'concept','component'} and edge['target'] not in result:
                        result.add(edge['target']);visit(edge['target'])
            visit(key);return result
        for prior in ['U-T24-03-03','X-T15-019','U-T55-01-03','U-T24-05-03']:
            self.assertIn(prior,ancestors('U-T43-09-03'))
        for prior in ['X-T03-001','U-T03-01-03','U-T02-01-03']:
            self.assertIn(prior,ancestors('U-T03-02-01'))
        text=route.render_node('U-T43-09-02')
        self.assertIn('supplied reference adapters',text)
        self.assertIn('deferred to U-T43-09-03',text)

    def test_fresh_profile_cannot_offer_advanced_expansion_work(self):
        advanced={'X-T50-001','X-T51-001','X-T52-001','X-T43-001','X-T14-003','X-T14-004'}
        with tempfile.TemporaryDirectory() as folder:
            learning.study_init(folder)
            packet=learning.study_next(folder)
            self.assertFalse(advanced & {x['id'] for x in packet['available_exercises']})
            self.assertTrue(packet['available_units'])
        for entry in self.active['nodes']:
            if entry['kind']=='exercise':
                self.assertTrue(route.blockers(entry,set(),capabilities=route.capabilities()),entry['id'])

    def test_expansion_gates_retain_actual_mechanisms_and_early_producers(self):
        nodes={n['id']:n for n in self.active['nodes']}
        def targets(key):return {e['target'] for e in nodes[key]['prerequisites'] if e['kind'] in {'concept','component'}}
        for exercise,prior in [('X-T50-001','U-T50-05-03'),('X-T51-001','U-T51-01-03'),
                               ('X-T52-001','U-T52-03-04'),('X-T43-001','U-T43-08-03')]:
            self.assertIn(prior,targets(exercise))
            text=route.render_node(exercise)
            self.assertNotIn('Required baseline units: none',text)
            self.assertIn(prior,re.search(r'(?m)^Required baseline units: (.*)$',text)[1])
        for exercise in ['X-T14-003','X-T14-004']:
            self.assertTrue({'U-T14-01-04','U-T03-02-03','U-T01-01-02'}<=targets(exercise))
        self.assertIn('U-T14-06-03',targets('X-T14-005'))
        self.assertIn('U-T15-03-03',targets('X-T15-003'))
        self.assertNotIn('U-T04-01-04',targets('X-T04-001'))
        self.assertNotIn('U-T03-02-03',targets('X-T03-001'))
        self.assertEqual(route.blockers(nodes['X-T04-001'],{'U-T01-03-03'},capabilities=route.capabilities()),[])
        self.assertNotIn('U-T14-09-01',targets('X-T14-016'))
        self.assertIn('single-root insertion/split',nodes['X-T14-016']['active_increment']['task'])
        for key in ['X-T56-002','X-T56-003','X-T56-004']:
            self.assertFalse(any(target.startswith(('U-T48','U-T50','U-T51','U-T53','U-T56')) for target in targets(key)))

    def test_expansions_inherit_upstream_context_without_future_adapter_gates(self):
        units=cli.read(ROOT/'curriculum/v5/units.json')['units']
        first={}
        for unit in units:first.setdefault(unit['project'],unit)
        source=cli.read(ROOT/SOURCE)
        exceptions=source['exercise_prerequisite_policy']['portable_core_exceptions']
        for entry in self.active['nodes']:
            if entry['kind']!='exercise' or entry['id'] in exceptions:continue
            gates={edge['target'] for edge in entry['prerequisites'] if edge['kind'] in {'concept','component'}}
            self.assertTrue(set(first[entry['project']]['requires'])<=gates,entry['id'])

    def test_graph_successor_requires_exact_exported_components(self):
        wanted={'containers.depth.ring','containers.depth.heap','containers.depth.csr','algorithms.depth.graph'}
        n=route.node('U-T36-07-01')
        actual={e['artifact'] for e in n['prerequisites'] if e['kind']=='component'}
        self.assertTrue(wanted<=actual)
        catalog=learning.catalog()['artifacts']
        for key in wanted:
            self.assertIn(key,catalog)
            self.assertIn('required_header',catalog[key])
        all_assessed={x['id'] for x in self.active['nodes']}
        missing=route.blockers(n,all_assessed,capabilities=route.capabilities())
        for key in wanted:self.assertIn('export:'+key+'@1',missing)

    def test_route_generation_is_hash_seed_independent(self):
        for seed in ['0','1']:
            result=subprocess.run([sys.executable,str(ROOT/'tools/generate_native_route.py'),'--check'],
                                  cwd=ROOT,env=dict(os.environ,PYTHONHASHSEED=seed),capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)

    def test_active_component_export_link_and_header_rejection(self):
        from cmastery import exercises
        cases=[('foundation.cursor','X-T03-001','U-T03-02-01'),
               ('containers.depth.csr','U-T14-08-03','U-T15-05-01')]
        for artifact,producer,consumer in cases:
            with self.subTest(artifact=artifact), tempfile.TemporaryDirectory() as folder:
                base=Path(folder);workspace=base/'producer';workspace.mkdir()
                header=learning.catalog()['artifacts'][artifact]['required_header']
                (workspace/'include').mkdir();(workspace/'src').mkdir()
                (workspace/'include'/Path(header).name).write_bytes((ROOT/header).read_bytes())
                (workspace/'src/provider.c').write_text('/* Export plumbing fixture, no mastery claim. */\nint route_export_fixture(void){return 0;}\n')
                specification={'id':producer,'path':'ACTIVE.md','route':route.load()['id'],'sha256':route.specification_identity(producer)}
                (workspace/'ACTIVE.md').write_text(route.render_node(producer))
                cli.save(workspace/'.cmastery/project.json',{'active_specification':specification})
                record={'id':'test-acceptance','workspace':str(workspace),'source':cli.snapshot(workspace),
                        'revision':{'source':cli.snapshot(workspace)['files']},'assessment':{},'active_specification':specification}
                accepted={producer:{'valid':True,'record':record}}
                declaration={'contract_version':'1','cmake_target':'route_component','profiles':['C'],'targets':['host'],
                             'sources':['src/provider.c'],'headers':['include/'+Path(header).name],
                             'include_dirs':['include'],'dependencies':[]}
                cli.save(base/'export.json',declaration)
                def command(argv):
                    output=io.StringIO()
                    with redirect_stdout(output):status=cli.main([str(arg) for arg in argv])
                    self.assertEqual(status,0,output.getvalue())
                    return json.loads(output.getvalue())
                with patch.object(learning,'accepted',return_value=accepted),patch.object(exercises,'accepted',return_value=accepted),patch.object(learning,'append',return_value={'id':'test-event'}):
                    result=command(['artifact','export',base/'profile',workspace,artifact,base/'bundle','--manifest',base/'export.json'])
                    self.assertEqual(result['artifact'],artifact)
                    dest=base/'consumer';dest.mkdir()
                    cli.save(dest/'.cmastery/project.json',{'unit':consumer,'active_specification':{'id':consumer}})
                    linked=command(['artifact','link',base/'profile',dest,base/'bundle','--profile','C','--target','host'])
                    self.assertEqual(linked['artifact'],artifact)
                    self.assertIn(artifact,learning.linked_state(dest))
                    (dest/'main.c').write_text('int route_export_fixture(void); int main(void){return route_export_fixture();}\n')
                    (dest/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(route_handoff C)\ninclude(.cmastery/components.cmake)\nadd_executable(consume main.c)\ntarget_link_libraries(consume PRIVATE route_component)\n')
                    for argv in [['cmake','-S',str(dest),'-B',str(base/'build')],['cmake','--build',str(base/'build')],[str(base/'build/consume')]]:
                        result=subprocess.run(argv,capture_output=True,text=True)
                        self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                    forged=base/'forged';shutil.copytree(base/'bundle',forged)
                    forged_header=forged/declaration['headers'][0]
                    forged_header.write_text('/* Wrong API under a self-consistent manifest. */\n')
                    manifest=cli.read(forged/'artifact.json')
                    manifest['files'][declaration['headers'][0]]=hashlib.sha256(forged_header.read_bytes()).hexdigest()
                    manifest.pop('sha256');manifest['sha256']=learning.digest(manifest)
                    cli.save(forged/'artifact.json',manifest)
                    learning.verify_bundle(forged)
                    with self.assertRaisesRegex(ValueError,'header content'):
                        learning.artifact_link(base/'profile',dest,forged,'C','host')
                    with self.assertRaisesRegex(ValueError,'header content'):
                        learning.verify_component_api(manifest)
                    altered=workspace/'include'/Path(header).name;altered.write_text(altered.read_text()+'\n/* changed API */\n')
                    record['source']=cli.snapshot(workspace);record['revision']['source']=cli.snapshot(workspace)['files']
                    with self.assertRaisesRegex(ValueError,'unchanged public header'):
                        learning.artifact_export(base/'profile',workspace,artifact,base/'rejected',base/'export.json')

    def as_active(self, identifier):
        """Simulate the edition selecting a route without editing curriculum/edition.json."""
        real=route.load
        def load(root=None, route_id=None):
            return real(root, route_id or identifier)
        return patch.object(route,'load',side_effect=load)

    def test_old_profile_migration_preserves_ledger_and_acceptances(self):
        for active in ['contextual-v1','contextual-v2']:
            with self.subTest(active=active), self.as_active(active), tempfile.TemporaryDirectory() as folder:
                learning.study_init(folder)
                cfg=learning.settings(folder);self.assertEqual(cfg['route'],active)
                cfg.pop('route');cli.save(learning.area(folder)/'profile.json',cfg)
                before=(learning.area(folder)/'ledger.jsonl').read_bytes()
                self.assertIsNone(route.selected(folder))
                result=route.migrate(folder)
                self.assertEqual(result['new_acceptances'],0)
                self.assertTrue((learning.area(folder)/'ledger.jsonl').read_bytes().startswith(before))
                self.assertEqual(learning.accepted(folder),{})
                self.assertEqual(route.selected(folder)['id'],active)

    def test_v1_pinned_profile_keeps_working_until_adoption(self):
        with self.as_active('contextual-v2'), tempfile.TemporaryDirectory() as folder:
            learning.study_init(folder)
            cfg=learning.settings(folder);cfg['route']='contextual-v1';cli.save(learning.area(folder)/'profile.json',cfg)
            self.assertEqual(route.selected(folder)['id'],'contextual-v1')
            packet=learning.study_next(folder)
            self.assertEqual(packet['route']['id'],'contextual-v1')
            self.assertTrue(packet['route']['available'])
            # A v1 card keeps its recorded identity; v1 nodes carry no episode metadata.
            v1=route.render_node('U-T01-03-01',route_id='contextual-v1')
            self.assertIn('Active route: contextual-v1.',v1)
            self.assertNotIn('## Episode',v1)
            self.assertEqual(route.specification_identity('U-T01-03-01','contextual-v1'),hashlib.sha256(v1.encode()).hexdigest())
            self.assertIn('## Episode: ',route.render_node('U-T01-03-01',route_id='contextual-v2'))
            # A pinned profile's study sheet renders its own route's card.
            sheet=Path(folder)/'sheet.md';learning.study_sheet(folder,sheet,'U-T01-03-01')
            self.assertIn('Active route: contextual-v1.',sheet.read_text())
            before=(learning.area(folder)/'ledger.jsonl').read_bytes()
            result=route.migrate(folder)
            self.assertEqual((result['previous_route'],result['route'],result['new_acceptances']),('contextual-v1','contextual-v2',0))
            self.assertEqual(result['migration'],self.v2['migration'])
            ledger=(learning.area(folder)/'ledger.jsonl').read_bytes()
            self.assertTrue(ledger.startswith(before))
            event=json.loads(ledger.decode().strip().splitlines()[-1])
            self.assertEqual((event['kind'],event['previous_route'],event['route']),('route-migrated','contextual-v1','contextual-v2'))
            self.assertEqual(learning.accepted(folder),{})
            self.assertFalse(route.migrate(folder)['changed'])
            self.assertEqual((learning.area(folder)/'ledger.jsonl').read_bytes(),ledger)
            cfg=learning.settings(folder);cfg['route']='contextual-v0';cli.save(learning.area(folder)/'profile.json',cfg)
            with self.assertRaisesRegex(ValueError,'Pinned study route unavailable'):
                route.selected(folder)

    def test_recorded_specification_without_route_means_contextual_v1(self):
        self.assertEqual(route.recorded_route({'id':'U-T01-03-01','sha256':'0'*64}),'contextual-v1')
        self.assertEqual(route.recorded_route({'id':'U-T01-03-01','route':'contextual-v2'}),'contextual-v2')
        with tempfile.TemporaryDirectory() as folder:
            workspace=Path(folder)
            text=route.render_node('U-T01-03-01',route_id='contextual-v1')
            (workspace/'ACTIVE.md').write_text(text)
            cli.save(workspace/'.cmastery/project.json',{'active_specification':{'id':'U-T01-03-01','path':'ACTIVE.md',
                     'sha256':hashlib.sha256(text.encode()).hexdigest()}})
            with self.as_active('contextual-v2'):
                route.verify_workspace_specification(workspace)

    def test_study_readiness_and_sheet_use_active_view(self):
        for active in ['contextual-v1','contextual-v2']:
            with self.subTest(active=active), self.as_active(active), tempfile.TemporaryDirectory() as folder:
                learning.study_init(folder)
                packet=learning.study_next(folder)
                self.assertEqual(packet['route']['id'],active)
                self.assertTrue(packet['route']['available'])
                sheet=Path(folder)/'sheet.md';learning.study_sheet(folder,sheet,'U-T01-03-01')
                self.assertIn('Active route: '+active,sheet.read_text())

    def test_guide_describes_the_selected_route(self):
        with self.as_active('contextual-v2'):
            guide=route.render_guide()
            self.assertIn('Authored episodes order this route',guide)
            self.assertIn('| Step | Increment | Episode or block | System context |',guide)
            self.assertIn('unauthored:',guide)
        with self.as_active('contextual-v1'):
            self.assertIn('the route interleaves systems, mechanisms and changed assumptions',route.render_guide())

    def test_effective_graph_card_and_canonical_revision_proof(self):
        text=route.render_node('U-T15-05-01')
        self.assertIn('negative',text)
        self.assertNotIn('duplicate-heavy input exposing a partition',text)
        self.assertNotIn('## Working card',route.project_brief('PR-T15-05'))
        packet=json.loads((ROOT/BASE/'teaching_revisions.json').read_text())
        for row in packet['records']:
            self.assertTrue(qualifies(row['path'],row['predecessor_sha256'],root=ROOT),row['path'])
            self.assertFalse(qualifies(row['path'],'0'*64,root=ROOT))
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);path=root/BASE/'teaching_revisions.json';path.parent.mkdir(parents=True);path.write_text(json.dumps(packet))
            row=packet['records'][0];path=root/row['path'];path.parent.mkdir(parents=True);path.write_bytes((ROOT/row['path']).read_bytes()+b'\nUnreviewed edit.\n')
            self.assertFalse(qualifies(row['path'],row['predecessor_sha256'],root=root))

    def test_cli_init_exports_effective_card_without_assessor_code(self):
        with tempfile.TemporaryDirectory() as folder:
            destination=Path(folder)/'learner'
            cli.instantiate('PR-T15-05',destination,unit_id='U-T15-05-01')
            self.assertIn('ACTIVE.md',(destination/'PROJECT.md').read_text())
            self.assertEqual((destination/'ACTIVE.md').read_text(),route.render_node('U-T15-05-01'))
            self.assertIn('ACTIVE.md',(destination/'UNIT.md').read_text())
            self.assertIn('ACTIVE.md',(destination/'ASSESSMENT.md').read_text())
            self.assertFalse(any('assessor' in p.relative_to(destination).parts for p in destination.rglob('*')))
            self.assertEqual(cli.read(destination/'.cmastery/project.json')['assessment'],'learner')

    def test_successor_explicit_package_and_criterion_aliases(self):
        from native_successors import resolve
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            teaching=root/'labs/demo/CONTRACTS.md';teaching.parent.mkdir(parents=True);teaching.write_text('Public instruction')
            support=root/'research/support.json';support.parent.mkdir(parents=True)
            support.write_text(json.dumps({'updates':[{'id':'record', 'criterion':'AX-ONE', 'dimensions':{'teaching':{'evidence':['labs/demo/CONTRACTS.md','assessor/private.md']}}}]}))
            data={'project':'PR-ONE','criteria':[{'id':'AX-ONE','objective':'O1'}], 'successor_support':{'path':'research/support.json','criterion_ids':['AX-ONE']}}
            self.assertEqual(resolve(data,root)[0]['public_contracts'],['labs/demo/CONTRACTS.md'])
            support.write_text(json.dumps({'id':'provider','original_criterion_ids':['AX-ONE'], 'public_artifacts':['labs/demo/CONTRACTS.md'],'assessor_artifacts':['assessor/private.md']}))
            self.assertEqual(resolve(data,root)[0]['public_contracts'],['labs/demo/CONTRACTS.md'])
            support.write_text(json.dumps({'updates':[{'id':'X-UNRELATED-V1','dimensions':{'teaching':{'evidence':['labs/demo/CONTRACTS.md']}}}]}))
            # The explicit parent binding authorizes package teaching, but keeps
            # its scope additive rather than replacing a criterion's task.
            data['successor_support']['scope']='Additional variant package; original criteria unchanged.'
            result=resolve(data,root)[0]
            self.assertEqual(result['public_contracts'],['labs/demo/CONTRACTS.md'])
            self.assertIn('Additional variant',result['binding_scopes'][0])
            data['successor_support']['criterion_ids']=['AX-OTHER']
            self.assertEqual(resolve(data,root)[0]['public_contracts'],[])

    def test_freestanding_addition_retains_original_implementation_tasks(self):
        text=route.render_node('U-T11-01-01')
        data=cli.read(ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T11-01.json')
        self.assertIn(data['criteria'][0]['task'],text)
        self.assertNotIn('not an additional implementation assignment',text)
        for resolution in route.node('U-T11-01-01')['criterion_resolutions']:
            self.assertEqual(resolution['mode'],'additive')

    def test_all_explicit_successors_resolve_without_private_contracts(self):
        records=json.loads((ROOT/BASE/'criterion_resolutions.json').read_text())['criteria']
        self.assertTrue(records)
        for record in records:
            self.assertEqual(record['unresolved_bindings'],[],record['criterion'])
            self.assertTrue(all(p.startswith(('labs/','curriculum/')) and 'assessor' not in Path(p).parts for p in record['public_contracts']))
        graph=route.render_node('U-T36-07-01')
        self.assertIn('labs/algorithm_frontier/CONTRACTS.md',graph)
        self.assertIn('No new assembly graph solver',graph)

    def test_effective_contract_tamper_and_source_change_reject(self):
        with tempfile.TemporaryDirectory() as folder:
            destination=Path(folder)/'learner'
            cli.instantiate('PR-T15-05',destination,unit_id='U-T15-05-01')
            route.verify_workspace_specification(destination)
            active=destination/'ACTIVE.md';original=active.read_text();active.write_text(original+'Changed acceptance.\n')
            with self.assertRaisesRegex(ValueError,'changed or is missing'):
                route.verify_workspace_specification(destination)
            active.write_text(original)
            with patch.object(route,'specification_identity',return_value='0'*64):
                with self.assertRaisesRegex(ValueError,'specification changed'):
                    route.verify_workspace_specification(destination)

    def test_operative_successor_replaces_predecessor_and_teaches_context(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            def put(name, text):
                path=root/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(text)
            def js(name, value):put(name,json.dumps(value))
            js('curriculum/edition.json',{'active_route':'route.json'})
            js('route.json',{'schema_version':1,'id':'test-route','contexts':[{'id':'graph','problem':'Trace dependencies','teaching':'curriculum/teaching.md'}], 'nodes':[{'id':'UNIT','kind':'unit','project':'PR-ONE','title':'Graph','context':'graph','why_now':'A dependency cycle must reject.','next_consumer':'compiler','prerequisites':[], 'objectives':['O1'],'sources':['curriculum/projects/PR-ONE.md'],'contracts':['labs/current/CONTRACTS.md'],'criterion_resolutions':[{'criterion':'AX-ONE','mode':'replace','public_contracts':['labs/current/CONTRACTS.md'],'binding_scopes':[]}]}]})
            js('curriculum/v5/units.json',{'units':[{'id':'UNIT','title':'Graph','steps':['OBSOLETE assembly sort requirement'],'checks':['OBSOLETE diagnostic'],'stop_condition':'OBSOLETE gate'}]})
            put('curriculum/projects/PR-ONE.md','# Old project\n\nOBSOLETE full project requirement')
            put('curriculum/teaching.md','# Context\n\nA graph worklist reaches a fixed point.')
            put('labs/current/CONTRACTS.md','# Current contract\n\nNEW frontier certificate requirement.')
            js('curriculum/v5/expansion/legacy_assessments/PR-ONE.json',{'criteria':[{'id':'AX-ONE','objective':'O1','title':'Frontier','task':'OBSOLETE criterion requirement','rubric':{'2':'Independent correctness'},'variants':[{'id':'AX-ONE-V1','minimum_days':7,'task':'OBSOLETE variant requirement'}]}]})
            with patch('cmastery.assessments.learner_brief',return_value='OBSOLETE criterion requirement'):
                text=route.render_node('UNIT',root)
            self.assertNotIn('OBSOLETE',text)
            self.assertEqual(text.count('NEW frontier certificate requirement.'),1)
            self.assertIn('A graph worklist reaches a fixed point.',text)
            self.assertIn('AX-ONE-V1',text)
            self.assertIn('Independent correctness',text)

    def test_live_acceptance_reopens_when_effective_contract_changes(self):
        with tempfile.TemporaryDirectory() as folder:
            entry={'unit':'U-T01-03-01','workspace':folder,'unit_contract':learning.digest(learning.unit('U-T01-03-01')),
                   'active_specification':{'id':'U-T01-03-01','sha256':'0'*64}}
            valid, reason=learning.acceptance_valid(entry)
            self.assertFalse(valid)
            self.assertEqual(reason,'effective learner contract changed')

    def test_project_only_initialization_selects_effective_successor(self):
        with tempfile.TemporaryDirectory() as folder:
            destination=Path(folder)/'project'
            cli.instantiate('PR-T36-07',destination)
            self.assertTrue((destination/'ACTIVE.md').is_file())
            text=(destination/'ACTIVE.md').read_text()
            self.assertNotIn('Implement lower-bound, stable insertion and heap repair kernels under both target ABIs',text)
            self.assertIn('No new assembly graph solver',text)
            self.assertEqual(cli.read(destination/'.cmastery/project.json')['active_specification']['id'],'PR-T36-07')

    def test_publisher_uses_same_effective_card_and_preserves_historical_route(self):
        from publish_native_route import prepare
        with tempfile.TemporaryDirectory() as folder:
            source=Path(folder);page=source/'learning/route.md';page.parent.mkdir(parents=True)
            old='(v5-route)=\n# Historical route\n';page.write_text(old)
            subset=dict(self.active,order=['U-T36-07-01'])
            expected=route.render_node('U-T36-07-01')
            with patch.object(route,'load',return_value=subset):
                docs,sections=prepare(source,[old])
            self.assertIn('learning/historical-route',docs)
            self.assertIn('(v5-historical-route)=',(source/'learning/historical-route.md').read_text())
            self.assertEqual((source/'learning/effective/U-T36-07-01.md').read_text(),expected)
            self.assertIn('effective/U-T36-07-01.md',page.read_text())
            self.assertNotIn(old,sections)

    def test_public_path_escape_rejects(self):
        for name in ['../private.md','/tmp/private.md','assessor/algorithms/algo.c']:
            with self.assertRaises(ValueError):route.public_path(ROOT,name)

    def test_inline_document_links_keep_their_original_public_base(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            for name in ['labs/component/CONTRACTS.md','labs/helper/api.h','assessor/private.md','tools/author_answer.py','research/private.json','reviews/key.md','planning/solution.md']:
                p=root/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_text('Public fixture' if 'assessor' not in name else 'WITHHELD_TEST_SENTINEL')
            text='[API](../helper/api.h) [online](https://example.com/a) [anchor](#argument) [private](../../assessor/private.md)'
            text+=' [tool](../../tools/author_answer.py) [metadata](../../research/private.json) [review](../../reviews/key.md) [plan](../../planning/solution.md)'
            actual=route.inline_markdown(text,'labs/component/CONTRACTS.md',root)
            self.assertIn('[API](labs/helper/api.h)',actual)
            self.assertIn('[online](https://example.com/a)',actual)
            self.assertIn('[anchor](#argument)',actual)
            self.assertNotIn('](../../assessor/',actual)
            self.assertNotIn('WITHHELD_TEST_SENTINEL',actual)
            destination=Path(folder)/'workspace'
            with patch.object(route,'node',return_value={'supplied_artifacts':[]}),patch.object(route,'render_node',return_value=actual):
                exported=route.export_assets('synthetic',destination,root)
            self.assertIn('labs/helper/api.h',exported)
            self.assertTrue((destination/'labs/helper/api.h').is_file())
            self.assertFalse((destination/'assessor').exists())
            for private in ['tools','research','reviews','planning']:
                self.assertFalse((destination/private).exists())
            for private in ['tools/author_answer.py','research/private.json','reviews/key.md','planning/solution.md']:
                with self.assertRaises(ValueError):route.public_path(root,private)
            wire=root/'research/enhancement/networking_contracts.json';wire.parent.mkdir(parents=True);wire.write_text('{"public":"wire format"}')
            names=set();text=route.inline_markdown('[wire](../../research/enhancement/networking_contracts.json)','curriculum/projects/example.md',root,names)
            self.assertIn('research/enhancement/networking_contracts.json',names)
            self.assertIn('](research/enhancement/networking_contracts.json)',text)

    def test_asset_parent_symlink_cannot_expose_assessor_or_redirect_writes(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)/'root';root.mkdir();(root/'assessor').mkdir();(root/'labs').mkdir()
            (root/'assessor/answer.h').write_text('WITHHELD_TEST_SENTINEL')
            (root/'labs/redirected').symlink_to(root/'assessor',target_is_directory=True)
            with self.assertRaises(ValueError):route.public_path(root,'labs/redirected/answer.h')
            with patch.object(route,'node',return_value={'supplied_artifacts':['labs/redirected/answer.h']}),patch.object(route,'render_node',return_value=''):
                with self.assertRaises(ValueError):route.export_assets('synthetic',Path(folder)/'rejected',root)
            dest=Path(folder)/'destination';dest.mkdir();(dest/'labs').symlink_to(root/'assessor',target_is_directory=True)
            with self.assertRaises(ValueError):route.public_path(dest,'labs/answer.h')


if __name__=='__main__':unittest.main()
