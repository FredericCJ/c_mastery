"""Glossary source of truth: loading, validation, term linking and glossary rendering.

Entries live in curriculum/vocabulary/<owner>.json where <owner> is a domain ID
or `_core` for cross-domain terms. Each entry carries an original definition,
normative authorities (registry resource IDs with locators), edition notes,
near-synonyms to distinguish, forms to avoid, idiomatic usage, related symbols
and recall prompts. Terms are globally unique; homonyms carry a disambiguator
such as "exception (Arm)". Published pages link the first occurrence of each
term through the MyST {term} role; the standalone edition links to anchors.
"""
from __future__ import annotations
import re
from pathlib import Path
from common import ROOT, anchor, read_json

VOCAB = ROOT / 'curriculum/vocabulary'
KINDS = {'iso-c', 'posix', 'linux', 'arm', 'elf-abi', 'toolchain', 'hardware', 'course', 'concurrency', 'numerical', 'security', 'programming-model'}
REQUIRED = ('id', 'term', 'owner', 'definition', 'authority', 'kind')
SKIP_LINE = re.compile(r'^\s*(#|<|\(|```|~~~|\||:::)')
INLINE_SKIP = re.compile(r'`[^`\n]*`|\{[a-z]+\}`[^`\n]*`|\[[^\]\n]*\]\([^)\n]*\)|<[^>\n]+>|\*\*[^*\n]*:\*\*', re.M)


def files():
    return sorted(p for p in VOCAB.glob('*.json') if not p.name.startswith('_') or p.name in {'_core.json'})


def load():
    """Return (entries, documents) where documents maps owner -> parsed file."""
    entries, documents = [], {}
    for path in files():
        value = read_json(path)
        owner = value.get('domain') or ('core' if path.name == '_core.json' else path.stem)
        documents[owner] = value
        for entry in value.get('entries', []):
            entry = dict(entry)
            entry.setdefault('owner', owner)
            entry['_file'] = path.relative_to(ROOT).as_posix() if path.is_relative_to(ROOT) else path.name
            entries.append(entry)
    return entries, documents


def validate(entries, resources, taxonomy):
    errors, pending = [], []
    records = {r['id']: r for r in resources}
    domains = {d['id'] for d in taxonomy}
    objectives = {o['id'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
    seen_terms, seen_ids = {}, set()
    for entry in entries:
        label = entry.get('id') or entry.get('term') or '?'
        for key in REQUIRED:
            if not entry.get(key):
                errors.append(f'{label}: missing {key}')
        if entry.get('id') in seen_ids:
            errors.append(f'{label}: duplicate entry id')
        seen_ids.add(entry.get('id'))
        term = str(entry.get('term', '')).strip()
        key = term.casefold()
        if key in seen_terms:
            errors.append(f'{label}: duplicate term "{term}" (also {seen_terms[key]})')
        seen_terms[key] = label
        for alias in entry.get('aliases', []):
            akey = str(alias).casefold()
            if akey in seen_terms and seen_terms[akey] != label:
                errors.append(f'{label}: alias "{alias}" collides with {seen_terms[akey]}')
            seen_terms[akey] = label
        if entry.get('kind') not in KINDS:
            errors.append(f'{label}: unknown kind {entry.get("kind")}')
        notes = entry.get('edition_notes', [])
        if not isinstance(notes, list) or any(not isinstance(n, dict) or not n.get('edition') or not n.get('note') for n in notes):
            errors.append(f'{label}: edition_notes needs a list of edition/note records')
        idiom = entry.get('idiom', [])
        if not isinstance(idiom, list) or any(not isinstance(s, str) or not s.strip() for s in idiom):
            errors.append(f'{label}: idiom needs a list of nonempty usage strings')
        if 'idiomatic_usage' in entry:
            errors.append(f'{label}: use the rendered idiom list, not idiomatic_usage')
        if entry.get('owner') != 'core' and entry.get('owner') not in domains:
            errors.append(f'{label}: unknown owner {entry.get("owner")}')
        for used in entry.get('also_used_in', []):
            if used not in domains:
                errors.append(f'{label}: unknown also_used_in {used}')
        for oid in entry.get('objectives', []):
            if oid not in objectives:
                errors.append(f'{label}: unknown objective {oid}')
        authorities = entry.get('authority') or []
        if not isinstance(authorities, list) or not authorities:
            errors.append(f'{label}: at least one authority is required')
            authorities = []
        for authority in authorities:
            record = records.get(authority.get('resource_id'))
            if record is None:
                errors.append(f'{label}: authority resource {authority.get("resource_id")} is not registered')
                continue
            if record.get('assignable') is False:
                errors.append(f'{label}: authority {authority["resource_id"]} is a bibliographic lead, not an inspected source')
            locator = authority.get('locator', '')
            if not locator:
                errors.append(f'{label}: authority without locator')
            elif locator in record.get('locators', []):
                authority['inspection'] = 'registry'
            elif authority.get('inspection') == 'pending':
                pending.append((label, authority['resource_id'], locator))
            else:
                errors.append(f'{label}: locator "{locator}" is not an inspected locator of {authority["resource_id"]} and is not declared pending')
        if entry.get('kind') == 'iso-c':
            if not entry.get('edition_notes'):
                errors.append(f'{label}: ISO C terms need edition_notes')
            if not any(str(a.get('resource_id', '')).lower().startswith(('r-wg14', 'r-sys-c-')) for a in authorities):
                errors.append(f'{label}: ISO C terms need a WG14 authority')
        distinctions = entry.get('distinguish_from', [])
        if not isinstance(distinctions, list):
            errors.append(f'{label}: distinguish_from needs a list of term/distinction records')
            distinctions = []
        for item in distinctions:
            if not isinstance(item, dict) or any(not isinstance(item.get(k), str) or not item[k].strip()
                                                 for k in ('term', 'distinction')):
                errors.append(f'{label}: distinguish_from needs term and distinction')
        avoided = entry.get('avoid', [])
        if not isinstance(avoided, list):
            errors.append(f'{label}: avoid needs a list of form/replace_with records')
            avoided = []
        for item in avoided:
            if not isinstance(item, dict) or any(not isinstance(item.get(k), str) or not item[k].strip()
                                                 for k in ('form', 'replace_with')):
                errors.append(f'{label}: avoid needs form and replace_with')
                continue
            try:
                re.compile(item.get('form', ''))
            except re.error:
                errors.append(f'{label}: avoid form is not a valid pattern')
        recall = entry.get('recall')
        if recall is not None and set(recall) != {'define', 'distinguish', 'counterexample'}:
            errors.append(f'{label}: recall needs define, distinguish and counterexample')
    return errors, pending


def term_index(entries):
    """Patterns ordered longest-first so multi-word terms win over their parts."""
    items = []
    for entry in entries:
        names = [entry['term']] + list(entry.get('aliases', []))
        for name in names:
            if entry.get('link') is False:
                continue
            display = name
            pattern = re.compile(r'(?<![\w-])' + re.escape(name) + r'(?![\w-])', re.IGNORECASE)
            items.append((len(name), pattern, display, entry))
    items.sort(key=lambda x: -x[0])
    return items


def link_terms(text, index, limit_per_term=1):
    """Link the first occurrence of each term in prose lines; code, links, roles and HTML stay untouched."""
    linked = {}
    out = []
    for line in text.split('\n'):
        if SKIP_LINE.match(line) or not line.strip():
            out.append(line)
            continue
        segments = []
        position = 0
        for match in INLINE_SKIP.finditer(line):
            segments.append((line[position:match.start()], True))
            segments.append((match.group(0), False))
            position = match.end()
        segments.append((line[position:], True))
        rebuilt = []
        for segment, editable in segments:
            if not editable:
                rebuilt.append(segment)
                continue
            consumed = []
            replacements = []
            for _, pattern, display, entry in index:
                key = entry['term']
                if linked.get(key, 0) >= limit_per_term:
                    continue
                for match in pattern.finditer(segment):
                    span = (match.start(), match.end())
                    if any(not (span[1] <= a or span[0] >= b) for a, b in consumed):
                        continue
                    consumed.append(span)
                    replacements.append((span, '{term}`' + match.group(0) + ' <' + key + '>`'))
                    linked[key] = linked.get(key, 0) + 1
                    break
            for (start, end), replacement in sorted(replacements, reverse=True):
                segment = segment[:start] + replacement + segment[end:]
            rebuilt.append(segment)
        out.append(''.join(rebuilt))
    return '\n'.join(out)


def term_anchor(term):
    return 'term-' + anchor(term)


def plain_terms(text):
    """Standalone edition: {term}`text <term>` becomes a link to the glossary anchor."""
    return re.sub(r'\{term\}`([^`<>]+) <([^`<>]+)>`', lambda m: '[' + m[1] + '](#' + term_anchor(m[2]) + ')', text)


def _entry_body(entry, resources):
    records = {r['id']: r for r in resources}
    lines = [entry['definition'].strip()]
    notes = []
    for authority in entry.get('authority', []):
        record = records.get(authority['resource_id'], {})
        title = record.get('title', authority['resource_id'])
        status = '' if authority.get('inspection') == 'registry' else ' (inspection pending)'
        notes.append(f'**Authority:** {title}, {authority["locator"]}{status}.')
    for note in entry.get('edition_notes', []):
        notes.append(f'**{note["edition"]}:** {note["note"]}')
    for item in entry.get('distinguish_from', []):
        notes.append(f'**Distinguish from {item["term"]}:** {item["distinction"]}')
    for item in entry.get('avoid', []):
        when = (' when ' + item['when']) if item.get('when') else ''
        notes.append(f'**Avoid** "{item["form"]}"{when}; write {item["replace_with"]}.')
    if entry.get('idiom'):
        notes.append('**Idiomatic usage:** ' + ' / '.join('"' + x + '"' for x in entry['idiom']))
    if entry.get('symbols'):
        notes.append('**Symbols:** ' + ', '.join('`' + s + '`' for s in entry['symbols']))
    if entry.get('misconception'):
        notes.append('**Misconception:** ' + entry['misconception'])
    if entry.get('owner') != 'core':
        notes.append('**Taught in:** {ref}`' + entry['owner'] + '`' +
                     (' (also ' + ', '.join('{ref}`' + d + '`' for d in entry['also_used_in']) + ')' if entry.get('also_used_in') else '') + '.')
    return lines + notes


def glossary_page(owner, entries, resources, title):
    """A MyST glossary directive for one owner (domain or core)."""
    selected = sorted((e for e in entries if e['owner'] == owner), key=lambda e: e['term'].casefold())
    page = [f'(glossary-{owner.lower()})=', f'# {title}', '',
            'Precise terms with their normative authorities. A term links here from its first use on each page; '
            'near-synonyms are distinguished explicitly and the listed forms are avoided in teaching prose.', '']
    if not selected:
        page.append('No terms are recorded for this domain yet.')
        return '\n'.join(page) + '\n'
    page.append('```{glossary}')
    for entry in selected:
        page.append(entry['term'])
        for paragraph in _entry_body(entry, resources):
            page.append('  ' + paragraph.replace('\n', ' '))
            page.append('')
    page.append('```')
    return '\n'.join(page) + '\n'


def plain_glossary(owner, entries, resources, title):
    """Standalone-edition rendering with explicit anchors instead of the directive."""
    selected = sorted((e for e in entries if e['owner'] == owner), key=lambda e: e['term'].casefold())
    page = [f'<a id="glossary-{owner.lower()}"></a>', '', f'# {title}', '']
    for entry in selected:
        page += [f'<a id="{term_anchor(entry["term"])}"></a>', '', f'**{entry["term"]}**', '']
        page += [paragraph for paragraph in _entry_body(entry, resources)] + ['']
    return '\n'.join(page) + '\n'


def index_page(entries, documents):
    rows = ['| Term | Kind | Owner | Definition |', '|---|---|---|---|']
    for entry in sorted(entries, key=lambda e: e['term'].casefold()):
        owner = entry['owner']
        target = 'glossary-' + owner.lower()
        rows.append('| {term}`' + entry['term'] + '` | ' + entry['kind'] + ' | {ref}`' + target + '` | ' +
                    entry['definition'].split('. ')[0].replace('|', '/')[:140] + ' |')
    owners = ['core'] + sorted(o for o in documents if o != 'core')
    page = ['(glossary-index)=', '# Glossary', '',
            f'{len(entries)} terms across {len(owners)} glossaries. Each term has one owner; cross-domain terms live in the core glossary.',
            '', '\n'.join(rows), '', '```{toctree}', ':maxdepth: 1', ''] + owners + ['```']
    return '\n'.join(page) + '\n'


def owner_title(owner, modules):
    if owner == 'core':
        return 'Core vocabulary: ISO C, POSIX, Arm, ELF and toolchain terms'
    meta = modules.get(owner, {}).get('meta', {})
    return f'{owner} vocabulary — {meta.get("title", owner)}'
