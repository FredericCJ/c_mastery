"""Revert the parts of the f6f8d8c07 text rename that the corrected
tools/rename_text.py would not have made (prose acronyms, non-C Python strings,
printed output). Writes a receipt of every reverted token."""
import io, json, re, subprocess, sys, tokenize as tk
from pathlib import Path
ROOT = Path('/home/fred/dev/c_mastery'); sys.path.insert(0, str(ROOT / 'tools'))
import rename_text as rt
REN = 'f6f8d8c07'
mapping = json.loads((ROOT / 'reviews/v5/expansion/polish_revision/mechanical/rename_map.json').read_text())
P = rt.Patterns(mapping)
_names = sorted((n for n in mapping if rt.distinctive(n) and len(n) >= 3), key=len, reverse=True)
OLD = re.compile(r"(?<![A-Za-z0-9_])(" + "|".join(map(re.escape, _names)) + r")(?![A-Za-z0-9_])")  # first run: no guards
def old_replace(text): return OLD.sub(lambda m: mapping[m.group(1)], text)
def git(rev, f): return subprocess.run(['git', 'show', f'{rev}:{f}'], cwd=ROOT, capture_output=True).stdout.decode('utf-8', 'surrogateescape')

def py_tokens(src):
    out = []
    for t in tk.generate_tokens(io.StringIO(src).readline):
        if t.type == tk.STRING: out.append(t.string)
    return out
def kind_of(f):
    if f.endswith('.py') and f.startswith(rt.PYTHON) and f not in rt.SKIP_PYTHON: return 'py'
    if f.endswith('.md') and f.startswith(rt.CANONICAL_MARKDOWN): return 'md'
    if f.endswith('.json') and f.startswith(rt.CANONICAL_JSON): return 'json'
    return None
def segments(text, kind):
    if kind == 'py': return py_tokens(text)
    if kind == 'json': return re.findall(r'"(?:[^"\\]|\\.)*"', text)
    return text.splitlines(keepends=True)
def old_seg(seg, kind, md_state=None):
    return old_replace(seg)                # first run applied the plain pattern to each segment
def new_seg(seg, kind):
    return P.rename(seg)

files = subprocess.run(['git', 'diff', '--name-only', REN + '^', REN], cwd=ROOT, capture_output=True, text=True).stdout.split()
report = dict(reverted=[], manual=[], unchanged_files=0)
for f in files:
    kind = kind_of(f)
    if not kind or f.startswith(('reviews/', 'research/', 'build/')) or not (ROOT / f).exists(): continue
    before, after = git(REN + '^', f), git(REN, f)
    if kind == 'md':
        corrected = rt.markdown_code(before, P)
        if corrected == after: report['unchanged_files'] += 1; continue
        a_lines, c_lines = after.splitlines(keepends=True), corrected.splitlines(keepends=True)
        pairs = [(a, c) for a, c in zip(a_lines, c_lines) if a != c] if len(a_lines) == len(c_lines) else None
    else:
        try:
            sb, sa = segments(before, kind), segments(after, kind)
        except Exception as e:
            report['manual'].append((f, 'tokenize: ' + str(e))); continue
        if len(sb) != len(sa) or any(old_replace(x) != y for x, y in zip(sb, sa)):
            # the commit changed this file beyond the text rename: decide by hand
            report['manual'].append((f, 'not a pure text-rename change')); continue
        pairs = [(y, new_seg(x, kind)) for x, y in zip(sb, sa) if new_seg(x, kind) != y]
        if not pairs: report['unchanged_files'] += 1; continue
    if pairs is None:
        report['manual'].append((f, 'markdown line count differs')); continue
    current = (ROOT / f).read_bytes().decode('utf-8', 'surrogateescape')
    for wrong, right in dict.fromkeys(pairs):
        n = current.count(wrong)
        if n == 0:
            report['manual'].append((f, 'segment no longer present: ' + wrong[:80])); continue
        current = current.replace(wrong, right)
        report['reverted'].append(dict(file=f, occurrences=n, renamed=wrong[:200], restored=right[:200]))
    if '--apply' in sys.argv:
        (ROOT / f).write_bytes(current.encode('utf-8', 'surrogateescape'))
json.dump(report, open('/home/fred/.cache/cmastery-tmp/orch/repair_report.json', 'w'), indent=1)
files_rev = sorted({r['file'] for r in report['reverted']})
print(len(report['reverted']), 'segments in', len(files_rev), 'files; manual:', len(report['manual']), '; files already correct:', report['unchanged_files'])
