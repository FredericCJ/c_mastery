"""Read-only discovery; derivatives are restricted to this project's private research tree."""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

sys.stdout.reconfigure(encoding='utf-8')

ROOT = Path(__file__).resolve().parents[1]
KNOWLEDGE = Path('F:/Knowledge')
DB = KNOWLEDGE / 'Library/_database/library.sqlite'
CLI = KNOWLEDGE / 'Library/_database/librarydb.exe'
SWE = Path('F:/Projects/Writing/corpora/SWE/explorer')

def digest(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()

def write_private(path, data, raw=False):
    path = Path(path).resolve()
    if not path.is_relative_to((ROOT / 'research/private').resolve()):
        raise ValueError('Derivatives must stay in c_mastery/research/private')
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(data if raw else json.dumps(data, ensure_ascii=False, indent=2), encoding='utf-8')

def rebase(value):
    if isinstance(value, dict):
        result = {k: rebase(v) for k, v in value.items()}
        if 'path_absolute' in result:
            old = result['path_absolute']
            candidate = Path(old.replace('F:\\Cours\\', 'F:\\Knowledge\\').replace('F:/Cours/', 'F:/Knowledge/'))
            result['resolved_path'] = str(candidate)
            result['resolved_exists'] = candidate.is_file()
        return result
    if isinstance(value, list):
        return [rebase(v) for v in value]
    return value

def query(args):
    command = [str(CLI), args.command, '--db', str(DB)]
    command += ['--q', args.query, '--limit', str(args.limit)] if args.command == 'search' else ['--id', args.id]
    result = subprocess.run(command, check=True, capture_output=True, text=True, encoding='utf-8')
    records = rebase(json.loads(result.stdout))
    write_private(args.output, records)
    rows = records if isinstance(records, list) else [records]
    print(json.dumps([{'id': r.get('document_id'), 'title': r.get('title'), 'exists': r.get('resolved_exists'), 'year': r.get('publication_year'), 'trust': r.get('enrichment_status')} for r in rows], ensure_ascii=False, indent=2))

def extract(args):
    path = Path(args.file).resolve()
    library = (KNOWLEDGE / 'Library').resolve()
    if not path.is_relative_to(library) or any(p.casefold() in {'_old', '_database', 'documentation', 'payload'} for p in path.parts):
        raise ValueError('Protected or opaque source boundary')
    if path.suffix.casefold() != '.pdf' or not path.is_file():
        raise ValueError('Only an explicitly selected ordinary PDF is accepted')
    command = ['pdftotext', '-f', str(args.first), '-l', str(args.last), '-layout', str(path), '-']
    result = subprocess.run(command, check=True, capture_output=True, text=True, encoding='utf-8')
    write_private(args.output, result.stdout, raw=True)
    print(json.dumps({'sha256': digest(path), 'pages': [args.first, args.last], 'characters': len(result.stdout), 'derivative': args.output}))

def swe(args):
    corpus = json.loads((SWE / 'data/corpus.json').read_text(encoding='utf-8-sig'))
    elements = json.loads((SWE / 'data/elements.json').read_text(encoding='utf-8-sig'))
    records = list(corpus.get('nodes', []))
    works = elements.get('works', {})
    records += list(works.values()) if isinstance(works, dict) else works
    words = [q.casefold() for q in args.query.split('|')]
    hits = [r for r in records if any(q in json.dumps(r, ensure_ascii=False).casefold() for q in words)]
    write_private(args.output, hits)
    print(json.dumps({'matches': len(hits), 'records': [{'id': r.get('id'), 'title': r.get('title'), 'verified': r.get('verified')} for r in hits[:args.limit]]}, ensure_ascii=False, indent=2))

def pins(args):
    selected = ['LIBRARY_RULES.yaml', 'LIBRARY_AGENTS.yaml', '_census/agents/SHARED.md', '_census/agents/roles/chief_librarian.md', '_census/agents/roles/metadata_provenance.md', '_census/v2/CURRENT.json', '_census/library_database/CURRENT.json', 'Library/_database/manifest.json']
    result = {'root': str(KNOWLEDGE), 'files': {p: digest(KNOWLEDGE / p) for p in selected}}
    current = json.loads((KNOWLEDGE / '_census/v2/CURRENT.json').read_text(encoding='utf-8-sig'))
    result['census_pointer'] = current
    write_private(args.output, result)
    print(json.dumps(result, ensure_ascii=False, indent=2))

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    for name in ['search', 'swe']:
        sub = commands.add_parser(name)
        sub.add_argument('query')
        sub.add_argument('--limit', type=int, default=12)
        sub.add_argument('--output', required=True)
    sub = commands.add_parser('show')
    sub.add_argument('id')
    sub.add_argument('--output', required=True)
    sub = commands.add_parser('extract')
    sub.add_argument('--file', required=True)
    sub.add_argument('--first', type=int, default=1)
    sub.add_argument('--last', type=int, required=True)
    sub.add_argument('--output', required=True)
    sub = commands.add_parser('pins')
    sub.add_argument('--output', required=True)
    args = parser.parse_args()
    {'search': query, 'show': query, 'swe': swe, 'extract': extract, 'pins': pins}[args.command](args)

if __name__ == '__main__':
    main()
