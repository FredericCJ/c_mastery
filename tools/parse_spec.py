"""Extract immutable coverage keys without treating the taxonomy as authored teaching."""
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
source = ROOT / 'c_programming_mastery_fine_grained_taxonomy.md'
text = source.read_text(encoding='utf-8-sig')
domains, domain, topic = [], None, None
for line in text.splitlines():
    found = re.match(r'^### (T\d{2}) — (.+)$', line)
    if found:
        domain = {'id': found[1], 'title': found[2], 'subtopics': []}
        domains.append(domain)
    found = re.match(r'^\*\*Track:\*\* (.+?)\. \*\*Depth:\*\* (.+?)\. \*\*Entry foundations:\*\* (.+)', line)
    if found and domain:
        domain.update(track=found[1], depth=found[2], prerequisite_domains=re.findall(r'T\d{2}', found[3]))
    found = re.match(r'^#### (T\d{2}\.\d{2}) — (.+)$', line)
    if found:
        topic = {'id': found[1], 'title': found[2].replace(' [E]', ''), 'elective': '[E]' in found[2], 'objectives': []}
        domain['subtopics'].append(topic)
    found = re.match(r'^- \*\*(T\d{2}\.\d{2}\.[a-z])\*\* (.+)$', line)
    if found:
        topic['objectives'].append({'id': found[1], 'text': found[2]})
assert len(domains) == 35
assert sum(len(d['subtopics']) for d in domains) == 395
assert sum(len(t['objectives']) for d in domains for t in d['subtopics']) == 1185
target = ROOT / 'curriculum/taxonomy.json'
target.parent.mkdir(parents=True, exist_ok=True)
target.write_text(json.dumps({'domains': domains}, ensure_ascii=False, indent=2), encoding='utf-8')
originals = ['c_programming_mastery_curriculum_prompt.md', 'c_programming_mastery_curriculum_prompt_expanded.md', 'c_programming_mastery_fine_grained_taxonomy.md']
inputs = {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in originals}
(ROOT / 'reviews/input_manifest.json').write_text(json.dumps(inputs, indent=2), encoding='utf-8')
print(f'Parsed {len(domains)} domains, 395 subtopics, 1185 objectives; pinned {len(inputs)} specifications')
