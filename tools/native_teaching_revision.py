"""Exact, reversible public teaching corrections without rewriting old evidence."""
import hashlib
import json
from pathlib import Path


def qualifies(name, expected_sha, *, root):
    packet = Path(root)/'curriculum/v5/routes/teaching_revisions.json'
    if not packet.is_file():
        return False
    try:
        rows=json.loads(packet.read_text(encoding='utf-8'))['records']
        matches=[r for r in rows if r['path']==name]
        if len(matches)!=1:
            return False
        row=matches[0];before=row['predecessor'].encode('utf-8')
        if hashlib.sha256(before).hexdigest()!=expected_sha or row['predecessor_sha256']!=expected_sha:
            return False
        after=before.decode('utf-8')
        for replacement in row['replacements']:
            if after.count(replacement['old'])!=1:
                return False
            after=after.replace(replacement['old'],replacement['new'])
        actual=(Path(root)/name).read_bytes()
        return actual==after.encode('utf-8') and hashlib.sha256(actual).hexdigest()==row['current_sha256']
    except (OSError,KeyError,ValueError,TypeError):
        return False
