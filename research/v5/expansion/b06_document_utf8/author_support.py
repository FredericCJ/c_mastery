"""Pure, idempotent public document attachment; original teaching stays intact."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
MANIFEST = 'research/v5/expansion/b06_document_utf8/bindings.json'
GUIDE = 'curriculum/guides/b06-current-documents.md'
HEADING = '\n## Current document renditions\n'
BODY = (HEADING + '\nUse the [current document guide](../guides/b06-current-documents.md) '
        'for the required readable teaching documents. Its UTF-8 renditions preserve '
        'the specified technical requirements; the original byte sequences remain '
        'available solely for historical source and execution identities. '
        'This publication correction adds no execution or learner evidence.\n')


def attach(project, meta, text):
    manifest = ROOT / MANIFEST
    if not manifest.is_file():
        return meta, text
    assets = json.loads(manifest.read_text(encoding='utf8'))['projects'].get(project)
    if assets is None:
        return meta, text
    meta['shared_contracts'] = list(dict.fromkeys(meta.get('shared_contracts', []) + [GUIDE]))
    meta['supplied_artifacts'] = list(dict.fromkeys(meta.get('supplied_artifacts', []) + assets))
    meta['document_rendition_support'] = 'research/v5/expansion/b06_document_utf8/completion_dispositions.json'
    if HEADING not in text:
        text = text.rstrip() + '\n' + BODY
    return meta, text
