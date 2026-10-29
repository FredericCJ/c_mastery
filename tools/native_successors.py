"""Resolve explicit criterion successor links to public operative teaching.

Read author support metadata, but project only public contract paths. Evidence
statuses, assessor analyses and historical reports are never learner instructions.
"""
from pathlib import Path, PurePosixPath
import json
import re

PUBLIC = re.compile(r'(?:labs|curriculum)/[A-Za-z0-9_./-]+\.md')


def strings(value):
    if isinstance(value,str):yield value
    elif isinstance(value,dict):
        for child in value.values():yield from strings(child)
    elif isinstance(value,list):
        for child in value:yield from strings(child)


def records(value):
    if isinstance(value,dict):
        if isinstance(value.get('id'),str):yield value
        for child in value.values():yield from records(child)
    elif isinstance(value,list):
        for child in value:yield from records(child)


def public_contracts(value, root):
    result=[]
    # Read only explicitly instructional fields and teaching evidence. Do not
    # use arbitrary input manifests to imply that every file is operative.
    for key in ('teaching','public_contract','learner_contract','contract','shared_contracts','public_artifacts'):
        if key in value:
            for text in strings(value[key]):
                for name in PUBLIC.findall(text):
                    if '..' not in PurePosixPath(name).parts and 'assessor' not in PurePosixPath(name).parts and (root/name).is_file():
                        result.append(name)
    dimensions=value.get('dimensions',{})
    if isinstance(dimensions,dict) and isinstance(dimensions.get('teaching'),dict):
        result+=public_contracts({'teaching':dimensions['teaching']},root)
    return sorted(set(result))


def resolve(data, root, dispositions=None):
    root=Path(root);resolved=[];loaded={}
    inherited=[data.get('successor_support'),*data.get('additional_successor_support',[])]
    for criterion in data['criteria']:
        bindings=inherited+[criterion.get('successor_support')]
        contracts=[];sources=[];unresolved=[];scopes=[];replacement=False
        for binding in bindings:
            if not isinstance(binding,dict) or not binding.get('path'):continue
            ids=binding.get('criterion_ids',[binding.get('criterion_id',binding.get('id'))])
            if any(ids) and criterion['id'] not in ids:continue
            name=binding['path'];rel=PurePosixPath(name)
            if not name.startswith('research/') or '..' in rel.parts or not name.endswith('.json'):raise ValueError('Unsafe successor binding')
            if name not in loaded:loaded[name]=json.loads((root/name).read_text(encoding='utf-8-sig'))
            matches=[row for row in records(loaded[name]) if row['id']==criterion['id'] or row.get('criterion')==criterion['id'] or criterion['id'] in row.get('criterion_ids',[]) or criterion['id'] in row.get('original_criterion_ids',[])]
            exact_match=bool(matches)
            if not matches and criterion['id'] in ids:
                # The canonical criterion explicitly binds a whole supplementary
                # package. Project its public teaching and retain the exact
                # additive scope; do not promote author closure claims to tasks.
                matches=list(records(loaded[name]))
            if binding.get('scope'):scopes.append(binding['scope'])
            selected=sorted(set(p for row in matches for p in public_contracts(row,root)))
            if selected:
                contracts+=selected;sources.append(name)
                scope=binding.get('scope','').lower()
                additive=any(term in scope for term in ('additive original x', 'original parent criteria unchanged', 'additional variant'))
                disposition=(dispositions or {}).get(name)
                if disposition:
                    if disposition['mode'] not in {'additive','replace'}:
                        raise ValueError('Unknown authored successor disposition')
                    additive=disposition['mode']=='additive'
                    scopes.append(disposition['reason'])
                replacement = replacement or (exact_match and not additive)
            else:unresolved.append(name)
        resolved.append(dict(criterion=criterion['id'],objective=criterion['objective'],project=data['project'],
                             public_contracts=sorted(set(contracts)),source_records=sorted(set(sources)),
                             disposition='explicit-public-successor' if contracts else 'canonical-criterion',
                             mode='replace' if replacement else ('additive' if contracts else 'retain'),
                             unresolved_bindings=sorted(set(unresolved)),binding_scopes=list(dict.fromkeys(scopes)),
                             policy='Exact criterion successor teaching is the operative task; explicitly additive packages augment the canonical task. Objective rubric and delayed identities remain mandatory. Historical author evidence is not learner work.'))
    return resolved
