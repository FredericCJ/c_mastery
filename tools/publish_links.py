"""Route canonical links without publishing assessor or author-only sources."""
from pathlib import Path, PurePosixPath
import posixpath
import re
import shutil
from urllib.parse import unquote, urlsplit

from common import ROOT, anchor

REPOSITORY_ROOTS = {'curriculum', 'labs', 'fixtures', 'research', 'tools',
                    'assessor', 'references', 'reviews', 'planning'}
DOWNLOAD_ROOTS = {'curriculum', 'labs', 'fixtures', 'research', 'tools'}
# A link begins before code in its label; an outer code span begins before a
# link-shaped example inside it. Match in source order instead of splitting
# inline code away first, which would tear [`file`](path) into three pieces.
INLINE = re.compile(
    r'(?P<link>(?<!!)(?<!\\)\[(?P<label>(?:`+[^`\r\n]*`+|\\.|[^\[\]\r\n])*)\]'
    r'\((?P<address>[^)\r\n]+)\))|'
    r'(?P<code>(?P<ticks>`+)(?!`).*?(?<!`)(?P=ticks)(?!`))', re.S)
DOWNLOADS = '_static/learner_sources'
EXCLUDED_PARTS = frozenset({'private', '.venv', '__pycache__'})


def excluded_source(relative):
    """Private captures and runtime caches belong to no publication bundle."""
    return bool(EXCLUDED_PARTS.intersection(relative.parts))


def local_source(address, current=None, root=ROOT):
    """Resolve an existing canonical file, never an escaped or missing target."""
    parsed = urlsplit(address)
    if parsed.scheme or parsed.netloc or not parsed.path:
        return None
    path = unquote(parsed.path)
    if '\\' in path or path.startswith('/'):
        raise ValueError('Unsafe local publication path: ' + address)
    if PurePosixPath(path).parts[0] not in REPOSITORY_ROOTS:
        if current is None:
            return None  # Already-generated sibling links are left for Sphinx.
        path = str(PurePosixPath(current).parent / path)
    name = posixpath.normpath(path)
    if name == '..' or name.startswith('../'):
        raise ValueError('Publication source escapes repository: ' + address)
    source = root / name
    try:
        resolved = source.resolve().relative_to(root.resolve())
    except ValueError as exc:
        raise ValueError('Publication source escapes repository: ' + address) from exc
    # An excluded citation can survive in an authorized source register while
    # its private capture is absent from an extracted source archive. Do not
    # require, read or copy that capture merely to render its exclusion notice.
    if not (excluded_source(PurePosixPath(name)) or excluded_source(resolved)) and not source.is_file():
        raise ValueError('Missing publication source file: ' + name)
    return name, source, resolved, parsed.fragment


def learner_source(relative):
    # Reuse the release policy: research authoring recipes and audits belong to
    # the full source bundle, even when an audit has no executable answers.
    from package_v5 import learner_file
    return (not excluded_source(relative) and relative.parts[0] in DOWNLOAD_ROOTS and 'assessor' not in relative.parts
            and learner_file(Path(relative)))


def link(label, address, *, source, page, current=None, contracts=(), root=ROOT):
    found = local_source(address, current, root)
    if found is None:
        return '[' + label + '](' + address + ')'
    name, original, resolved, fragment = found
    if excluded_source(PurePosixPath(name)) or excluded_source(resolved):
        return label + ' (private research evidence; excluded from publication and all bundles)'
    if not learner_source(PurePosixPath(name)) or not learner_source(resolved):
        bundle = 'assessor' if 'assessor' in resolved.parts else 'full source'
        return (label + ' (restricted source evidence: `' + name + '`; ' +
                'available only in the ' + bundle + ' bundle)')
    document = None
    identifier = None
    if name in contracts:
        from publish_contracts_v5 import LEGACY, target
        identifier, contract_page = target(name)
        document = ('learning/legacy-assessments/shared-contract' if name == LEGACY
                    else 'learning/exercises/' + contract_page)
    else:
        parts = PurePosixPath(name).parts
        if len(parts) == 3 and parts[:2] in {('curriculum', 'modules'),
                                           ('curriculum', 'projects'),
                                           ('curriculum', 'guides')} and parts[2].endswith('.md'):
            document = '/'.join(parts[1:])[:-3]
            identifier = ('guide-' if parts[1] == 'guides' else '') + PurePosixPath(name).stem
        elif name.startswith('curriculum/v5/exercises/X-') and name.endswith('.md'):
            identifier = anchor(PurePosixPath(name).stem)
            document = 'learning/exercises/' + PurePosixPath(name).stem
    clean_label = label.replace('`', '')
    if identifier and not fragment:
        return '{ref}`' + clean_label + ' <' + identifier + '>`'
    if document:
        # Preserve an explicit section request rather than silently linking the
        # page top. Sphinx still validates the fragment in the actual document.
        return '[' + label + '](/' + document + '.md#' + fragment + ')'
    destination = source / DOWNLOADS / name
    expected = source.resolve() / DOWNLOADS / name
    if destination.resolve() != expected:
        raise ValueError('Unsafe publication download destination: ' + name)
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(original, destination)
    relative = posixpath.relpath(DOWNLOADS + '/' + name, str(PurePosixPath(page).parent))
    result = '{download}`' + clean_label + ' <' + relative + '>`'
    if fragment:
        result += ' (source section `' + fragment.replace('`', '') + '`)'
    return result


def links(body, *, source, page, current=None, contracts=(), root=ROOT):
    from markdown_it import MarkdownIt

    def inline(text, html):
        matcher = (re.compile('(?P<html>' + '|'.join(re.escape(value) for value in html) + ')|' +
                              INLINE.pattern, re.S) if html else INLINE)
        return matcher.sub(lambda match: link(
            match['label'], match['address'], source=source, page=page,
            current=current, contracts=contracts, root=root) if match['link']
            else match[0], text)

    # CommonMark block maps decide what is prose. In particular, an unmatched
    # backtick cannot consume a later paragraph, and code/HTML block contents
    # must remain byte-for-byte source examples. Keep original line prefixes
    # (headings, list indentation, blockquotes) rather than re-rendering Markdown.
    offsets = [0]
    for line in body.splitlines(keepends=True):
        offsets.append(offsets[-1] + len(line))
    blocks = {}
    for token in MarkdownIt('commonmark').parse(body):
        if token.type == 'inline' and token.map:
            blocks.setdefault(tuple(token.map), set()).update(
                child.content for child in token.children or [] if child.type == 'html_inline')
    result, position = [], 0
    for (first, last), html in sorted(blocks.items()):
        start, end = offsets[first], offsets[last]
        if start < position:
            raise ValueError('Overlapping inline publication blocks')
        result.extend([body[position:start], inline(body[start:end], sorted(html, key=len, reverse=True))])
        position = end
    result.append(body[position:])
    return ''.join(result)


def plain_downloads(body):
    """Standalone Markdown refers to the same learner-safe canonical files."""
    return re.sub(r'\{download\}`([^`]+) <(?:\.\./)*' + re.escape(DOWNLOADS) + r'/([^>]+)>`',
                  lambda match: '[' + match[1] + '](' + match[2] + ')', body)
